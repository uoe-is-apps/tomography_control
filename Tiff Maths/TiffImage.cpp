
#include "stdafx.h"

#include "tiffio.h"
#include "tiff.h"

#include "Exceptions.h"
#include "TiffImage.h"

		TiffImage::TiffImage(char *filename)
{
	this -> buffer = NULL;

	void *image;

	TIFF* tif = TIFFOpen(filename, "r");
    if (tif)
	{
		tdata_t buf;
		tstrip_t strip;
		
		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &this -> width);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &this -> height);

		buf = _TIFFmalloc(TIFFStripSize(tif));
		image = malloc(TIFFStripSize(tif) * TIFFNumberOfStrips(tif));

		for (strip = 0; strip < TIFFNumberOfStrips(tif); strip++)
		{
			TIFFReadEncodedStrip(tif, strip, buf, (tsize_t) -1);
			memcpy(((char *)image) + (strip * TIFFStripSize(tif)), buf, TIFFStripSize(tif));
		}

		_TIFFfree(buf);
		TIFFClose(tif);
    }
	else
	{
		throw image_error("Could not open TIFF image.");
	}

	this -> buffer = (unsigned short *)image;
}

		TiffImage::~TiffImage()
{
	if (NULL != this -> buffer)
	{
		free(this -> buffer);
	}
}

