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
	for (u_int frame = 0; frame < frames; frame++, (*frameCount)++)
	{
		char *filename = GenerateImageFilename(frameType, *frameCount);
		window -> PostMessage(WM_USER_CAPTURING_FRAME, 0, (LPARAM)filename);

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