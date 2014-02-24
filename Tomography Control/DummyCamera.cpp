#include "stdafx.h"

#include "stdafx.h"
#include "DummyCamera.h"

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

void DummyCamera::TakeFrame(char* output_file)
{
	// TODO: Write something to disk
	Sleep((DWORD)(this -> m_exposureTimeSeconds * 1000));
}