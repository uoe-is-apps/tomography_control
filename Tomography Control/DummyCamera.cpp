#include "stdafx.h"
#include "Tomography Control.h"
#include "Camera.h"
#include "Exceptions.h"

#include "tiffio.h"
#include "tiff.h"

	DummyCamera::DummyCamera(char *directory) : Camera(directory)
{
	this -> m_nWidth = 1024;
	this -> m_nHeight = 2048;
}

DummyCamera::~DummyCamera()
{
}

void DummyCamera::CaptureFrames(u_int frames, u_int *frameCount, FrameType frameType, CWnd* window)
{
	unsigned short *frameBuffer = (unsigned short *)calloc(sizeof(unsigned short), this -> GetImageWidth() * this -> GetImageHeight());

	// Should handle out of memory condition

	for (u_int frame = 0; frame < frames; frame++, (*frameCount)++)
	{
		char *filename = GenerateImageFilename(frameType, *frameCount);
		window -> PostMessage(WM_USER_CAPTURING_FRAME, 0, (LPARAM)filename);

		Sleep((DWORD)(this -> m_exposureTimeSeconds * 1000));

		this -> WriteTiff(filename, frameBuffer);

		window -> PostMessage(WM_USER_FRAME_CAPTURED, 0, (LPARAM)(*frameCount));
	}

	free(frameBuffer);
}


char *DummyCamera::GenerateImageFilename(FrameType frameType, u_int frame) {
	return Camera::GenerateImageFilename(frameType, frame, "tiff");
}

u_short DummyCamera::GetImageHeight() {
	return this -> m_nHeight;
}

u_short DummyCamera::GetImageWidth() {
	return this -> m_nWidth;
}

void DummyCamera::SetupCamera(float exposureTimeSeconds)
{
	assert (exposureTimeSeconds > 0.000);

	this -> m_exposureTimeSeconds = exposureTimeSeconds;
}