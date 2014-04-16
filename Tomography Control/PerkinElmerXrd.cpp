#include "stdafx.h"
#include "Tomography Control.h"
#include "Camera.h"

#include "tiffio.h"
#include "tiff.h"

#include "Exceptions.h"

/* Timeout captures after 6 minutes */
#define DEFAULT_TIMEOUT 300000

PerkinElmerXrd::PerkinElmerXrd(CString directory, u_int cameraMode) : Camera(directory)
{
	if (cameraMode > 7)
	{
		sprintf_s(this -> m_errorBuffer, ERROR_BUFFER_SIZE - 1,
			"Camera mode must be between 0 and 7 inclusive; provided mode was %d.", cameraMode);
		throw camera_setting_error(this -> m_errorBuffer);
	}

	this -> m_cameraMode = cameraMode;
}	

void PerkinElmerXrd::SetupCamera()
{
	BOOL bEnableIRQ = TRUE;
	int iRet;							// Return value
	BOOL bSelfInit = TRUE;
	unsigned int dwRows=0, dwColumns=0;
	long lOpenMode = HIS_GbIF_IP;
	long lPacketDelay = 256;
	long ulNumSensors = 0;				// nr of GigE detector in network

	this -> m_sumFrame = NULL;
	this -> m_detectorInitialised = FALSE;
	
	// find GbIF Detectors in Subnet	
	iRet = Acquisition_GbIF_GetDeviceCnt(&ulNumSensors);
	if (iRet != HIS_ALL_OK)
	{
		sprintf_s(this -> m_errorBuffer, ERROR_BUFFER_SIZE - 1,
			"%s failed with error code %d\t\t\t\t\n","Acquisition_GbIF_GetDetectorCnt", iRet);
		throw camera_init_error(this -> m_errorBuffer);
	}
				
	if(ulNumSensors == 0)
	{
		throw camera_init_error("Could not detect any cameras.");
	}

	if (ulNumSensors > 1)
	{
		sprintf_s(this -> m_errorBuffer, ERROR_BUFFER_SIZE - 1,
			"Found %l cameras, expected only 1.", ulNumSensors);
		throw camera_init_error(this -> m_errorBuffer);
	}

	// get device params of GbIF Detectors in Subnet
	GBIF_DEVICE_PARAM pGbIF_DEVICE_PARAM;
			
	iRet = Acquisition_GbIF_GetDeviceList(&pGbIF_DEVICE_PARAM, 1);
	if (iRet != HIS_ALL_OK)
	{
		sprintf_s(this -> m_errorBuffer, ERROR_BUFFER_SIZE - 1,
			"%s failed with error code %d\t\t\t\t\n","Acquisition_GbIF_GetDeviceList",iRet);
		throw camera_init_error(this -> m_errorBuffer);
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
		sprintf_s(this -> m_errorBuffer, ERROR_BUFFER_SIZE - 1,
			"%s failed with error code %d\t\t\t\t\n","Acquisition_GbIF_Init",iRet);
		throw camera_init_error(this -> m_errorBuffer);
	}

	CHwHeaderInfo headInfo;
	unsigned short usTiming = 0;
	unsigned short usNetworkLoadPercent=80;
	
	// Calibrate connection
	if (Acquisition_GbIF_CheckNetworkSpeed(this -> m_hAcqDesc, &usTiming, &lPacketDelay, usNetworkLoadPercent) == HIS_ALL_OK)
	{
		if (Acquisition_GbIF_SetPacketDelay(this -> m_hAcqDesc, lPacketDelay) != HIS_ALL_OK)
		{
			DWORD hisError;
			DWORD boardError;
			Acquisition_GetErrorCode(this -> m_hAcqDesc, &hisError, &boardError);

			sprintf_s(this -> m_errorBuffer, ERROR_BUFFER_SIZE - 1,
				"%s failed with error code %d, board error %d\n", "Acquisition_GbIF_SetPacketDelay", hisError, boardError);
			throw camera_init_error(this -> m_errorBuffer);
		}
	}

	if (Acquisition_SetCameraMode(this-> m_hAcqDesc, this -> m_cameraMode) != HIS_ALL_OK)
	{
		DWORD hisError;
		DWORD boardError;
		Acquisition_GetErrorCode(this -> m_hAcqDesc, &hisError, &boardError);

		sprintf_s(this -> m_errorBuffer, ERROR_BUFFER_SIZE - 1,
			"%s failed with error code %d, board error %d\n", "Acquisition_SetCameraMode", hisError, boardError);
		throw camera_init_error(this -> m_errorBuffer);
	}
	if (Acquisition_SetFrameSyncMode(this -> m_hAcqDesc, HIS_SYNCMODE_FREE_RUNNING) != HIS_ALL_OK)
	{
		DWORD hisError;
		DWORD boardError;
		Acquisition_GetErrorCode(this -> m_hAcqDesc, &hisError, &boardError);

		sprintf_s(this -> m_errorBuffer, ERROR_BUFFER_SIZE - 1,
			"%s failed with error code %d, board error %d\n", "Acquisition_SetFrameSyncMode", hisError, boardError);
		throw camera_init_error(this -> m_errorBuffer);
	}

	this -> m_detectorInitialised = TRUE;

	if (Acquisition_GetHwHeaderInfo(this -> m_hAcqDesc, &headInfo) != HIS_ALL_OK)
	{
		DWORD hisError;
		DWORD boardError;
		Acquisition_GetErrorCode(this -> m_hAcqDesc, &hisError, &boardError);

		sprintf_s(this -> m_errorBuffer, ERROR_BUFFER_SIZE - 1,
			"%s failed with error code %d, board error %d\n","Acquisition_GetHwHeaderInfo", hisError, boardError);
		throw camera_init_error(this -> m_errorBuffer);
	}
	
	this -> m_nHeight = headInfo.dwNrRows;
	this -> m_nWidth = headInfo.dwNrColumns;
	this -> m_sumFrame = (unsigned int *)malloc(sizeof (unsigned int) * this -> m_nHeight * this -> m_nWidth);

	if (NULL == this -> m_sumFrame)
	{
		throw camera_init_error("Could not allocate buffer to hold average/sum data.");
	}

	Acquisition_SetCallbacksAndMessages(this -> m_hAcqDesc,
		NULL,
		0, 0,
		OnEndFramePEX,
		OnEndAcquisitionPEX);
}

