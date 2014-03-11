#include "stdafx.h"
#include "Tomography Control.h"
#include "Camera.h"
#include "Exceptions.h"

	Camera::Camera(char* directory)
{
	this -> m_directory = directory;
}

	
char *Camera::GetDirectory()
{
	return this -> m_directory;
}

char *Camera::GenerateImageFilename(FrameType frameType, u_int frame, char* fileEnding) {
	switch (frameType)
	{
	case SINGLE:
		sprintf_s(this -> filenameBuffer, FILENAME_BUFFER_SIZE - 1, "%s\\IMAGE%04d.%s", this -> m_directory, frame, fileEnding);
		break;
	case DARK:
		sprintf_s(this -> filenameBuffer, FILENAME_BUFFER_SIZE - 1, "%s\\DC%04d.%s", this -> m_directory, frame, fileEnding);
		break;
	case FLAT_FIELD:
		sprintf_s(this -> filenameBuffer, FILENAME_BUFFER_SIZE - 1, "%s\\FF%04d.%s", this -> m_directory, frame, fileEnding);
		break;
	default:
		throw new bad_frame_type_error("Unknown frame type.");
	}

	return this -> filenameBuffer;
}