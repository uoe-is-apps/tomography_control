#include "stdafx.h"
#include "Tomography Control.h"
#include "Camera.h"
#include "Exceptions.h"

	DummyCamera::DummyCamera(char *directory, float exposureTimeSeconds)
{
	assert (exposureTimeSeconds > 0.000);

	this -> m_directory = directory;
	this -> m_exposureTimeSeconds = exposureTimeSeconds;
}

DummyCamera::~DummyCamera()
{
}

void DummyCamera::CaptureFrames(u_int frames, FrameType frameType, CWnd* window)
{
	char filename[FILENAME_BUFFER_SIZE];

	for (u_int frame = 0; frame < frames; frame++)
	{
		GenerateImageFilename(filename, FILENAME_BUFFER_SIZE - 1, frameType, frame);
		window -> PostMessage(WM_USER_CAPTURING_FRAME, 0, (LPARAM)&filename);

		// TODO: Write something to disk
		Sleep((DWORD)(this -> m_exposureTimeSeconds * 1000));

		window -> PostMessage(WM_USER_FRAME_CAPTURED, 0, (LPARAM)frame);
	}
}

int DummyCamera::GenerateImageFilename(char* buffer, size_t maxLength, FrameType frameType, u_int frame) {
	switch (frameType)
	{
	case SINGLE:
		return sprintf_s(buffer, maxLength, "%s\\IMAGE%04d.tiff", this -> m_directory, frame);
	case DARK:
		return sprintf_s(buffer, maxLength, "%s\\DC%04d.tiff", this -> m_directory, frame);
	case FLAT_FIELD:
		return sprintf_s(buffer, maxLength, "%s\\FF%04d.tiff", this -> m_directory, frame);
	default:
		throw new bad_frame_type_error("Unknown frame type.");
	}
}