PerkinElmerXrd::~PerkinElmerXrd()
{
	if (NULL != this -> m_sumFrame)
	{
		free (this -> m_sumFrame);
	}

	if (this -> m_detectorInitialised)
	{
		Acquisition_Close(this -> m_hAcqDesc);
	}
}

void PerkinElmerXrd::CaptureFrames(u_int frames, u_int *current_position,
	FrameSavingOptions captureType, FrameType frameType, CWnd* window,
	CTime timeoutAt)
{
	PerkinElmerAcquisition task;
	PerkinElmerAcquisition *taskPtr = &task;
	
	task.camera = this;
	task.endAcquisitionEvent.ResetEvent();
	task.window = window;
	task.captureType = captureType;
	task.frameType = frameType;
	task.imageCount = current_position;
	task.lastPixelAverageValid = FALSE;
	task.capturedFrames = 0;
	task.frames = frames;

	// Warning - this will be break on a 64-bit system as it presumes a 32-bit pointer

	Acquisition_SetAcqData(this -> m_hAcqDesc, (DWORD)taskPtr);

	// Clear the average/sum buffer
	memset(this -> m_sumFrame, 0, sizeof(unsigned int) * this -> m_nWidth * this -> m_nHeight);
	
	// We have to allocate the acquisition buffer here, as we need to know how many frames
	// we're taking, before we can allocate it.
	task.acquisitionBuffer = (unsigned short*)calloc(this -> m_nWidth * this -> m_nHeight * frames, sizeof(unsigned short));
	if (NULL == task.acquisitionBuffer)
	{
		throw camera_acquisition_error("Could not allocate image acquisition buffer.");
	}
	
	if (Acquisition_DefineDestBuffers(this -> m_hAcqDesc,
		task.acquisitionBuffer,
		frames, this -> m_nHeight, this -> m_nWidth) != HIS_ALL_OK)
	{
		DWORD hisError;
		DWORD boardError;
		Acquisition_GetErrorCode(this -> m_hAcqDesc, &hisError, &boardError);

		sprintf_s(this -> m_errorBuffer, ERROR_BUFFER_SIZE - 1, "%s failed with error code %d, board error %d\n", "Acquisition_DefineDestBuffers", hisError, boardError);
		throw camera_acquisition_error(this -> m_errorBuffer);
	}
	while (task.capturedFrames < task.frames
		&& CTime::GetCurrentTime() < timeoutAt)
	{
		Acquisition_Acquire_Image(this -> m_hAcqDesc,
			task.capturedFrames - task.frames, 0, // Frames, skip frames
			HIS_SEQ_ONE_BUFFER,
			NULL, NULL, NULL // Offset, gain, pixel correction
		);

		// TODO: Handle timeout on acquisition
		::WaitForSingleObject(task.endAcquisitionEvent.m_hObject, DEFAULT_TIMEOUT);
	}

	if (task.capturedFrames < task.frames)
	{
		// Timeout due to beam failure
		throw xray_beam_failure_error("X-ray beam failure detected. Timed out while attempting recovery.");
	}

	free(task.acquisitionBuffer);
}

u_short PerkinElmerXrd::GetImageHeight() {
	return this -> m_nHeight;
}

