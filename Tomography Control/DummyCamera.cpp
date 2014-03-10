#include "stdafx.h"
#include "Tomography Control.h"
#include "Camera.h"
#include "Exceptions.h"

#include "tiffio.h"
#include "tiff.h"

	DummyCamera::DummyCamera(char *directory)
{
	this -> m_directory = directory;
	this -> m_nWidth = 1024;
	this -> m_nHeight = 2048;
}

DummyCamera::~DummyCamera()
{
}

void DummyCamera::CaptureFrames(u_int frames, u_int *frameCount, FrameType frameType, CWnd* window)
{
	char filename[FILENAME_BUFFER_SIZE];

	for (u_int frame = 0; frame < frames; frame++, (*frameCount)++)
	{
		GenerateImageFilename(filename, FILENAME_BUFFER_SIZE - 1, frameType, *frameCount);
		window -> PostMessage(WM_USER_CAPTURING_FRAME, 0, (LPARAM)&filename);

		Sleep((DWORD)(this -> m_exposureTimeSeconds * 1000));
		

		TIFF* tif = TIFFOpen(filename, "w");
		TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, this -> m_nWidth);
		TIFFSetField(tif, TIFFTAG_IMAGELENGTH, this -> m_nHeight);
		TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
		TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, sizeof(WORD));
		TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);

		TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);


		tdata_t rowData = _TIFFmalloc(this -> m_nWidth * 2);
		for (u_int row = 0; row < this -> m_nHeight; row++)
		{
			// TODO: Generate random data

			TIFFWriteScanline(tif, rowData, row);
		}

		_TIFFfree(rowData);

		TIFFClose(tif);

		window -> PostMessage(WM_USER_FRAME_CAPTURED, 0, (LPARAM)(*frameCount));
	}
}

int DummyCamera::GenerateImageFilename(char* buffer, size_t maxLength, FrameType frameType, u_int frame) {
	switch (frameType)
	{
	case DARK:
		return sprintf_s(buffer, maxLength, "%s\\DC%04d.tiff", this -> m_directory, frame);
	case FLAT_FIELD:
		return sprintf_s(buffer, maxLength, "%s\\FF%04d.tiff", this -> m_directory, frame);
	default:
		return sprintf_s(buffer, maxLength, "%s\\IMAGE%04d.tiff", this -> m_directory, frame);
	}
}

void DummyCamera::SetupCamera(float exposureTimeSeconds)
{
	assert (exposureTimeSeconds > 0.000);

	this -> m_exposureTimeSeconds = exposureTimeSeconds;
}