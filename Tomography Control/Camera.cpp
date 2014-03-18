#include "stdafx.h"
#include "Tomography Control.h"
#include "Camera.h"
#include "Exceptions.h"

	Camera::Camera(char* directory)
{
	this -> m_directory = directory;
}

void Camera::AddFrameToBuffer(unsigned int *dest, unsigned short *src)
{
	for (unsigned short row = 0; row < this -> GetImageHeight(); row++)
	{
      	for (unsigned short col = 0; col < this -> GetImageWidth(); col++)
      	{	
      		*(dest++) += *(src++);
      	}
	}
}
	
double Camera::CalculatePixelAverage(unsigned short *frameBuffer)
{
	long pixelSum = 0;

	for (unsigned short row = 0; row < GetImageHeight(); row++)
	{
      	for (unsigned short col = 0; col < GetImageWidth(); col++)
      	{	
      		pixelSum += *(frameBuffer++);
      	}
	}

	return pixelSum / (double)(GetImageWidth() * GetImageHeight());
}
	
char *Camera::GetDirectory()
{
	return this -> m_directory;
}

char *Camera::GenerateImageFilename(FrameType frameType, u_int frame, char* fileEnding) {
	switch (frameType)
	{
	case SINGLE:
		sprintf_s(this -> filenameBuffer, MAX_PATH - 1, "%s\\IMAGE%04d.%s", this -> m_directory, frame, fileEnding);
		break;
	case DARK:
		sprintf_s(this -> filenameBuffer, MAX_PATH - 1, "%s\\DC%04d.%s", this -> m_directory, frame, fileEnding);
		break;
	case FLAT_FIELD:
		sprintf_s(this -> filenameBuffer, MAX_PATH - 1, "%s\\FF%04d.%s", this -> m_directory, frame, fileEnding);
		break;
	default:
		throw new bad_frame_type_error("Unknown frame type.");
	}

	return this -> filenameBuffer;
}


void Camera::WriteTiff(char* filename, unsigned short *frameBuffer)
{
	TIFF* tif = TIFFOpen(filename, "w");
	TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, this -> GetImageWidth());
	TIFFSetField(tif, TIFFTAG_IMAGELENGTH, this -> GetImageHeight());
	TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, this -> GetImageHeight());
	TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
	TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, sizeof(unsigned short) * 8);
	TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);

	TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

	unsigned int frameSize = this -> GetImageWidth() * this -> GetImageHeight();
	TIFFWriteRawStrip(tif, 0, frameBuffer,
		frameSize * sizeof(unsigned short));

    TIFFClose(tif);
}

void Camera::WriteTiff(char* filename, unsigned int *frameBuffer)
{
	TIFF* tif = TIFFOpen(filename, "w");
	TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, this -> GetImageWidth());
	TIFFSetField(tif, TIFFTAG_IMAGELENGTH, this -> GetImageHeight());
	TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, this -> GetImageHeight());
	TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
	TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, sizeof(unsigned int) * 8);
	TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);

	TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

	unsigned int frameSize = this -> GetImageWidth() * this -> GetImageHeight();
	TIFFWriteRawStrip(tif, 0, frameBuffer,
		frameSize * sizeof(unsigned int));

    TIFFClose(tif);
}