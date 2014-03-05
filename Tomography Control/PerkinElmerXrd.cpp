#include "stdafx.h"
#include "Tomography Control.h"
#include "Camera.h"

#include "tiffio.h"
#include "tiff.h"

PerkinElmerXrd::PerkinElmerXrd(char* directory, float exposureTimeSeconds, GBIF_STRING_DATATYPE *ipAddress)
{
	this -> m_directory = directory;
	this -> m_acquisitionBuffer = NULL;
	this -> m_exposureTimeSeconds = exposureTimeSeconds; // TODO: Set this on the camera
	this -> m_offsetBuffer = NULL;

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
	
	this -> m_endAcquisitionEvent.ResetEvent();

	// Warning - this will be break on a 64-bit system as it presumes a 32-bit pointer
	Acquisition_SetAcqData(this -> m_hAcqDesc, (DWORD)this);


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
		OnEndFramePEX,
		OnEndAcquisitionPEX);
	
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

void PerkinElmerXrd::CaptureFrames(CWnd* window, u_int frames)
{
	this -> m_window = window;

	Acquisition_Acquire_Image(this -> m_hAcqDesc,
		frames, 0, // Frames, skip frames
		HIS_SEQ_ONE_BUFFER,
		NULL, NULL, NULL // Offset, gain, pixel correction
	);

	// TODO: Handle timeout
	::WaitForSingleObject(this -> m_endAcquisitionEvent.m_hObject, 300000);
}

void PerkinElmerXrd::CaptureDarkImages(CWnd* window, u_int frames)
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
		frames
	);
	
	// TODO: Handle timeout
	::WaitForSingleObject(this -> m_endAcquisitionEvent.m_hObject, 30000);

	free(offsetData);
}

void PerkinElmerXrd::CaptureFlatFields(CWnd* window, u_int frames)
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
	
	// TODO: Handle timeout
	::WaitForSingleObject(this -> m_endAcquisitionEvent.m_hObject, 30000);
	Acquisition_Acquire_GainImage(this -> m_hAcqDesc,
		offsetData, gainData,
		this -> m_nHeight, this -> m_nWidth,
		frames
	);
	
	// TODO: Handle timeout
	::WaitForSingleObject(this -> m_endAcquisitionEvent.m_hObject, 30000);
	
	free(offsetData);
	free(gainData);
}

int PerkinElmerXrd::GenerateImageFilename(char* buffer, size_t maxLength, u_int frame) {
	return sprintf_s(buffer, maxLength, "%s\\IMAGE%4d.tiff", this -> m_directory, frame);
}

int PerkinElmerXrd::GenerateDarkImageFilename(char* buffer, size_t maxLength, u_int frame) {
	return sprintf_s(buffer, maxLength, "%s\\DC%4d.tiff", this -> m_directory, frame);
}

int PerkinElmerXrd::GenerateFlatFieldFilename(char* buffer, size_t maxLength, u_int frame) {
	return sprintf_s(buffer, maxLength, "%s\\FF%4d.tiff", this -> m_directory, frame);
}

void CALLBACK OnEndAcquisitionPEX(HACQDESC hAcqDesc)
{
	DWORD dwAcqData;
	Acquisition_GetAcqData(hAcqDesc, &dwAcqData);

	PerkinElmerXrd *camera = (PerkinElmerXrd *)dwAcqData;

	camera -> m_endAcquisitionEvent.PulseEvent();
}

void CALLBACK OnEndFramePEX(HACQDESC hAcqDesc)
{
	char filename[FILENAME_BUFFER_SIZE];
	DWORD dwAcqData, dwActFrame, dwSecFrame;

	Acquisition_GetAcqData(hAcqDesc, &dwAcqData);
	Acquisition_GetActFrame(hAcqDesc, &dwActFrame, &dwSecFrame);

	PerkinElmerXrd *camera = (PerkinElmerXrd *)dwAcqData;
	
	camera -> GenerateImageFilename(filename, FILENAME_BUFFER_SIZE - 1, dwActFrame);

	TIFF* tif = TIFFOpen(filename, "w");
	TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, camera -> m_nWidth);
	TIFFSetField(tif, TIFFTAG_IMAGELENGTH, camera -> m_nHeight);
	TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
	TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);

	TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

	
	TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, sizeof(WORD));

	tdata_t rowData = _TIFFmalloc(camera -> m_nWidth * 2);
	for (u_int row = 0; row < camera -> m_nHeight; row++)
	{
		WORD *sourceRowStart = camera -> m_acquisitionBuffer + (row * camera -> m_nWidth * 2);
		memcpy(rowData, sourceRowStart, sizeof(rowData));

		TIFFWriteScanline(tif, rowData, row);
	}

	_TIFFfree(rowData);


    TIFFClose(tif);

	camera -> m_window -> PostMessage(WM_USER_FRAME_CAPTURED, 0, (LPARAM)dwActFrame);

	Acquisition_SetReady(hAcqDesc, 1);
}
