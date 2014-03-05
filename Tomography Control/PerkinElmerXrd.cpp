#include "stdafx.h"
#include "Camera.h"

#include "tiffio.h"
#include "tiff.h"

PerkinElmerXrd::PerkinElmerXrd(float exposureTimeSeconds, GBIF_STRING_DATATYPE *ipAddress)
{
	this -> m_acquisitionBuffer = NULL;
	this -> m_exposureTimeSeconds = exposureTimeSeconds; // TODO: Set this on the camera
	this -> m_offsetBuffer = NULL;

	this -> m_captureData.m_endAcquisitionEvent.ResetEvent();
	this -> m_captureData.m_endFrameEvent.ResetEvent();

	CHwHeaderInfo headInfo;

	memset(&this -> m_hAcqDesc, 0, sizeof(HACQDESC));
	if (!Acquisition_GbIF_Init(&this -> m_hAcqDesc, this -> m_nChannelNr,
		0,
		0, 0, // Rows and columns - these are retrieved from the device
		TRUE, FALSE, // Self init and always open
		HIS_GbIF_IP, ipAddress))
	{
		// TODO: Check detector initialised successfully 
	}

	// Warning - this will be break on a 64 bit system
	Acquisition_SetAcqData(this -> m_hAcqDesc, (DWORD)&this -> m_captureData);

	// Do we need to call Acquisition_Init as well?

	this -> m_detectorInitialised = TRUE;

	if (!Acquisition_GetHwHeaderInfo(&this -> m_hAcqDesc, &headInfo))
	{
		// TODO: Report error
	}
	
	this -> m_nHeight = headInfo.dwNrRows;
	this -> m_nWidth = headInfo.dwNrColumns;

	Acquisition_SetCallbacksAndMessages(&this -> m_hAcqDesc,
		NULL,
		0, 0,
		endFrameCallback,
		endAcquisitionCallback);
	
	// Note that the buffer requires twice the width * height; this is following the
	// documentation
	this -> m_acquisitionBuffer = (unsigned short*)malloc(this -> m_nWidth * this -> m_nHeight * 2);
		
	Acquisition_DefineDestBuffers(this -> m_hAcqDesc,
		this -> m_acquisitionBuffer,
		1, // Frames,
		this -> m_nHeight, this -> m_nWidth);
}

PerkinElmerXrd::~PerkinElmerXrd()
{
	if (NULL != this -> m_offsetBuffer)
	{
		free(this -> m_offsetBuffer);
	}
	if (NULL != this -> m_acquisitionBuffer)
	{
		free(this -> m_acquisitionBuffer);
	}

	if (this -> m_detectorInitialised)
	{
		// Acquisition_Close(&this -> m_hAcqDesc);
	}
}

void PerkinElmerXrd::CaptureFrame(char* output_file)
{
	Acquisition_Acquire_Image(this -> m_hAcqDesc,
		1, 0, // Frames, skip frames
		HIS_SEQ_ONE_BUFFER,
		NULL, NULL, NULL // Offset, gain, pixel correction
	);

	// TODO: Handle timeout
	::WaitForSingleObject(this -> m_captureData.m_endAcquisitionEvent.m_hObject, 30000);
	WriteTiff(output_file, this -> m_acquisitionBuffer);

	Acquisition_SetReady(&this -> m_hAcqDesc, 1);
}

void PerkinElmerXrd::CaptureDarkImage(char* output_file)
{
	unsigned short *offsetData;

	offsetData = (unsigned short *)malloc(sizeof(unsigned short) * this -> m_nHeight * this -> m_nWidth * 2);
	if (NULL == offsetData)
	{
		// TODO: Handle problems allocating buffer
	}

	Acquisition_Acquire_OffsetImage(this -> m_hAcqDesc,
		offsetData,
		this -> m_nHeight, this -> m_nWidth,
		1 // Frames
	);
	
	WriteTiff(output_file, offsetData);

	free(offsetData);
}

void PerkinElmerXrd::CaptureFlatField(char* output_file)
{
	WORD *offsetData;
	DWORD *gainData;
	
	offsetData = (WORD *)malloc(sizeof(WORD) * this -> m_nHeight * this -> m_nWidth * 2);
	if (NULL == offsetData)
	{
		// TODO: Handle problems allocating buffer
	}
	gainData = (DWORD *)malloc(sizeof(DWORD) * this -> m_nHeight * this -> m_nWidth * 2);
	if (NULL == gainData)
	{
		// TODO: Handle problems allocating buffer
	}

	Acquisition_Acquire_GainImage(this -> m_hAcqDesc,
		offsetData, gainData,
		this -> m_nHeight, this -> m_nWidth,
		1 // Frames
	);
	
	WriteTiff(output_file, gainData);
	
	free(offsetData);
	free(gainData);
}

void PerkinElmerXrd::WriteTiff(char* output_file, WORD *buffer)
{
	TIFF* tif = TIFFOpen(output_file, "w");
	TIFFSetField (tif, TIFFTAG_IMAGEWIDTH, this -> m_nWidth);
	TIFFSetField(tif, TIFFTAG_IMAGELENGTH, this -> m_nHeight);
	TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
	TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, sizeof(WORD));
	TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);

	TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

	tdata_t rowData = _TIFFmalloc(this -> m_nWidth * 2);
	for (u_int row = 0; row < this -> m_nHeight; row++)
	{
		WORD *sourceRowStart = buffer + (row * this -> m_nWidth * 2);
		memcpy(rowData, sourceRowStart, sizeof(rowData));

		TIFFWriteScanline(tif, rowData, row);
	}
    TIFFClose(tif);
	_TIFFfree(rowData);
}

void PerkinElmerXrd::WriteTiff(char* output_file, DWORD *buffer)
{
	TIFF* tif = TIFFOpen(output_file, "w");
	TIFFSetField (tif, TIFFTAG_IMAGEWIDTH, this -> m_nWidth);
	TIFFSetField(tif, TIFFTAG_IMAGELENGTH, this -> m_nHeight);
	TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
	TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, sizeof(DWORD));
	TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);

	TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

	tdata_t rowData = _TIFFmalloc(this -> m_nWidth * 2);
	for (u_int row = 0; row < this -> m_nHeight; row++)
	{
		DWORD *sourceRowStart = buffer + (row * this -> m_nWidth * 2);
		memcpy(rowData, sourceRowStart, sizeof(rowData));

		TIFFWriteScanline(tif, rowData, row);
	}
    TIFFClose(tif);
	_TIFFfree(rowData);
}

void CALLBACK endAcquisitionCallback(HACQDESC hAcqDesc)
{
	DWORD dwAcqData;
	PerkinElmerAcquisitionData *captureData;

	Acquisition_GetAcqData(hAcqDesc, &dwAcqData);
	captureData = (PerkinElmerAcquisitionData *)dwAcqData;
	captureData -> m_endAcquisitionEvent.PulseEvent();
}

void CALLBACK endFrameCallback(HACQDESC hAcqDesc)
{
	DWORD dwAcqData;
	PerkinElmerAcquisitionData *captureData;

	Acquisition_GetAcqData(hAcqDesc, &dwAcqData);
	captureData = (PerkinElmerAcquisitionData *)dwAcqData;
	captureData -> m_endFrameEvent.PulseEvent();
}
