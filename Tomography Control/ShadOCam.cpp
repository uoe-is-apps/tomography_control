#include "stdafx.h"

#include <math.h>

#include "Tomography Control.h"
#include "Camera.h"
#include "Exceptions.h"

	ShadOCam::ShadOCam(CString directory, float exposureTimeSeconds, char* camFilePath, char *pixMapFilePath) : Camera(directory)
{
	strcpy(this -> m_camFilePath, camFilePath);
	strcpy(this -> m_pixMapFilePath, pixMapFilePath);
	
	this -> m_bPxdLoaded = FALSE;
	this -> m_currentFrame = NULL;
	this -> m_sumFrame = NULL;
	this -> m_bCamTypeLoaded = FALSE;
	this -> m_bFrameGrabberAllocated = FALSE;
	this -> m_bFramelibLoaded = FALSE;

	this -> m_exposureTimeSeconds = exposureTimeSeconds;
}

ShadOCam::~ShadOCam()
{
	if (this -> m_bPxdLoaded)
	{
		if (NULL != this -> m_pixMap)
		{
			GlobalFree(this -> m_pixMap);
		}

		if (NULL != m_sumFrame)
		{
			this -> m_pxd.FreeFrame(this -> m_sumFrame);
		}

		if (NULL != m_currentFrame)
		{
			this -> m_pxd.FreeFrame(this -> m_currentFrame);
		}

		if (this -> m_bCamTypeLoaded)
		{
			this -> m_pxd.FreeConfig(this -> m_camType);
		}

		if (this -> m_bFrameGrabberAllocated)
		{
			this -> m_pxd.FreeFG(this -> m_hFG);
		}

		if (this -> m_bFramelibLoaded)
		{
			imagenation_CloseLibrary(&this -> m_framelib);
		}
		if (this -> m_bPxdLoaded)
		{
			imagenation_CloseLibrary(&this -> m_pxd);
		}
	}
}

void ShadOCam::AddFrameToBuffer(FRAME *destFrame, FRAME *currentFrame)
{
	short *currentFramePtr = (short *)this -> m_framelib.FrameBuffer(currentFrame);
	short *destFramePtr = (short *)this -> m_framelib.FrameBuffer(destFrame);
	
	int pixelCount = GetImageHeight() * GetImageWidth();

	for (int pixel = 0; pixel < pixelCount; pixel++)
	{
      	*(destFramePtr++) += *(currentFramePtr++);
	}
}

/* Calculate the average of every pixel in a frame, based on a frame containing
 * pixel sums, and number of captured images.
 */
void ShadOCam::CalculatePixelAverages(FRAME *dest, FRAME *src, unsigned short capturedFrames)
{
	unsigned int pixelCount = GetImageHeight() * GetImageWidth();
	short *destBufferPtr = (short *)this -> m_framelib.FrameBuffer(dest);
	short *sourceBufferPtr = (short *)this -> m_framelib.FrameBuffer(src);

	for (unsigned int pixel = 0; pixel < pixelCount; pixel++)
	{
		double sum = *(sourceBufferPtr + pixel);
		double average = sum / capturedFrames;

		*(destBufferPtr++) = (unsigned short)floor(average + 0.50);
	}
}

/* Copies the sum data into the standard buffer, ready to write out to disk.
 */
void ShadOCam::CalculatePixelSums(FRAME *dest, FRAME *src)
{
	unsigned int pixelCount = GetImageHeight() * GetImageWidth();
	short *destBufferPtr = (short *)this -> m_framelib.FrameBuffer(dest);
	short *sourceBufferPtr = (short *)this -> m_framelib.FrameBuffer(src);

	// Copy the sum data into the image buffer
	for (unsigned int pixel = 0; pixel < pixelCount; pixel++)
	{
		*(destBufferPtr + pixel) = *(sourceBufferPtr + pixel);
	}
}

void ShadOCam::ClearFrame(FRAME *destFrame)
{
	short *destFramePtr = (short *)this -> m_framelib.FrameBuffer(destFrame);
	
	int pixelCount = GetImageHeight() * GetImageWidth();

	for (int pixel = 0; pixel < pixelCount; pixel++)
	{
      	*(destFramePtr++) = 0;
    }
}

double ShadOCam::CalculatePixelAverage(FRAME *currentFrame)
{
	long pixelSum = 0;
	short *currentFramePtr = (short *)this -> m_framelib.FrameBuffer(currentFrame);
	
	int pixelCount = GetImageHeight() * GetImageWidth();

	for (int pixel = 0; pixel < pixelCount; pixel++)
	{
		pixelSum += *(currentFramePtr++);
	}

	return ((double)pixelSum) / (this -> GetImageHeight() * this -> GetImageWidth());
}

