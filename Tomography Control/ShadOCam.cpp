#include "stdafx.h"
#include "Tomography Control.h"
#include "Camera.h"
#include "Exceptions.h"

	ShadOCam::ShadOCam(char* directory, char* camFilePath, char *pixMapFilePath) : Camera(directory)
{
	this -> m_camFilePath = camFilePath;
	this -> m_pixMapFilePath = pixMapFilePath;
}

ShadOCam::~ShadOCam()
{
	if (this -> m_bPxdLoaded)
	{
		if (this -> m_bFramelibLoaded)
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
				this -> m_pxd.FreeFG(this -> m_hFrameGrabber);
			}

			imagenation_CloseLibrary(&this -> m_framelib);
		}
		imagenation_CloseLibrary(&this -> m_pxd);
	}
}
	
void ShadOCam::AddFrameToBuffer(unsigned int *dest, FRAME *currentFrame)
{
	short *currentFramePtr = (short *)this -> m_framelib.FrameBuffer(currentFrame);

	for (unsigned short row = 0; row < this -> GetFrameBufferHeight(); row++)
	{
      	for (unsigned short col = 0; col < this -> GetFrameBufferWidth(); col++)
      	{	
      		*(dest++) += *(currentFramePtr++);
      	}
	}
}

double ShadOCam::CalculatePixelAverage(FRAME *currentFrame)
{
	long pixelSum = 0;
	short *currentFramePtr = (short *)this -> m_framelib.FrameBuffer(currentFrame);
	
	for (unsigned short row = 0; row < this -> GetFrameBufferHeight(); row++)
	{
      	for (unsigned short col = 0; col < this -> GetFrameBufferWidth(); col++)
      	{
			pixelSum += *(currentFramePtr++);
		}
	}

	return ((double)pixelSum) / (this -> GetFrameBufferHeight() * this -> GetFrameBufferWidth());
}

void ShadOCam::CaptureFrames(u_int frames, u_int *frameCount, FrameSavingOptions captureType, FrameType frameType, CWnd* window)
{
	unsigned short capturedImages = 0;
	short *currentFramePtr = (short *)this -> m_framelib.FrameBuffer(this -> m_currentFrame);
	BOOL lastPixelAverageValid = FALSE;
	double lastPixelAverage = 0.0;
	long qh; // handle for grab; only needed to confirm image was taken
			 // successfully, does not reserve memory for the grab.
	char *filename;

	for (u_int frame = 0; frame < frames; frame++, *frameCount++)
	{
		// grab
		qh = this -> m_pxd.Grab(this -> m_hFrameGrabber, this -> m_currentFrame, 0); // 0 indicates the method should not return until the image has been captured
		if (!qh)
		{
      		throw "Unable to acquire image.";
		}

		// Verify the beam is still active, by checking pixel average against last value for
		// this set.
		double pixelAverage = this -> CalculatePixelAverage(m_currentFrame);

		// We can only check against previous value, if we actually have a previous value.
		if (lastPixelAverageValid)
		{
			double pixelRatio = pixelAverage / lastPixelAverage;
			double variation = abs(1.0 - pixelRatio);

			if (variation >= PIXEL_AVERAGE_TOLERANCE)
			{
				// Likely beam failure, skip this image
				continue;
			}
		}
      
      	// deinterlace image
      	ScDeinterlace(currentFramePtr, this -> m_framelib.FrameWidth(this -> m_currentFrame),
			this -> GetFrameBufferWidth(), this -> GetFrameBufferHeight(),
			SCCAMTYPE_4K , TRUE);
		
      	//pixel map correction
      	ScPixelCorrection(currentFramePtr,
			this -> GetFrameBufferWidth(), this -> GetFrameBufferHeight(),
			this -> m_pixMap, this -> m_pixMapEntries, SCMETHOD_INTERPOLATE);
		
		short *currentTempPtr = currentFramePtr;
		for (unsigned short row = 0; row < this -> GetFrameBufferHeight(); row++)
		{
      		for (unsigned short col = 0; col < this -> GetFrameBufferWidth(); col++)
      		{
         		// swap bytes
   				*currentTempPtr = (*currentTempPtr>>8) + (((unsigned char)*currentTempPtr)<<8);
				currentTempPtr++;
        	}
		}
      
		switch (captureType)
		{
		case SUM:
		case AVERAGE:
			this -> AddFrameToBuffer(this -> m_avgSumFrame, this -> m_currentFrame);
			break;
		default:
			filename = GenerateImageFilename(frameType, (*frameCount)++);
			window -> PostMessage(WM_USER_CAPTURING_FRAME, 0, (LPARAM)filename);

			//save file
			this -> m_framelib.WriteBin(this -> m_currentFrame, filename, 1);

			window -> PostMessage(WM_USER_FRAME_CAPTURED, 0, (LPARAM)(*frameCount));
			*(frameCount)++;
		}

		capturedImages++;
	}

	if (captureType == INDIVIDUAL)
	{
		// Nothing more to do
		return;
	}
	else
	{
		// Write out sum/average files
		filename = GenerateImageFilename(frameType, (*frameCount)++);
		// Clear the current frame, so we can use it to generate the file to be written out.
		memset(currentFramePtr, 0, this -> GetFrameBufferWidth() * this -> GetFrameBufferHeight() * sizeof(short));
	
		short *averageBufferPtr = currentFramePtr;
		unsigned int *sourceFramePtr = this -> m_avgSumFrame;

		switch (captureType)
		{
		case SUM:
			// Copy sum values into current frame buffer
			for (unsigned short row = 0; row < this -> GetFrameBufferHeight(); row++)
			{
				for (unsigned short col = 0; col < this -> GetFrameBufferWidth(); col++)
				{
					*(averageBufferPtr++) = *(sourceFramePtr++);
				}
			}

			break;
		case AVERAGE:
			// Calculate averages and write them into the current frame buffer
			for (unsigned short row = 0; row < this -> GetFrameBufferHeight(); row++)
			{
				for (unsigned short col = 0; col < this -> GetFrameBufferWidth(); col++)
				{
					double sum = *(sourceFramePtr++);

					*(averageBufferPtr++) = (short)(sum / capturedImages);
				}
			}

			break;
		}

		// Write out the current frame buffer
		window -> PostMessage(WM_USER_CAPTURING_FRAME, 0, (LPARAM)filename);

		this -> WriteTiff(filename, this -> m_avgSumFrame);
	
		window -> PostMessage(WM_USER_FRAME_CAPTURED, 0, (LPARAM)(*frameCount));
		*(frameCount)++;
	}
}

