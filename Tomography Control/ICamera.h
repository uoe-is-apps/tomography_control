#pragma once

class ICamera
{
public:
	virtual void SetupCamera(float exposureTimeSeconds) = 0;
	virtual void CaptureFrame(char* filename) = 0;
	virtual void CaptureDarkImage(char* filename) = 0;
	virtual void CaptureFlatField(char* filename) = 0;
};