void ShadOCam::CaptureFrames(u_int frames, u_int *current_position,
	FrameSavingOptions frameSavingOptions, FrameType frameType, CWnd* window,
	CTime timeoutAt)
{
	unsigned short capturedFrames = 0;
	BOOL lastPixelAverageValid = FALSE;
	double lastPixelAverage = 0.0;
	int pixelCount = GetImageHeight() * GetImageWidth();
	long qh; // handle for grab; only needed to confirm image was taken
			 // successfully, does not reserve memory for the grab.
	char *filename;
	char *filepath;

	short *currentFramePtr // The frame currently being captured into
		= (short *)this -> m_framelib.FrameBuffer(this -> m_currentFrame);
	short *currentTempPtr; // Working position within the frame

	currentFramePtr++; // point to first pixel in buffer1; 

	// Clear the sum buffer
	ClearFrame(this -> m_sumFrame);


	// Loop through the capture process until we acquire enough valid images.
	while (capturedFrames < frames
		&& CTime::GetCurrentTime() < timeoutAt)
	{
		// grab
		qh = this -> m_pxd.Grab(this -> m_hFG, this -> m_currentFrame,
			0); // 0 indicates the method should not return until the image has been captured
		if (!qh)
		{
			sprintf_s(this -> m_errorBuffer, ERROR_BUFFER_SIZE, "Unable to acquire image: %d",
				this -> m_pxd.CheckError(this -> m_hFG));

      		throw camera_acquisition_error(this -> m_errorBuffer);
		}

		// Verify the beam is still active, by checking pixel average against last value for
		// this set.
		double pixelAverage = this -> CalculatePixelAverage(m_currentFrame);

		// We can only check against previous value, if we actually have a previous value.
		if (lastPixelAverageValid)
		{
			lastPixelAverage = pixelAverage;

			double pixelRatio = pixelAverage / lastPixelAverage;
			double variation = abs(1.0 - pixelRatio);

			if (variation >= PIXEL_AVERAGE_TOLERANCE)
			{
				// Likely beam failure, skip frame. Recovery is done after main capture
				window -> PostMessage(WM_USER_BEAM_FAILURE, 0, NULL);
				continue;
			}
		}
		else
		{
			lastPixelAverageValid = TRUE;
			lastPixelAverage = pixelAverage;
		}
      
      	// deinterlace image
      	ScDeinterlace(currentFramePtr, this -> m_framelib.FrameWidth(this -> m_currentFrame),
			this -> m_nWidth, (this -> m_nHeight/2),
			SCCAMTYPE_4K , TRUE);

      	//pixel map correction
      	ScPixelCorrection(currentFramePtr,
			this -> m_nWidth + 2, this -> m_nHeight,
			this -> m_pixMap, this -> m_pixMapEntries, SCMETHOD_INTERPOLATE);
      
		switch (frameSavingOptions)
		{
		case SUM:
		case AVERAGE:
			this -> AddFrameToBuffer(this -> m_sumFrame, this -> m_currentFrame);
			break;
		case INDIVIDUAL:		
			currentTempPtr = currentFramePtr;

			for (int pixel = 0; pixel < pixelCount; pixel++)
			{
         		// swap bytes
   				*currentTempPtr = (*currentTempPtr >> 8) + (((unsigned char)*currentTempPtr)<<8);
				currentTempPtr++;
			}

			filename = GenerateImageFilename(frameType, *current_position);
			filepath = GenerateImagePath(filename);

			window -> PostMessage(WM_USER_CAPTURING_FRAME, 0, (LPARAM)filename);

			//save file
			this -> m_framelib.WriteBin(this -> m_currentFrame, filepath, 1);

			window -> PostMessage(WM_USER_FRAME_CAPTURED, 0, (LPARAM)(*current_position));
			(*current_position)++;
			break;
		default:
			throw bad_frame_saving_options_error("Unknown frame saving options specified.");
		}

		capturedFrames++;
	}
    
	if (capturedFrames < frames)
	{
		// Timeout due to beam failure
		throw xray_beam_failure_error("X-ray beam failure detected. Timed out while attempting recovery.");
	}

	// For average/sum images, we haven't been writing them as we go along,
	// but instead write them at the end
	if (frameSavingOptions == INDIVIDUAL)
	{
		return;
	}
	
	switch (frameSavingOptions)
	{
	case INDIVIDUAL:
		return;

	case SUM:
		this -> CalculatePixelSums(this -> m_currentFrame, this -> m_sumFrame);
		break;

	case AVERAGE:
		this -> CalculatePixelAverages(this -> m_currentFrame, this -> m_sumFrame, capturedFrames);
		break;

	default:
		throw bad_frame_saving_options_error("Unknown frame saving options specified.");
	}
		
	currentTempPtr = currentFramePtr;

	for (int pixel = 0; pixel < pixelCount; pixel++)
	{
        // swap bytes
   		*currentTempPtr = (*currentTempPtr >> 8) + (((unsigned char)*currentTempPtr)<<8);
		currentTempPtr++;
	}

	// Write out the current frame buffer
	filename = GenerateImageFilename(frameType, *current_position);
	filepath = GenerateImagePath(filename);

	window -> PostMessage(WM_USER_CAPTURING_FRAME, 0, (LPARAM)filename);
		
	this -> m_framelib.WriteBin(this -> m_currentFrame, filepath, 1);
	
	window -> PostMessage(WM_USER_FRAME_CAPTURED, 0, (LPARAM)(*current_position));
	(*current_position)++;
}

