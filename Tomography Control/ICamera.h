
class ICamera
{
public:
	virtual void SetupCamera(float exposureTime) = 0;
	virtual void TakeImage(char* filename) = 0;
};