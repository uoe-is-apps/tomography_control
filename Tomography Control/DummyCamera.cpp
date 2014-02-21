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
	this -> m_exposureTimeSeconds = exposureTimeSeconds;
}

void DummyCamera::TakeImage(char* output_file)
{
	// TODO: Write something to disk
	Sleep((DWORD)(this -> m_exposureTimeSeconds * 1000));
}