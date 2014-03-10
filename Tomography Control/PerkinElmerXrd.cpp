#include "stdafx.h"
#include "Tomography Control.h"
#include "Camera.h"

#include "tiffio.h"
#include "tiff.h"

#include "Exceptions.h"

PerkinElmerXrd::PerkinElmerXrd(char* directory)
{
	this -> m_directory = directory;
}	

void PerkinElmerXrd::SetupCamera(float exposureTimeSeconds)
{
	BOOL bEnableIRQ = TRUE;
	int iRet;							// Return value
	BOOL bSelfInit = TRUE;
	unsigned int dwRows=0, dwColumns=0;
	long lOpenMode = HIS_GbIF_IP;
	long lPacketDelay = 256;
	long ulNumSensors = 0;				// nr of GigE detector in network
	
	// find GbIF Detectors in Subnet	
	iRet = Acquisition_GbIF_GetDeviceCnt(&ulNumSensors);
	if (iRet != HIS_ALL_OK)
	{
		sprintf(this -> m_errorBuffer, "%s fail! Error Code %d\t\t\t\t\n","Acquisition_GbIF_GetDetectorCnt", iRet);
		throw new camera_init_error(this -> m_errorBuffer);
	}
				
	if(ulNumSensors == 0)
	{
		throw new camera_init_error("Could not detect any cameras.");
	}

	if (ulNumSensors > 1)
	{
		sprintf(this -> m_errorBuffer, "Found %l cameras, expected only 1.", ulNumSensors);
		throw new camera_init_error(this -> m_errorBuffer);
	}

	// get device params of GbIF Detectors in Subnet
	GBIF_DEVICE_PARAM pGbIF_DEVICE_PARAM;
			
	iRet = Acquisition_GbIF_GetDeviceList(&pGbIF_DEVICE_PARAM, 1);
	if (iRet != HIS_ALL_OK)
	{
		sprintf(this -> m_errorBuffer, "%s fail! Error Code %d\t\t\t\t\n","Acquisition_GbIF_GetDeviceList",iRet);
		throw new camera_init_error(this -> m_errorBuffer);
	}

	memset(&this -> m_hAcqDesc, 0, sizeof(HACQDESC));
	iRet = Acquisition_GbIF_Init(
		&this -> m_hAcqDesc,
		//iSelected,							// Index to access individual detector
		0,										// here set to zero for a single detector device
		bEnableIRQ, 
		dwRows, dwColumns,						// Image dimensions
		bSelfInit,								// retrieve settings (rows,cols.. from detector
		FALSE,									// If communication port is already reserved by another process, do not open
		lOpenMode,								// here: HIS_GbIF_IP, i.e. open by IP address 
		pGbIF_DEVICE_PARAM.ucIP		// IP address of the connection to open
		);

	if (iRet != HIS_ALL_OK)
	{
		sprintf(this -> m_errorBuffer, "%s fail! Error Code %d\t\t\t\t\n","Acquisition_GbIF_Init",iRet);
		throw new camera_init_error(this -> m_errorBuffer);
	}

	this -> m_exposureTimeSeconds = exposureTimeSeconds; // TODO: Set this on the camera

	CHwHeaderInfo headInfo;
	unsigned short usTiming=0;
	unsigned short usNetworkLoadPercent=80;
	
	// Calibrate connection
	if (Acquisition_GbIF_CheckNetworkSpeed(this -> m_hAcqDesc, &usTiming, &lPacketDelay, usNetworkLoadPercent) == HIS_ALL_OK)
	{
		printf("%s result: suggested timing: %d packetdelay %d @%d networkload\t\t\t\n"
			,"Acquisition_GbIF_CheckNetworkSpeed",usTiming,lPacketDelay,usNetworkLoadPercent);
		if (Acquisition_GbIF_SetPacketDelay(this -> m_hAcqDesc, lPacketDelay) != HIS_ALL_OK)
		{
			throw new camera_init_error("Could not set packet delay.");
		}
	}

	Acquisition_SetCameraMode(this-> m_hAcqDesc, 1);
	Acquisition_SetFrameSyncMode(this -> m_hAcqDesc, HIS_SYNCMODE_FREE_RUNNING);

	this -> m_detectorInitialised = TRUE;

	if (Acquisition_GetHwHeaderInfo(this -> m_hAcqDesc, &headInfo) != HIS_ALL_OK)
	{
		DWORD hisError;
		DWORD boardError;
		Acquisition_GetErrorCode(this -> m_hAcqDesc, &hisError, &boardError);

		sprintf(this -> m_errorBuffer, "%s fail! Error Code %d, Board Error %d\n","Acquisition_GetHwHeaderInfo", hisError, boardError);
		throw new camera_init_error(this -> m_errorBuffer);
	}
	
	this -> m_nHeight = headInfo.dwNrRows;
	this -> m_nWidth = headInfo.dwNrColumns;

	Acquisition_SetCallbacksAndMessages(this -> m_hAcqDesc,
		NULL,
		0, 0,
		OnEndFramePEX,
		OnEndAcquisitionPEX);
}

PerkinElmerXrd::~PerkinElmerXrd()
{
	if (this -> m_detectorInitialised)
	{
		Acquisition_Close(this -> m_hAcqDesc);
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
	
	task.acquisitionBuffer = (unsigned short*)calloc(this -> m_nWidth * this -> m_nHeight * frames, sizeof(unsigned short));
	if (NULL == task.acquisitionBuffer)
	{
		// TODO: Throw an exception
		return;
	}
	
	if (Acquisition_DefineDestBuffers(this -> m_hAcqDesc,
		task.acquisitionBuffer,
		frames, this -> m_nHeight, this -> m_nWidth) != HIS_ALL_OK)
	{
		DWORD hisError;
		DWORD boardError;
		Acquisition_GetErrorCode(this -> m_hAcqDesc, &hisError, &boardError);

		sprintf(this -> m_errorBuffer, "%s fail! Error Code %d, Board Error %d\n", "Acquisition_DefineDestBuffers", hisError, boardError);
		throw new camera_acquisition_error(this -> m_errorBuffer);
	}
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
	task -> window -> PostMessage(WM_USER_CAPTURING_FRAME, 0, (LPARAM)&filename);

	TIFF* tif = TIFFOpen(filename, "w");
	TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, camera -> m_nWidth);
	TIFFSetField(tif, TIFFTAG_IMAGELENGTH, camera -> m_nHeight);
	TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, camera -> m_nHeight);
	TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
	TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, sizeof(unsigned short) * 8);
	TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);

	TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
	

	// Find the start of the current frame
	unsigned int frameSize = camera -> m_nWidth * camera -> m_nHeight;
	unsigned short *frameBuffer
		= task -> acquisitionBuffer + ((dwSecFrame - 1) * frameSize);
	TIFFWriteRawStrip(tif, 0, frameBuffer,
		frameSize * sizeof(unsigned short));

    TIFFClose(tif);
	
	task -> window -> PostMessage(WM_USER_FRAME_CAPTURED, 0, (LPARAM)(*task -> frameCount));
	(*task -> frameCount)++;

	// Acquisition_SetReady(hAcqDesc, 1);
}
