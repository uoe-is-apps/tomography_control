#pragma once

class ICamera
{
public:
	virtual void SetupCamera(float exposureTimeSeconds) = 0;
	virtual void TakeFrame(char* filename) = 0;
};