#include "stdafx.h"
#include "Tomography Control.h"
#include "Camera.h"
#include "Exceptions.h"

	Camera::Camera(CString directory)
{
	this -> m_directory = directory;
}

	Camera::~Camera()
{
}

void Camera::AddFrameToBuffer(unsigned int *dest, unsigned short *src)
{
	unsigned int pixelCount = GetImageHeight() * GetImageWidth();

	for (unsigned int pixel = 0; pixel < pixelCount; pixel++, dest++, src++)
	{
      	*dest += *src;
	}
}

/* Calculate the single average value for all pixels in a frame. */
double Camera::CalculatePixelAverage(unsigned short *frameBuffer)
{
	unsigned double pixelSum = 0;
	unsigned int pixelCount = GetImageHeight() * GetImageWidth();

	for (unsigned int pixel = 0; pixel < pixelCount; pixel++)
	{
      	pixelSum += *(frameBuffer + pixel);
	}

	return pixelSum / pixelCount;
}

/* Calculate the average of every pixel in a frame, based on a frame containing
 * pixel sums, and number of captured images.
 */
void Camera::CalculatePixelAverages(unsigned short *dest, unsigned int *sourceBufferPtr, unsigned short capturedImages)
{
	unsigned int pixelCount = GetImageHeight() * GetImageWidth();

	for (unsigned int pixel = 0; pixel < pixelCount; pixel++, sourceBufferPtr++)
	{
		double sum = *sourceBufferPtr;
		double average = sum / capturedImages;

		*(dest++) = (unsigned short)floor(average + 0.50);
	}
}

/* Calculate the average of every pixel in a frame, based on a frame containing
 * pixel sums, and number of captured images.
 */
void Camera::CalculatePixelSums(unsigned short *destBufferPtr, unsigned int *sourceBufferPtr)
{
	unsigned int pixelCount = GetImageHeight() * GetImageWidth();
	unsigned int maxSum = 0;
	unsigned char rightShift = 0;

	// Find the largest value, to find how much we need to shift the
	// data to fit it in 16 bits.
	for (unsigned int pixel = 0; pixel < pixelCount; pixel++)
	{
		unsigned int sum = *(sourceBufferPtr + pixel);

		if (sum > maxSum)
		{
			maxSum = sum;
		}
		// Copy unshifted data in case it's all below 0xff
		*(destBufferPtr + pixel) = (unsigned short)sum;
	}

	// Don't shift data if we don't have to
	if (maxSum <= 0xff)
	{
		return;
	}

	while ((maxSum >> rightShift) > 0xffff)
	{
		rightShift++;
	}

	// Put the shifted data into the image buffer
	for (unsigned int pixel = 0; pixel < pixelCount; pixel++)
	{
		unsigned int sum = *(sourceBufferPtr + pixel);

		*(destBufferPtr + pixel) = (unsigned short)(sum >> rightShift);
	}
}
	
CString Camera::GetDirectory()
{
	return this -> m_directory;
}

char *Camera::GenerateImageFilename(FrameType frameType, u_int frame, char* fileEnding) {
	switch (frameType)
	{
	case SINGLE:
		sprintf_s(this -> m_filenameBuffer, MAX_PATH - 1, "IMAGE%04d.%s", frame, fileEnding);
		break;
	case DARK:
		sprintf_s(this -> m_filenameBuffer, MAX_PATH - 1, "DC%04d.%s", frame, fileEnding);
		break;
	case FLAT_FIELD:
		sprintf_s(this -> m_filenameBuffer, MAX_PATH - 1, "FF%04d.%s", frame, fileEnding);
		break;
	default:
		throw bad_frame_type_error("Unknown frame type.");
	}

	return this -> m_filenameBuffer;
}

char *Camera::GenerateImagePath(char *filename)
{
	strcpy_s(this -> m_filepathBuffer, MAX_PATH - 1, this -> GetDirectory());
	PathAppend(this -> m_filepathBuffer, filename);

	return this -> m_filepathBuffer;
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
