#include "stdafx.h"
#include "Tomography Control.h"
#include "Camera.h"
#include "Exceptions.h"

#include "tiffio.h"
#include "tiff.h"

	DummyCamera::DummyCamera(CString directory) : Camera(directory)
{
	this -> m_nWidth = 1024;
	this -> m_nHeight = 2048;
}

DummyCamera::~DummyCamera()
{
	if (NULL != this -> m_sumFrame)
	{
		free(this -> m_sumFrame);
	}
}

void DummyCamera::CaptureFrames(u_int frames, u_int *current_position,
	FrameSavingOptions captureType, FrameType frameType, CWnd* window)
{
	unsigned short capturedImages = 0;
	char *filename;
	char *filepath;
	BOOL lastPixelAverageValid = FALSE;
	double lastPixelAverage;
	unsigned short *frameBuffer = (unsigned short *)calloc(sizeof(unsigned short), this -> GetImageWidth() * this -> GetImageHeight());

	// Should handle out of memory condition when allocating buffer
	memset(this -> m_sumFrame, 0, sizeof(unsigned short) * this -> GetImageWidth() * this -> GetImageHeight());

	for (u_int frame = 0; frame < frames; frame++, (*current_position)++)
	{
		// Verify the beam is still active, by checking pixel average against last value for
		// this set.
		double pixelAverage = this -> CalculatePixelAverage(frameBuffer);

		// We can only check against previous value, if we actually have a previous value.
		if (lastPixelAverageValid)
		{
			double pixelRatio = pixelAverage / lastPixelAverage;
			double variation = abs(1.0 - pixelRatio);

			if (variation >= PIXEL_AVERAGE_TOLERANCE)
			{
				// Likely beam failure, skip
				continue;
			}
		}

		lastPixelAverage = pixelAverage;
		lastPixelAverageValid = TRUE;
		
		switch (captureType)
		{
		case SUM:
		case AVERAGE:
			this -> AddFrameToBuffer(this -> m_sumFrame, frameBuffer);
			break;
		default:
			filename = GenerateImageFilename(frameType, *current_position);
			// Notify the dialog of the updated filename
			window -> PostMessage(WM_USER_CAPTURING_FRAME, 0, (LPARAM)filename);
			strcpy_s(this -> m_filepathBuffer, MAX_PATH, this -> m_directory);
			PathAppend(this -> m_filepathBuffer, filename);

			Sleep((DWORD)(this -> m_exposureTimeSeconds * 1000));

			this -> WriteTiff(this -> m_filepathBuffer, frameBuffer);

			window -> PostMessage(WM_USER_FRAME_CAPTURED, 0, (LPARAM)(*current_position));
			break;
		}
		capturedImages++;
	}
	
	// Generate filename for end of capture file, incase we need it
	filename = GenerateImageFilename(frameType, *current_position);
	filepath = GenerateImagePath(filename);

	switch (captureType)
	{
	case SUM:
		// Notify the dialog of the updated filename
		window -> PostMessage(WM_USER_CAPTURING_FRAME, 0, (LPARAM)filename);

		this -> WriteTiff(filepath, this -> m_sumFrame);
	
		window -> PostMessage(WM_USER_FRAME_CAPTURED, 0, (LPARAM)(*current_position));
		(*current_position)++;

		break;
	case AVERAGE:
		// Notify the dialog of the updated filename
		window -> PostMessage(WM_USER_CAPTURING_FRAME, 0, (LPARAM)filename);

		unsigned int *sourceBufferPtr = this -> m_sumFrame;
		unsigned short *averageBuffer = frameBuffer;
		unsigned short *averageBufferPtr = averageBuffer;

		for (unsigned short row = 0; row < this -> GetImageHeight(); row++)
		{
			for (unsigned short col = 0; col < this -> GetImageWidth(); col++)
			{
				double sum = *(sourceBufferPtr++);

				*(averageBufferPtr++) = (unsigned short)(sum / capturedImages);
			}
		}

		this -> WriteTiff(filepath, averageBuffer);
	
		window -> PostMessage(WM_USER_FRAME_CAPTURED, 0, (LPARAM)(*current_position));
		(*current_position)++;

		break;
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
	this -> m_sumFrame = (unsigned int *)malloc(sizeof(unsigned int) * this -> GetImageWidth() * this -> GetImageHeight());

	// Report error if we can't allocate buffer
}