#include <iostream>
#include <fstream>

#include "stdafx.h"

#include "stdafx.h"
#include "ShadOCam.h"

	ShadOCam::ShadOCam(char* camFile)
{
	this -> m_bPxdLoaded = FALSE;
	this -> m_bCamTypeLoaded = FALSE;
	this -> m_bFramelibLoaded = FALSE;
	this -> m_bFrameGrabberAllocated = FALSE;

	this -> m_nWidth  = 2048;		// width of image
	this -> m_nHeight = 2000;		// height of image

	this -> m_currentFrame = NULL;

	// Load the image capture libraries
	if ( !imagenation_OpenLibrary("pxd_32.dll", &this -> m_pxd, sizeof(PXD)) )  {
		throw "Frame grabber library not loaded.";
	}
	this -> m_bPxdLoaded = TRUE;

	if ( !imagenation_OpenLibrary("frame_32.dll", &this -> m_framelib, sizeof(FRAMELIB)) )  {
		throw "Frame library not loaded.";
	}
	this -> m_bFramelibLoaded = TRUE;

	// request access to frame grabber
	if ( !(this -> m_hFrameGrabber = this -> m_pxd.AllocateFG(-1)) )  {
		throw "PXD frame grabber not found.";
	}
	this -> m_bFrameGrabberAllocated = TRUE;

	// initialize camera configuration
	if ( !(this -> m_camType = this -> m_pxd.LoadConfig(camFile)) )  {
		throw "Camera configuration not loaded.";
	}
	this -> m_bCamTypeLoaded = TRUE;
	
	int frameGrabberWidth = this -> m_pxd.GetWidth(this -> m_hFrameGrabber);
	int frameGrabberHeight = this -> m_pxd.GetHeight(this -> m_hFrameGrabber);

	// set up image destination buffers
	if ( !(this -> m_currentFrame = this -> m_pxd.AllocateBuffer (frameGrabberWidth, frameGrabberHeight, PBITS_Y16)) )  {
		throw "Unable to create image buffer.";
	}
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


void ShadOCam::SetupCamera(float exposureTime)
{
	const long len = 65536; // length of LUT
	unsigned short nLUT[len];  // LUT

	//set exposure time
	float ft = this -> m_pxd.GetFramePeriod(this -> m_camType);
	this -> m_pxd.SetFramePeriod(this -> m_camType, exposureTime);
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

void ShadOCam::TakeImage(char* output_file)
{
	long qh;				// handle for grab

	// grab
    qh = this -> m_pxd.Grab(this -> m_hFrameGrabber, this -> m_currentFrame, 0);
    if (!qh)
    {
      	throw "Unable to acquire image.";
	}
      
    //save file
	strcat(output_file, ".crude");
	this -> m_framelib.WriteBin(this -> m_currentFrame, output_file, 1);
}