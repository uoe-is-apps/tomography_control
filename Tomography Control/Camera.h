#pragma once

#include "xis/Acq.h"
#include "iframe.h"
#include "pxd.h"
#include "Scilib20.h"

enum FrameType { SINGLE, DARK, FLAT_FIELD };

class ICamera
{
public:
	/* Captures a series of frames and write them to disk.
	 *
	 * frames: number of frames to capture
	 * frameCount: counter for frames captured in a single run, used to derive filename. Will be updated
	 * as frames as captured.
	 * frameType: the type of frame, used to determine filename.
	 * window: the dialog window to notify of progress.
	 */
	virtual void CaptureFrames(u_int frames, u_int *frameCount, FrameType frameType, CWnd* window) = 0;
	virtual int GenerateImageFilename(char* buffer, size_t maxLength, FrameType frameType, u_int frame) = 0;
};

class DummyCamera : public ICamera
{
public:
	DummyCamera(char* directory, float exposureTime);
	~DummyCamera();
	
	virtual void CaptureFrames(u_int frames, u_int *frameCount, FrameType frameType, CWnd* window);
	virtual int GenerateImageFilename(char* buffer, size_t maxLength, FrameType frameType, u_int frame);
	
	u_int m_nWidth;
	u_int m_nHeight;

protected:
	char *m_directory; // Directory to write images to
	float m_exposureTimeSeconds;
};

class PerkinElmerXrd : public ICamera
{
public:
	PerkinElmerXrd(char* directory, float exposureTime);
	~PerkinElmerXrd();
	
	u_int m_nWidth;			// width of image
	u_int m_nHeight;		// height of image
	
	virtual void CaptureFrames(u_int frames, u_int *frameCount, FrameType frameType, CWnd* window);
	virtual int GenerateImageFilename(char* buffer, size_t maxLength, FrameType frameType, u_int frame);

protected:
	void WriteTiff(char* directory, WORD *buffer);
	void WriteTiff(char* directory, DWORD *buffer);

	char		m_errorBuffer[2048];
	char		*m_directory;
	float		m_exposureTimeSeconds;
	HACQDESC	m_hAcqDesc;
	int			m_nChannelNr;
	BOOL		m_detectorInitialised;
};

struct PerkinElmerAcquisition {
	PerkinElmerXrd *camera;
	
	char *directory;
	CWnd *window;
	CEvent endAcquisitionEvent;
	
	u_int *frameCount;
	FrameType frameType;
	unsigned short *acquisitionBuffer;
	unsigned short *offsetBuffer;
	DWORD *gainBuffer;
};

void CALLBACK OnEndAcquisitionPEX(HACQDESC hAcqDesc);
void CALLBACK OnEndFramePEX(HACQDESC hAcqDesc);

class ShadOCam : public ICamera
{
public:
	ShadOCam(char* directory, char* camFile, float exposureTime);
	~ShadOCam();

	int m_nWidth;			// width of image
	int m_nHeight;			// height of image
	
	virtual void CaptureFrames(u_int frames, u_int *frameCount, FrameType frameType, CWnd* window);
	virtual int GenerateImageFilename(char* buffer, size_t maxLength, FrameType frameType, u_int frame);

protected:
	char		*m_directory; // Directory to write images to

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