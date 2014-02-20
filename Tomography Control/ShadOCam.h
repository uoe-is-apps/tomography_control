
#include "iframe.h"
#include "pxd.h"
#include "Scilib20.h"

class ShadOCam
{
public:
	ShadOCam(char* camFile);
	~ShadOCam();

	int m_nWidth;			// width of image
	int m_nHeight;			// height of image

protected:
	PXD m_pxd;				// pxd library structure
	FRAMELIB m_framelib;	// frame library structure

	long m_hFrameGrabber;	// adress of frame grabber
	long m_qh;				// handle for grab

	CAMERA_TYPE* m_camType;	// pointer to camera object

	FRAME* m_currentFrame;	// pointer to FRAME object
	short* m_currentFramePtr;	// pointer to start of frame
	FRAME* m_newFrame;

	BOOL m_bPxdLoaded;
	BOOL m_bFramelibLoaded;
	BOOL m_bCamTypeLoaded;
	BOOL m_bFrameGrabberAllocated;
};