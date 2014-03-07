#include "stdafx.h"
#include "Tomography Control.h"
#include "Camera.h"
#include "Exceptions.h"

	ShadOCam::ShadOCam(char* directory, char* camFile, float exposureTimeSeconds)
{
	this -> m_directory = directory;

	this -> m_bPxdLoaded = FALSE;
	this -> m_bCamTypeLoaded = FALSE;
	this -> m_bFramelibLoaded = FALSE;
	this -> m_bFrameGrabberAllocated = FALSE;

	this -> m_nWidth  = 2048;		// width of image
	this -> m_nHeight = 2000;		// height of image

	this -> m_currentFrame = NULL;

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
	if ( !(this -> m_camType = this -> m_pxd.LoadConfig(camFile)) )  {
		throw new camera_init_error("Camera configuration not loaded.");
	}
	this -> m_bCamTypeLoaded = TRUE;
	
	int frameGrabberWidth = this -> m_pxd.GetWidth(this -> m_hFrameGrabber);
	int frameGrabberHeight = this -> m_pxd.GetHeight(this -> m_hFrameGrabber);

	// set up image destination buffers
	if ( !(this -> m_currentFrame = this -> m_pxd.AllocateBuffer (frameGrabberWidth, frameGrabberHeight, PBITS_Y16)) )  {
		throw new camera_init_error("Unable to create image buffer.");
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
}

ShadOCam::~ShadOCam()
{
	if (this -> m_bPxdLoaded)
	{
		if (this -> m_bFramelibLoaded)
		{
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

void ShadOCam::CaptureFrames(u_int frames, u_int *frameCount, FrameType frameType, CWnd* window)
{
	long qh; // handle for grab; only needed to confirm image was taken
			 // successfully, does not reserve memory for the grab.
	char filename[FILENAME_BUFFER_SIZE];

	for (u_int frame = 0; frame < frames; frame++, *frameCount++)
	{
		GenerateImageFilename(filename, FILENAME_BUFFER_SIZE - 1, frameType, (*frameCount)++);
		window -> PostMessage(WM_USER_CAPTURING_FRAME, 0, (LPARAM)&filename);
		
		// grab
		qh = this -> m_pxd.Grab(this -> m_hFrameGrabber, this -> m_currentFrame, 0); // 0 indicates the method should not return until the image has been captured
		if (!qh)
		{
      		throw "Unable to acquire image.";
		}
      
		//save file
		this -> m_framelib.WriteBin(this -> m_currentFrame, filename, 1);

		window -> PostMessage(WM_USER_FRAME_CAPTURED, 0, (LPARAM)(*frameCount));
	}
}

int ShadOCam::GenerateImageFilename(char* buffer, size_t maxLength, FrameType frameType, u_int frame) {
	switch (frameType)
	{
	case SINGLE:
		return sprintf_s(buffer, maxLength, "%s\\IMAGE%04d.raw", this -> m_directory, frame);
	case DARK:
		return sprintf_s(buffer, maxLength, "%s\\DC%04d.raw", this -> m_directory, frame);
	case FLAT_FIELD:
		return sprintf_s(buffer, maxLength, "%s\\FF%04d.raw", this -> m_directory, frame);
	default:
		throw new bad_frame_type_error("Unknown frame type.");
	}
}