char *ShadOCam::GenerateImageFilename(FrameType frameType, u_int frame) {
	return Camera::GenerateImageFilename(frameType, frame, "raw");
}

u_short ShadOCam::GetImageHeight() {
	return this -> m_nHeight;
}

u_short ShadOCam::GetImageWidth() {
	return this -> m_nWidth;
}

void ShadOCam::SetupCamera()
{
	this -> m_pixMap = NULL;
	this -> m_bPxdLoaded = FALSE;
	this -> m_bCamTypeLoaded = FALSE;
	this -> m_bFramelibLoaded = FALSE;
	this -> m_bFrameGrabberAllocated = FALSE;
	
	this -> m_currentFrame = NULL;
	this -> m_sumFrame = NULL;

	// Load the image capture libraries
	if ( !imagenation_OpenLibrary("pxd_32.dll", &this -> m_pxd, sizeof(PXD)) )  {
		throw camera_init_error("Frame grabber library not loaded.");
	}
	this -> m_bPxdLoaded = TRUE;
	
	if ( !imagenation_OpenLibrary("frame_32.dll", &this -> m_framelib, sizeof(FRAMELIB)) )  {
		throw camera_init_error("Frame library not loaded.");
	}
	this -> m_bFramelibLoaded = TRUE;

	// request access to frame grabber
	if ( !(this -> m_hFG = this -> m_pxd.AllocateFG(-1)) )  {
		throw camera_init_error("PXD frame grabber not found.");
	}
	this -> m_bFrameGrabberAllocated = TRUE;

	// initialize camera configuration
	if ( !(this -> m_camType = this -> m_pxd.LoadConfig(this -> m_camFilePath)) )  {
		throw camera_init_error("Camera configuration not loaded.");
	}
	this -> m_bCamTypeLoaded = TRUE;

	const long len = 65536; // length of LUT
	unsigned short nLUT[len];  // LUT

	//set exposure time
	float ft = this -> m_pxd.GetFramePeriod(this -> m_camType);
	this -> m_pxd.SetFramePeriod(this -> m_camType, this -> m_exposureTimeSeconds);
	ft = this -> m_pxd.GetFramePeriod(this -> m_camType);

	this -> m_pxd.SetCameraConfig(this -> m_hFG, this -> m_camType);
	this -> m_pxd.ContinuousStrobes(this -> m_hFG, TRUE);  // turn on camera frame sync

	// initialize input LUT to shift image data down by two bits
	for (int i = 0; i < len; i++)
	{
		nLUT[i] = i>>2;
	}
	this -> m_pxd.SetInputLUT(this -> m_hFG, 16, 0, 0, len, nLUT);

	// set up image destination buffers
	if ( !(this -> m_currentFrame = this -> m_pxd.AllocateBuffer(
		this -> m_pxd.GetWidth(this -> m_hFG), this -> m_pxd.GetHeight(this -> m_hFG), PBITS_Y16)) )  {
		throw camera_init_error("Unable to create image buffer.");
	}

	// set up image destination buffers
	if ( !(this -> m_sumFrame = this -> m_pxd.AllocateBuffer(
		this -> m_pxd.GetWidth(this -> m_hFG), this -> m_pxd.GetHeight(this -> m_hFG), PBITS_Y16)) )  {
		throw camera_init_error("Unable to create sum image buffer.");
	}

    this -> m_pixMap = (PIXMAPENTRY*)GlobalAlloc(0, SCMAXPIXMAPSIZE* sizeof(PIXMAPENTRY));
    ScReadPixMap(this -> m_pixMapFilePath, this -> m_pixMap, &this -> m_pixMapEntries);
	
	this -> m_nWidth  = 2048;
	this -> m_nHeight = 2000;
}