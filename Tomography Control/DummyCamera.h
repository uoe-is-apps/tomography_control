
#include "ICamera.h"

class DummyCamera : public ICamera
{
public:
	DummyCamera();
	~DummyCamera();

	virtual void SetupCamera(float exposureTime);
	virtual void CaptureFrame(char* filename);
	virtual void CaptureDarkImage(char* filename);
	virtual void CaptureFlatField(char* filename);

protected:
	float m_exposureTimeSeconds;
};
