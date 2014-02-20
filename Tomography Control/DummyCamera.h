
#include "ICamera.h"

class DummyCamera : ICamera
{
public:
	DummyCamera();
	~DummyCamera();

	virtual void SetupCamera(float exposureTime);
	virtual void TakeImage(char* filename);
};
