#include "stdafx.h"
#include "Tomography Control.h"
#include "Camera.h"

	DummyCamera::DummyCamera(char *directory, float exposureTimeSeconds)
{
	assert (exposureTimeSeconds > 0.000);

	this -> m_directory = directory;
	this -> m_exposureTimeSeconds = exposureTimeSeconds;
}

DummyCamera::~DummyCamera()
{
}

void DummyCamera::CaptureFrames(CWnd* window, u_int frames)
{
	char filename[FILENAME_BUFFER_SIZE];

	for (u_int frame = 0; frame < frames; frame++)
	{
		GenerateImageFilename(filename, FILENAME_BUFFER_SIZE - 1, frame);
		window -> PostMessage(WM_USER_CAPTURING_FRAME, 0, (LPARAM)&filename);

		// TODO: Write something to disk
		Sleep((DWORD)(this -> m_exposureTimeSeconds * 1000));

		window -> PostMessage(WM_USER_FRAME_CAPTURED, 0, (LPARAM)frame);
	}
}

void DummyCamera::CaptureDarkImages(CWnd* window, u_int frames)
{
	char filename[FILENAME_BUFFER_SIZE];

	for (u_int frame = 0; frame < frames; frame++)
	{
		GenerateDarkImageFilename(filename, FILENAME_BUFFER_SIZE - 1, frame);
		window -> PostMessage(WM_USER_CAPTURING_FRAME, 0, (LPARAM)&filename);

		// TODO: Write something to disk
		Sleep((DWORD)(this -> m_exposureTimeSeconds * 1000));

		window -> PostMessage(WM_USER_FRAME_CAPTURED, 0, (LPARAM)frame);
	}
}

void DummyCamera::CaptureFlatFields(CWnd* window, u_int frames)
{
	char filename[FILENAME_BUFFER_SIZE];

	for (u_int frame = 0; frame < frames; frame++)
	{
		GenerateFlatFieldFilename(filename, FILENAME_BUFFER_SIZE - 1, frame);
		window -> PostMessage(WM_USER_CAPTURING_FRAME, 0, (LPARAM)&filename);

		// TODO: Write something to disk
		Sleep((DWORD)(this -> m_exposureTimeSeconds * 1000));

		window -> PostMessage(WM_USER_FRAME_CAPTURED, 0, (LPARAM)frame);
	}
}

int DummyCamera::GenerateImageFilename(char* buffer, size_t maxLength, u_int frame) {
	return sprintf_s(buffer, maxLength, "%s\\IMAGE%4d.tiff", this -> m_directory, frame);
}

int DummyCamera::GenerateDarkImageFilename(char* buffer, size_t maxLength, u_int frame) {
	return sprintf_s(buffer, maxLength, "%s\\DC%4d.tiff", this -> m_directory, frame);
}

int DummyCamera::GenerateFlatFieldFilename(char* buffer, size_t maxLength, u_int frame) {
	return sprintf_s(buffer, maxLength, "%s\\FF%4d.tiff", this -> m_directory, frame);
}