char *ShadOCam::GenerateImageFilename(FrameType frameType, u_int frame) {
	return Camera::GenerateImageFilename(frameType, frame, "raw");
}

u_short ShadOCam::GetFrameBufferHeight() {
	return this -> m_frameGrabberHeight;
}

u_short ShadOCam::GetFrameBufferWidth() {
	return this -> m_frameGrabberWidth;
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

	this -> m_nWidth  = 2048;		// width of image
	this -> m_nHeight = 2000;		// height of image
	
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
	if ( !(this -> m_hFrameGrabber = this -> m_pxd.AllocateFG(-1)) )  {
		throw new camera_init_error("PXD frame grabber not found.");
	}
	this -> m_bFrameGrabberAllocated = TRUE;

	// initialize camera configuration
	if ( !(this -> m_camType = this -> m_pxd.LoadConfig(this -> m_camFilePath)) )  {
		throw new camera_init_error("Camera configuration not loaded.");
	}
	this -> m_bCamTypeLoaded = TRUE;
	
	this -> m_frameGrabberWidth = this -> m_pxd.GetWidth(this -> m_hFrameGrabber);
	this -> m_frameGrabberHeight = this -> m_pxd.GetHeight(this -> m_hFrameGrabber);

	// set up image destination buffers
	if ( !(this -> m_currentFrame = this -> m_pxd.AllocateBuffer (this -> m_frameGrabberWidth, this -> m_frameGrabberHeight, PBITS_Y16)) )  {
		throw new camera_init_error("Unable to create image buffer.");
	}

	// set up image destination buffers
	this -> m_avgSumFrame = (unsigned int *)malloc(sizeof(unsigned int) * this -> m_frameGrabberWidth * this -> m_frameGrabberHeight);
	if (NULL == this -> m_avgSumFrame)  {
		throw new camera_init_error("Unable to create average/sum image buffer.");
	}

	const long len = 65536; // length of LUT
	unsigned short nLUT[len];  // LUT

	//set exposure time
	float ft = this -> m_pxd.GetFramePeriod(this -> m_camType);
	this -> m_pxd.SetFramePeriod(this -> m_camType, exposureTimeSeconds);
	ft = this -> m_pxd.GetFramePeriod(this -> m_camType);

	this -> m_pxd.SetCameraConfig(this -> m_hFrameGrabber, this -> m_camType);
	this -> m_pxd.ContinuousStrobes(this -> m_hFrameGrabber, TRUE);  // turn on camera frame sync

	// initialize input LUT to shift image data down by two bits
	for (int i = 0; i < len; i++)
	{
		nLUT[i] = i>>2;
	}
	this -> m_pxd.SetInputLUT(this -> m_hFrameGrabber, 16, 0, 0, len, nLUT);

    this -> m_pixMap = (PIXMAPENTRY*)GlobalAlloc(0, SCMAXPIXMAPSIZE* sizeof(PIXMAPENTRY));
    ScReadPixMap(this -> m_pixMapFilePath, this -> m_pixMap, &this -> m_pixMapEntries);
}