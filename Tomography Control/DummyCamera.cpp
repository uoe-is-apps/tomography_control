#include "stdafx.h"
#include "Tomography Control.h"
#include "Camera.h"
#include "Exceptions.h"

#include "tiffio.h"
#include "tiff.h"

	DummyCamera::DummyCamera(CString directory, float exposureTimeSeconds) : Camera(directory)
{
	this -> m_nWidth = 1024;
	this -> m_nHeight = 2048;

	assert (exposureTimeSeconds > 0.000);

	this -> m_exposureTimeSeconds = exposureTimeSeconds;
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
	
	if (captureType == SUM
		|| captureType == AVERAGE)
	{
		unsigned short *sumAverageBuffer = frameBuffer;
		unsigned short *sumAverageBufferPtr = sumAverageBuffer;
		unsigned int pixelCount = GetImageHeight() * GetImageWidth();
		unsigned int maxSum = 0;
		unsigned int rightShift = 0;

		// Generate filename for end of capture file, incase we need it
		filename = GenerateImageFilename(frameType, *current_position);
		filepath = GenerateImagePath(filename);

		// Notify the dialog of the updated filename
		window -> PostMessage(WM_USER_CAPTURING_FRAME, 0, (LPARAM)filename);

		switch (captureType)
		{
		case SUM:
			CalculatePixelSums(sumAverageBuffer, this -> m_sumFrame);
			break;
		case AVERAGE:
			CalculatePixelAverages(sumAverageBuffer, this -> m_sumFrame, capturedImages);
			break;
		}

		this -> WriteTiff(filepath, sumAverageBuffer);
	
		window -> PostMessage(WM_USER_FRAME_CAPTURED, 0, (LPARAM)(*current_position));
		(*current_position)++;
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

void DummyCamera::SetupCamera()
{
	this -> m_sumFrame = (unsigned int *)malloc(sizeof(unsigned int) * this -> GetImageWidth() * this -> GetImageHeight());

	// Report error if we can't allocate buffer

	if (NULL == this -> m_sumFrame)
	{
		throw camera_init_error("Could not allocate image sum buffer.");
	}
}