u_short PerkinElmerXrd::GetImageWidth() {
	return this -> m_nWidth;
}

char *PerkinElmerXrd::GenerateImageFilename(FrameType frameType, u_int frame) {
	return Camera::GenerateImageFilename(frameType, frame, "tiff");
}

void CALLBACK OnEndAcquisitionPEX(HACQDESC hAcqDesc)
{
	DWORD dwAcqData;
	PerkinElmerXrd *camera;
	PerkinElmerAcquisition *task;

	Acquisition_GetAcqData(hAcqDesc, &dwAcqData);
	task = (PerkinElmerAcquisition *)dwAcqData;
	camera = task -> camera;
	
	if (task -> capturedFrames < task -> frames)
	{
		// Need more images, pass control back to the outer loop for recovery
		task -> endAcquisitionEvent.PulseEvent();
		return;
	}

	if (task -> captureType == SUM
		|| task -> captureType == AVERAGE)
	{
		char *filename = camera -> GenerateImageFilename(task -> frameType, *task -> imageCount);
		char *filepath = camera -> GenerateImagePath(filename);

		unsigned short *sumAverageBuffer = (unsigned short *)malloc(camera -> GetImageWidth() * camera -> GetImageHeight() * sizeof(unsigned short));
		unsigned short *sumAverageBufferPtr = sumAverageBuffer;
		unsigned int pixelCount = camera -> GetImageHeight() * camera -> GetImageWidth();
		unsigned int maxSum = 0;
		unsigned int rightShift = 0;

		switch (task -> captureType)
		{
		case SUM:
			camera -> CalculatePixelSums(sumAverageBuffer, camera -> m_sumFrame);

			break;
		case AVERAGE:
			camera -> CalculatePixelAverages(sumAverageBuffer, camera -> m_sumFrame,  task -> capturedFrames);

			break;
		}
		
		// Notify the dialog of the updated filename
		task -> window -> PostMessage(WM_USER_CAPTURING_FRAME, 0, (LPARAM)filename);

		camera -> WriteTiff(filepath, sumAverageBuffer);
	
		task -> window -> PostMessage(WM_USER_FRAME_CAPTURED, 0, (LPARAM)(*task -> imageCount));
		(*task -> imageCount)++;

		free(sumAverageBuffer);
	}

	task -> endAcquisitionEvent.PulseEvent();
}

void CALLBACK OnEndFramePEX(HACQDESC hAcqDesc)
{
	char *filename;
	char *filepath;
	DWORD dwAcqData, dwActFrame, dwSecFrame;
	PerkinElmerXrd *camera;
	PerkinElmerAcquisition *task;

	Acquisition_GetAcqData(hAcqDesc, &dwAcqData);

	task = (PerkinElmerAcquisition *)dwAcqData;
	camera = task -> camera;

	if (Acquisition_GetActFrame(hAcqDesc, &dwActFrame, &dwSecFrame) != HIS_ALL_OK)
	{
		throw camera_acquisition_error("Error getting active frame.");
	}
	
	
	// Find the start of the current frame
	unsigned int frameSize = camera -> GetImageWidth() * camera -> GetImageHeight();
	unsigned short *frameBuffer
		= task -> acquisitionBuffer + ((dwSecFrame - 1) * frameSize);
	
	// Verify the beam is still active, by checking pixel average against last value for
	// this set.
	double pixelAverage = camera -> CalculatePixelAverage(frameBuffer);

	// We can only check against previous value, if we actually have a previous value.
	if (task -> lastPixelAverageValid)
	{
		double pixelRatio = pixelAverage / task -> lastPixelAverage;
		double variation = abs(1.0 - pixelRatio);

		if (variation >= PIXEL_AVERAGE_TOLERANCE)
		{
			// Likely beam failure, skip frame. Recovery is done by the control process
			task -> window -> PostMessage(WM_USER_BEAM_FAILURE, 0, NULL);
			return;
		}
	}

	task -> lastPixelAverage = pixelAverage;
	task -> lastPixelAverageValid = TRUE;

	unsigned int *avgSumBufferPtr = camera -> m_sumFrame;

	switch (task -> captureType)
	{
	case SUM:
	case AVERAGE:
		camera -> AddFrameToBuffer(camera -> m_sumFrame, frameBuffer);
		break;
	default:
		filename = camera -> GenerateImageFilename(task -> frameType, *task -> imageCount);
		filepath = camera -> GenerateImagePath(filename);

		// Notify the dialog of the updated filename
		task -> window -> PostMessage(WM_USER_CAPTURING_FRAME, 0, (LPARAM)filename);

		camera -> WriteTiff(filepath, frameBuffer);
	
		task -> window -> PostMessage(WM_USER_FRAME_CAPTURED, 0, (LPARAM)(*task -> imageCount));
		(*task -> imageCount)++;
		break;
	}
	task -> capturedFrames++;
}
