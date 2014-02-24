
#include "ICamera.h"

class DummyCamera : public ICamera
{
public:
	DummyCamera();
	~DummyCamera();

	virtual void SetupCamera(float exposureTime);
	virtual void TakeFrame(char* filename);

protected:
	float m_exposureTimeSeconds;
};
