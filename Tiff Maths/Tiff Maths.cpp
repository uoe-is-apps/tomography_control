// Tiff Maths.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdlib.h>

#include "TiffImage.h"

int _tmain(int argc, _TCHAR* argv[])
{
	TiffImage image1("Image2.tif");
	TiffImage image2("Image3.tif");
	TiffImage image3("Image4.tif");
	TiffImage expectedAvg("RESULT OF 3 AVERAGES.tif");
	TiffImage expectedSum("RESULT OF 3 SUMMED.tif");

	printf("Reference image %d x %d.\n", image1.width, image1.height);

	unsigned int *sumBuffer = (unsigned int *)calloc(image1.width * image1.height, sizeof(unsigned int));

	if (NULL == sumBuffer)
	{
		puts("Out of memory while attempting to allocate sum buffer.");
	}
	else
	{
		unsigned int pixel = 0;
		unsigned int pixelCount = image1.width * image1.height;

		for (int pixel = 0; pixel < pixelCount && pixel < 100; pixel++)
		{
			*(sumBuffer + pixel) += *(image1.buffer + pixel);
			*(sumBuffer + pixel) += *(image2.buffer + pixel);
			*(sumBuffer + pixel) += *(image3.buffer + pixel);

			printf("Pixel %d, result %d, expected %d.\n", pixel, *(sumBuffer + pixel), *(expectedSum.buffer + pixel));
		}
	}

	getchar();

	return 0;
}

