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

	// Load the image capture libraries
	if ( !imagenation_OpenLibrary("pxd_32.dll", &this -> m_pxd, sizeof(PXD)) )  {
		throw "Frame grabber library not loaded.";
	}
	this -> m_bPxdLoaded = TRUE;

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

	if ( !imagenation_OpenLibrary("frame_32.dll", &this -> m_framelib, sizeof(FRAMELIB)) )  {
		throw "Frame library not loaded.";
	}
	this -> m_bFramelibLoaded = TRUE;
}

ShadOCam::~ShadOCam()
{
	if (this -> m_bFramelibLoaded)
	{
		imagenation_CloseLibrary(&this -> m_framelib);
	}

	if (this -> m_bCamTypeLoaded)
	{
		this -> m_pxd.FreeConfig(this -> m_camType);
	}

	if (this -> m_bFrameGrabberAllocated)
	{
		this -> m_pxd.FreeFG(this -> m_hFrameGrabber);
	}
	if (this -> m_bPxdLoaded)
	{
		imagenation_CloseLibrary(&this -> m_pxd);
	}
}