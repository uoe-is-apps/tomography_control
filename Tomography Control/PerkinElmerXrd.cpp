#include "stdafx.h"
#include "Tomography Control.h"
#include "Camera.h"

#include "tiffio.h"
#include "tiff.h"

#include "Exceptions.h"

PerkinElmerXrd::PerkinElmerXrd(CString directory) : Camera(directory)
{
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
		sprintf_s(this -> m_errorBuffer, ERROR_BUFFER_SIZE - 1,
			"%s fail! Error Code %d\t\t\t\t\n","Acquisition_GbIF_GetDetectorCnt", iRet);
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
			"%s fail! Error Code %d\t\t\t\t\n","Acquisition_GbIF_GetDeviceList",iRet);
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
			"%s fail! Error Code %d\t\t\t\t\n","Acquisition_GbIF_Init",iRet);
		throw camera_init_error(this -> m_errorBuffer);
	}

	this -> m_exposureTimeSeconds = exposureTimeSeconds; // TODO: Set this on the camera

	CHwHeaderInfo headInfo;
	unsigned short usTiming=0;
	unsigned short usNetworkLoadPercent=80;
	
	// Calibrate connection
	if (Acquisition_GbIF_CheckNetworkSpeed(this -> m_hAcqDesc, &usTiming, &lPacketDelay, usNetworkLoadPercent) == HIS_ALL_OK)
	{
		if (Acquisition_GbIF_SetPacketDelay(this -> m_hAcqDesc, lPacketDelay) != HIS_ALL_OK)
		{
			throw camera_init_error("Could not set packet delay.");
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

		sprintf_s(this -> m_errorBuffer, ERROR_BUFFER_SIZE - 1,
			"%s fail! Error Code %d, Board Error %d\n","Acquisition_GetHwHeaderInfo", hisError, boardError);
		throw camera_init_error(this -> m_errorBuffer);
	}
	
	this -> m_nHeight = headInfo.dwNrRows;
	this -> m_nWidth = headInfo.dwNrColumns;
	this -> m_avgSumFrame = (unsigned int *)malloc(sizeof (unsigned int) * this -> m_nHeight * this -> m_nWidth);

	if (NULL == this -> m_avgSumFrame)
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
	if (NULL != this -> m_avgSumFrame)
	{
		free (this -> m_avgSumFrame);
	}

	if (this -> m_detectorInitialised)
	{
		Acquisition_Close(this -> m_hAcqDesc);
	}
}

void PerkinElmerXrd::CaptureFrames(u_int frames, u_int *current_position,
	FrameSavingOptions captureType, FrameType frameType, CWnd* window)
{
	PerkinElmerAcquisition task;
	
	task.camera = this;
	task.endAcquisitionEvent.ResetEvent();
	task.window = window;
	task.captureType = captureType;
	task.frameType = frameType;
	task.imageCount = current_position;
	task.lastPixelAverageValid = FALSE;
	task.capturedImages = 0;

	// Warning - this will be break on a 64-bit system as it presumes a 32-bit pointer
	Acquisition_SetAcqData(this -> m_hAcqDesc, (DWORD)&task);

	// Clear the average/sum buffer
	memset(this -> m_avgSumFrame, 0, sizeof(unsigned int) * this -> m_nWidth * this -> m_nHeight);
	
	// We have to allocate the acquisition buffer here, as we need to know how many frames
	// we're taking, before we can allocate it.
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

		sprintf_s(this -> m_errorBuffer, ERROR_BUFFER_SIZE - 1, "%s fail! Error Code %d, Board Error %d\n", "Acquisition_DefineDestBuffers", hisError, boardError);
		throw camera_acquisition_error(this -> m_errorBuffer);
	}
	Acquisition_Acquire_Image(this -> m_hAcqDesc,
		frames, 0, // Frames, skip frames
		HIS_SEQ_ONE_BUFFER,
		NULL, NULL, NULL // Offset, gain, pixel correction
	);

	// TODO: Handle timeout on acquisition
	long millisPerFrame = (long)(this -> m_exposureTimeSeconds * 1000);
	long expectedTimeMillis = millisPerFrame * frames;
	::WaitForSingleObject(task.endAcquisitionEvent.m_hObject, expectedTimeMillis * 2);

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
	char *filename;
	char *filepath;
	DWORD dwAcqData;
	PerkinElmerXrd *camera;
	PerkinElmerAcquisition *task;

	Acquisition_GetAcqData(hAcqDesc, &dwAcqData);
	task = (PerkinElmerAcquisition *)dwAcqData;
	camera = task -> camera;
	
	filename = camera -> GenerateImageFilename(task -> frameType, *task -> imageCount);
	filepath = camera -> GenerateImagePath(filename);

	switch (task -> captureType)
	{
	case SUM:
		// Notify the dialog of the updated filename
		task -> window -> PostMessage(WM_USER_CAPTURING_FRAME, 0, (LPARAM)filename);

		camera -> WriteTiff(filepath, camera -> m_avgSumFrame);
	
		task -> window -> PostMessage(WM_USER_FRAME_CAPTURED, 0, (LPARAM)(*task -> imageCount));
		(*task -> imageCount)++;
		break;
	case AVERAGE:
		unsigned int *sourceBufferPtr = camera -> m_avgSumFrame;
		unsigned short *averageBuffer = (unsigned short *)malloc(camera -> GetImageWidth() * camera -> GetImageHeight() * sizeof(unsigned short));
		unsigned short *averageBufferPtr = averageBuffer;

		for (unsigned short row = 0; row < camera -> GetImageHeight(); row++)
		{
			for (unsigned short col = 0; col < camera -> GetImageWidth(); col++)
			{
				double sum = *(sourceBufferPtr++);

				*(averageBufferPtr++) = (unsigned short)(sum / task -> capturedImages);
			}
		}
		
		// Notify the dialog of the updated filename
		task -> window -> PostMessage(WM_USER_CAPTURING_FRAME, 0, (LPARAM)filename);

		camera -> WriteTiff(filepath, averageBuffer);
	
		task -> window -> PostMessage(WM_USER_FRAME_CAPTURED, 0, (LPARAM)(*task -> imageCount));
		(*task -> imageCount)++;

		free(averageBuffer);

		break;
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
	Acquisition_GetActFrame(hAcqDesc, &dwActFrame, &dwSecFrame);
	
	task = (PerkinElmerAcquisition *)dwAcqData;
	camera = task -> camera;
	
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
			// Likely beam failure, skip
			return;
		}
	}

	task -> lastPixelAverage = pixelAverage;
	task -> lastPixelAverageValid = TRUE;

	unsigned int *avgSumBufferPtr = camera -> m_avgSumFrame;

	switch (task -> captureType)
	{
	case SUM:
	case AVERAGE:
		camera -> AddFrameToBuffer(camera -> m_avgSumFrame, frameBuffer);
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
	task -> capturedImages++;
}
