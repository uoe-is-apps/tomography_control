#include "stdafx.h"
#include "Camera.h"

	DummyCamera::DummyCamera()
{

}

DummyCamera::~DummyCamera()
{
}


void DummyCamera::SetupCamera(float exposureTimeSeconds)
{
	assert (exposureTimeSeconds > 0.001);
	this -> m_exposureTimeSeconds = exposureTimeSeconds;
}

void DummyCamera::CaptureFrame(char* output_file)
{
	// TODO: Write something to disk
	Sleep((DWORD)(this -> m_exposureTimeSeconds * 1000));
}

void DummyCamera::CaptureDarkImage(char* output_file)
{
	DummyCamera::CaptureFrame(output_file);
}

void DummyCamera::CaptureFlatField(char* output_file)
{
	DummyCamera::CaptureFrame(output_file); 
}