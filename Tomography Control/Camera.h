#pragma once

#include "iframe.h"
#include "pxd.h"
#include "Scilib20.h"

class ICamera
{
public:
	virtual void CaptureFrame(char* filename) = 0;
	virtual void CaptureDarkImage(char* filename) = 0;
	virtual void CaptureFlatField(char* filename) = 0;
};

class DummyCamera : public ICamera
{
public:
	DummyCamera(float exposureTime);
	~DummyCamera();

	virtual void CaptureFrame(char* filename);
	virtual void CaptureDarkImage(char* filename);
	virtual void CaptureFlatField(char* filename);

protected:
	float m_exposureTimeSeconds;
};

class PerkinElmerXrd : public ICamera
{
public:
	PerkinElmerXrd(float exposureTime);
	~PerkinElmerXrd();

	int m_nWidth;			// width of image
	int m_nHeight;			// height of image

	virtual void CaptureFrame(char* filename);
	virtual void CaptureDarkImage(char* filename);
	virtual void CaptureFlatField(char* filename);

protected:
	void WriteTiff(char* filename, unsigned short *buffer);

	float		m_exposureTimeSeconds;
	CEvent		m_endAcquisitionEvent;
	CEvent		m_endFrameEvent;
	// HACQDESC	m_hAcqDesc;
	int			m_nChannelNr;
	unsigned short *m_acquisitionBuffer;
	DWORD		*m_offsetBuffer;
	BOOL		m_detectorInitialised;
};

class ShadOCam : public ICamera
{
public:
	ShadOCam(char* camFile, float exposureTime);
	~ShadOCam();

	int m_nWidth;			// width of image
	int m_nHeight;			// height of image

	virtual void CaptureFrame(char* filename);
	virtual void CaptureDarkImage(char* filename);
	virtual void CaptureFlatField(char* filename);

protected:
	PXD m_pxd;				// pxd library structure
	FRAMELIB m_framelib;	// frame library structure

	long m_hFrameGrabber;	// adress of frame grabber

	CAMERA_TYPE* m_camType;	// pointer to camera object

	FRAME* m_currentFrame;	// pointer to FRAME object
	short* m_currentFramePtr;	// pointer to start of frame

	BOOL m_bPxdLoaded;
	BOOL m_bFramelibLoaded;
	BOOL m_bCamTypeLoaded;
	BOOL m_bFrameGrabberAllocated;
};