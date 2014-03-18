#include "stdafx.h"
#include "Tomography Control.h"
#include "Camera.h"
#include "Exceptions.h"

	ShadOCam::ShadOCam(char* directory, char* camFilePath, char *pixMapFilePath) : Camera(directory)
{
	strcpy(this -> m_camFilePath, camFilePath);
	strcpy(this -> m_pixMapFilePath, pixMapFilePath);
}

ShadOCam::~ShadOCam()
{
	if (this -> m_bPxdLoaded)
	{
		if (NULL != this -> m_pixMap)
		{
			GlobalFree(this -> m_pixMap);
		}

		if (NULL != m_avgSumFrame)
		{
			free(this -> m_avgSumFrame);
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
		imagenation_CloseLibrary(&this -> m_pxd);
	}
}
	
void ShadOCam::AddFrameToBuffer(unsigned int *dest, FRAME *currentFrame)
{
	short *currentFramePtr = (short *)this -> m_framelib.FrameBuffer(currentFrame);

	for (unsigned short row = 0; row < this -> GetImageHeight(); row++)
	{
      	for (unsigned short col = 0; col < this -> GetImageWidth(); col++)
      	{	
      		*(dest++) += *(currentFramePtr++);
      	}
	}
}

double ShadOCam::CalculatePixelAverage(FRAME *currentFrame)
{
	long pixelSum = 0;
	short *currentFramePtr = (short *)this -> m_framelib.FrameBuffer(currentFrame);
	
	for (unsigned short row = 0; row < this -> GetImageHeight(); row++)
	{
      	for (unsigned short col = 0; col < this -> GetImageWidth(); col++)
      	{
			pixelSum += *(currentFramePtr++);
		}
	}

	return ((double)pixelSum) / (this -> GetImageHeight() * this -> GetImageWidth());
}

void ShadOCam::CaptureFrames(u_int frames, u_int *current_position,
	FrameSavingOptions frameSavingOptions, FrameType frameType, CWnd* window)
{
	unsigned short capturedImages = 0;
	BOOL lastPixelAverageValid = FALSE;
	double lastPixelAverage = 0.0;
	long qh; // handle for grab; only needed to confirm image was taken
			 // successfully, does not reserve memory for the grab.
	char *filename;

	for (u_int frame = 0; frame < frames; frame++)
	{
		// grab
		qh = this -> m_pxd.Grab(this -> m_hFG, this -> m_currentFrame,
			0); // 0 indicates the method should not return until the image has been captured
		if (!qh)
		{
			sprintf_s(this -> m_errorBuffer, ERROR_BUFFER_SIZE, "Unable to acquire image: %d",
				this -> m_pxd.CheckError(this -> m_hFG));

      		throw new camera_acquisition_error(this -> m_errorBuffer);
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
				// Likely beam failure, skip this image
				continue;
			}
		}
		else
		{
			lastPixelAverageValid = TRUE;
			lastPixelAverage = pixelAverage;
		}
      
      	// deinterlace image
		
		short *currentFramePtr = (short *)this -> m_framelib.FrameBuffer(this -> m_currentFrame);
		currentFramePtr++; // point to first pixel in buffer1

      	ScDeinterlace(currentFramePtr, this -> m_framelib.FrameWidth(this -> m_currentFrame),
			this -> m_nWidth, (this -> m_nHeight/2),
			SCCAMTYPE_4K , TRUE);

      	//pixel map correction
      	ScPixelCorrection(currentFramePtr,
			this -> m_nWidth + 2, this -> m_nHeight,
			this -> m_pixMap, this -> m_pixMapEntries, SCMETHOD_INTERPOLATE);
		
		short *currentTempPtr = currentFramePtr;
		for (unsigned short row = 0; row < this -> GetImageHeight(); row++)
		{
      		for (unsigned short col = 0; col < this -> GetImageWidth() + 2; col++)
      		{
         		// swap bytes
   				*currentTempPtr = (*currentTempPtr >> 8) + (((unsigned char)*currentTempPtr)<<8);
				currentTempPtr++;
        	}
		}
      
		switch (frameSavingOptions)
		{
		case SUM:
		case AVERAGE:
			this -> AddFrameToBuffer(this -> m_avgSumFrame, this -> m_currentFrame);
			break;
		case INDIVIDUAL:
			filename = GenerateImageFilename(frameType, *current_position);
			window -> PostMessage(WM_USER_CAPTURING_FRAME, 0, (LPARAM)(filename + strlen(this -> GetDirectory()) + 1));

			//save file
			this -> m_framelib.WriteBin(this -> m_currentFrame, filename, 1);

			window -> PostMessage(WM_USER_FRAME_CAPTURED, 0, (LPARAM)(*current_position));
			(*current_position)++;
			break;
		default:
			throw new bad_frame_saving_options_error("Unknown frame saving options specified.");
		}

		capturedImages++;
	}

	if (frameSavingOptions == INDIVIDUAL)
	{
		// Nothing more to do
		return;
	}
	else
	{
		// Write out sum/average files
		filename = GenerateImageFilename(frameType, *current_position);
	
		short *averageBufferPtr = (short *)this -> m_framelib.FrameBuffer(this -> m_currentFrame);
		unsigned int *sourceFramePtr = this -> m_avgSumFrame;

		switch (frameSavingOptions)
		{
		case SUM:
			// Copy sum values into current frame buffer
			for (unsigned short row = 0; row < this -> GetImageHeight(); row++)
			{
				for (unsigned short col = 0; col < this -> GetImageWidth(); col++)
				{
					*(averageBufferPtr++) = *(sourceFramePtr++);
				}
			}

			break;
		case AVERAGE:
			// Calculate averages and write them into the current frame buffer
			for (unsigned short row = 0; row < this -> GetImageHeight(); row++)
			{
				for (unsigned short col = 0; col < this -> GetImageWidth(); col++)
				{
					double sum = *(sourceFramePtr++);

					*(averageBufferPtr++) = (short)(sum / capturedImages);
				}
			}

			break;
		default:
			throw new bad_frame_saving_options_error("Unknown frame saving options specified.");
		}

		// Write out the current frame buffer
		window -> PostMessage(WM_USER_CAPTURING_FRAME, 0, (LPARAM)(filename + strlen(this -> GetDirectory()) + 1));

		this -> WriteTiff(filename, this -> m_avgSumFrame);
	
		window -> PostMessage(WM_USER_FRAME_CAPTURED, 0, (LPARAM)(*current_position));
		(*current_position)++;
	}
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

void ShadOCam::SetupCamera(float exposureTimeSeconds)
{
	this -> m_pixMap = NULL;
	this -> m_bPxdLoaded = FALSE;
	this -> m_bCamTypeLoaded = FALSE;
	this -> m_bFramelibLoaded = FALSE;
	this -> m_bFrameGrabberAllocated = FALSE;
	
	this -> m_currentFrame = NULL;
	this -> m_avgSumFrame = NULL;

	// Load the image capture libraries
	if ( !imagenation_OpenLibrary("pxd_32.dll", &this -> m_pxd, sizeof(PXD)) )  {
		throw new camera_init_error("Frame grabber library not loaded.");
	}
	this -> m_bPxdLoaded = TRUE;
	
	if ( !imagenation_OpenLibrary("frame_32.dll", &this -> m_framelib, sizeof(FRAMELIB)) )  {
		throw new camera_init_error("Frame library not loaded.");
	}
	this -> m_bFramelibLoaded = TRUE;

	// request access to frame grabber
	if ( !(this -> m_hFG = this -> m_pxd.AllocateFG(-1)) )  {
		throw new camera_init_error("PXD frame grabber not found.");
	}
	this -> m_bFrameGrabberAllocated = TRUE;

	// initialize camera configuration
	if ( !(this -> m_camType = this -> m_pxd.LoadConfig(this -> m_camFilePath)) )  {
		throw new camera_init_error("Camera configuration not loaded.");
	}
	this -> m_bCamTypeLoaded = TRUE;

	const long len = 65536; // length of LUT
	unsigned short nLUT[len];  // LUT

	//set exposure time
	float ft = this -> m_pxd.GetFramePeriod(this -> m_camType);
	this -> m_pxd.SetFramePeriod(this -> m_camType, exposureTimeSeconds);
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
		throw new camera_init_error("Unable to create image buffer.");
	}

	// set up image destination buffers
	this -> m_avgSumFrame = (unsigned int *)malloc(sizeof(unsigned int)
		* this -> m_pxd.GetWidth(this -> m_hFG) * this -> m_pxd.GetHeight(this -> m_hFG));
	if (NULL == this -> m_avgSumFrame)  {
		throw new camera_init_error("Unable to create average/sum image buffer.");
	}

    this -> m_pixMap = (PIXMAPENTRY*)GlobalAlloc(0, SCMAXPIXMAPSIZE* sizeof(PIXMAPENTRY));
    ScReadPixMap(this -> m_pixMapFilePath, this -> m_pixMap, &this -> m_pixMapEntries);
	
	this -> m_nWidth  = 2048;
	this -> m_nHeight = 2000;
}