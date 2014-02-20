
#include "ICamera.h"

class DummyCamera : public ICamera
{
public:
	DummyCamera();
	~DummyCamera();

	virtual void SetupCamera(float exposureTime);
	virtual void TakeImage(char* filename);
};
