
#include "ICamera.h"
#include "iframe.h"
#include "pxd.h"
#include "Scilib20.h"

class ShadOCam : public ICamera
{
public:
	ShadOCam(char* camFile);
	~ShadOCam();

	int m_nWidth;			// width of image
	int m_nHeight;			// height of image

	virtual void SetupCamera(float exposureTime);
	virtual void TakeFrame(char* filename);

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