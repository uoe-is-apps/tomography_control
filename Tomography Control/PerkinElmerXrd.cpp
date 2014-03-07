#include "stdafx.h"
#include "Tomography Control.h"
#include "Camera.h"

#include "tiffio.h"
#include "tiff.h"

#include "Exceptions.h"

PerkinElmerXrd::PerkinElmerXrd(char* directory, float exposureTimeSeconds, CString macAddress)
{
	unsigned char *macAddressVal = (unsigned char *)alloca(macAddress.GetLength() * sizeof(unsigned char));

	// Convert the signed characters in the CString to unsigned
	for (int charIdx = 0; charIdx < macAddress.GetLength(); charIdx++)
	{
		macAddressVal[charIdx] = macAddress.GetAt(charIdx);
	}

	this -> m_directory = directory;
	this -> m_exposureTimeSeconds = exposureTimeSeconds; // TODO: Set this on the camera

	CHwHeaderInfo headInfo;

	memset(&this -> m_hAcqDesc, 0, sizeof(HACQDESC));
	if (!Acquisition_GbIF_Init(&this -> m_hAcqDesc, this -> m_nChannelNr,
		0,
		0, 0, // Rows and columns - these are retrieved from the device
		TRUE, FALSE, // Self init and always open
		HIS_GbIF_MAC, macAddressVal))
	{
		// sprintf(strBuffer,"%s fail! Error Code %d\t\t\t\t\n","Acquisition_GbIF_Init",iRet);
		throw new camera_init_error("Could not initialise camera.");
	}
	
	// TODO: Pull camera mode from network speed tests
	Acquisition_SetCameraMode(this-> m_hAcqDesc, 1);
	Acquisition_SetFrameSyncMode(this -> m_hAcqDesc, HIS_SYNCMODE_FREE_RUNNING);

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
}

PerkinElmerXrd::~PerkinElmerXrd()
{
	if (this -> m_detectorInitialised)
	{
		Acquisition_Close(&this -> m_hAcqDesc);
	}
}

void PerkinElmerXrd::CaptureFrames(u_int frames, u_int *frameCount, FrameType frameType, CWnd* window)
{
	PerkinElmerAcquisition task;
	
	task.camera = this;
	task.directory = this -> m_directory;
	task.endAcquisitionEvent.ResetEvent();
	task.window = window;
	task.frameType = frameType;
	task.frameCount = frameCount;

	// Warning - this will be break on a 64-bit system as it presumes a 32-bit pointer
	Acquisition_SetAcqData(this -> m_hAcqDesc, (DWORD)&task);
	
	task.acquisitionBuffer = (unsigned short*)malloc(this -> m_nWidth * this -> m_nHeight * sizeof(unsigned short) * frames);
	if (NULL == task.acquisitionBuffer)
	{
		// TODO: Throw an exception
		return;
	}
	
	Acquisition_DefineDestBuffers(this -> m_hAcqDesc,
		task.acquisitionBuffer,
		frames, this -> m_nHeight, this -> m_nWidth);
	Acquisition_Acquire_Image(this -> m_hAcqDesc,
		frames, 0, // Frames, skip frames
		HIS_SEQ_ONE_BUFFER,
		NULL, NULL, NULL // Offset, gain, pixel correction
	);

	// TODO: Handle timeout
	long millisPerFrame = (long)(this -> m_exposureTimeSeconds * 1000);
	long expectedTimeMillis = millisPerFrame * frames;
	::WaitForSingleObject(task.endAcquisitionEvent.m_hObject, expectedTimeMillis * 2);

	free(task.acquisitionBuffer);
}

int PerkinElmerXrd::GenerateImageFilename(char* buffer, size_t maxLength, FrameType frameType, u_int frame) {
	switch (frameType)
	{
	case SINGLE:
		return sprintf_s(buffer, maxLength, "%s\\IMAGE%04d.tiff", this -> m_directory, frame);
	case DARK:
		return sprintf_s(buffer, maxLength, "%s\\DC%04d.tiff", this -> m_directory, frame);
	case FLAT_FIELD:
		return sprintf_s(buffer, maxLength, "%s\\FF%04d.tiff", this -> m_directory, frame);
	default:
		throw new bad_frame_type_error("Unknown frame type.");
	}
}

void CALLBACK OnEndAcquisitionPEX(HACQDESC hAcqDesc)
{
	DWORD dwAcqData;
	PerkinElmerAcquisition *task;

	Acquisition_GetAcqData(hAcqDesc, &dwAcqData);
	task = (PerkinElmerAcquisition *)dwAcqData;

	task -> endAcquisitionEvent.PulseEvent();
}

void CALLBACK OnEndFramePEX(HACQDESC hAcqDesc)
{
	char filename[FILENAME_BUFFER_SIZE];
	DWORD dwAcqData, dwActFrame, dwSecFrame;
	PerkinElmerXrd *camera;
	PerkinElmerAcquisition *task;

	Acquisition_GetAcqData(hAcqDesc, &dwAcqData);
	Acquisition_GetActFrame(hAcqDesc, &dwActFrame, &dwSecFrame);
	
	task = (PerkinElmerAcquisition *)dwAcqData;
	camera = task -> camera;
	
	camera -> GenerateImageFilename(filename, FILENAME_BUFFER_SIZE - 1, task -> frameType, *task -> frameCount);

	TIFF* tif = TIFFOpen(filename, "w");
	TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, camera -> m_nWidth);
	TIFFSetField(tif, TIFFTAG_IMAGELENGTH, camera -> m_nHeight);
	TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
	TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, sizeof(WORD));
	TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);

	TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);


	tdata_t rowData = _TIFFmalloc(camera -> m_nWidth * 2);
	for (u_int row = 0; row < camera -> m_nHeight; row++)
	{
		WORD *sourceRowStart = task -> acquisitionBuffer + (row * camera -> m_nWidth * 2);
		memcpy(rowData, sourceRowStart, sizeof(rowData));

		TIFFWriteScanline(tif, rowData, row);
	}

	_TIFFfree(rowData);

    TIFFClose(tif);
	
	task -> window -> PostMessage(WM_USER_FRAME_CAPTURED, 0, (LPARAM)(*task -> frameCount));
	(*task -> frameCount)++;

	// Acquisition_SetReady(hAcqDesc, 1);
}
