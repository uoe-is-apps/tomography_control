#pragma once

class ICamera
{
public:
	virtual void SetupCamera(float exposureTimeSeconds) = 0;
	virtual void TakeImage(char* filename) = 0;
};