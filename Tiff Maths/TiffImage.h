#pragma once

#include <stdlib.h>

#include "tiff.h"

class TiffImage
{
public:
	TiffImage(char *filename);
	~TiffImage();

	uint32 width;
	uint32 height;
	unsigned short *buffer;
};