#include "stdafx.h"

// #include <acq.h>
#include "PerkinElmerXrd.h"

	PerkinElmerXrd::PerkinElmerXrd(char* camFile)
{
	this -> m_acquisitionBuffer = NULL;
	this -> m_offsetBuffer = NULL;

	this -> m_endAcquisitionEvent.ResetEvent();
	this -> m_endFrameEvent.ResetEvent();

	/*
	CHwHeaderInfo headInfo;

	memset(&this -> m_hAcqDesc, 0, sizeof(HACQDESC));
	if (!Acquisition_GbIF_Init(&this -> m_hAcqDesc, this -> m_nChannelNr,
		0,
		0, 0, // Rows and columns - these are retrieved from the device
		TRUE, FALSE, // Self init and always open
		HIS_GbIF_IP, ""))
	{
		// TODO: Check detector initialised successfully 
	}

	// Do we need to call Acquisition_Init as well?

	this -> m_detectorInitialised = TRUE;

	if (!Acquisition_GetHwHeaderInfo(&this -> m_hAcqDesc, &headInfo))
	{
		// TODO: Report error
	}
	
	this -> m_nHeight = headInfo.dwNrRows;
	this -> m_nWidth = headInfo.dwNrColumns;

	Acquisition_SetCallbacksAndMessages(&this -> m_hAcqDesc,
		hWnd?,
		"Error during image acquisition",
		"Losing frames",
		endFrameCallback,
		endAcquisitionCallback);
		
	this -> m_acquisitionBuffer = malloc(sizeof(unsigned short) * this -> m_nWidth * this -> m_nHeight);

	Acquisition_DefineDestBuffers();
	*/

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


void PerkinElmerXrd::SetupCamera(float exposureTime)
{
}

void PerkinElmerXrd::CaptureFrame(char* output_file)
{
	/*
	// TODO: Set acquisition buffer here

	Acquisition_Acquire_Image(this -> m_hAcqDesc,
		1, 0, // Frames, skip frames
		HIS_SEQ_ONE_BUFFER,
		NULL, NULL, NULL // Offset, gain, pixel correction
	);

	// TODO: Wait for acquisition completed event to fire

	// TODO: Write image to disk

	Acquisition_SetReady(&this -> m_hAcqDesc, 1);

	*/

}

void PerkinElmerXrd::CaptureDarkImage(char* output_file)
{
	/* unsigned short *offsetData;

	offsetData = (unsigned short *)malloc(sizeof(unsigned short) * this -> m_nHeight * this -> m_nWidth);
	if (NULL == offsetData)
	{
		// TODO: Handle problems allocating buffer
	}

	Acquisition_Acquire_OffsetImage(this -> m_hAcqDesc,
		offsetData,
		this -> m_nHeight, this -> m_nWidth,
		1 // Frames
	);

	free(offsetData); */
}

void PerkinElmerXrd::CaptureFlatField(char* output_file)
{
	/* DWORD *offsetData;

	offsetData = (DWORD *)malloc(sizeof(DWORD) * this -> m_nHeight * this -> m_nWidth);
	if (null == offsetData)
	{
		// TODO: Handle problems allocating buffer
	}

	Acquisition_Acquire_GainImage(this -> m_hAcqDesc,
		offsetData,
		this -> m_nHeight, this -> m_nWidth,
		1 // Frames
	);

	free(offsetData); */
}

/* 
CALLBACK PerkinElmerXrd::endAcquisitionCallback(HACQDESC hAcqDesc)
{
	this -> m_endAcquisitionEvent.PulseEvent();
}

CALLBACK PerkinElmerXrd::endFrameCallback(HACQDESC hAcqDesc)
{
	this -> m_endFrameEvent.PulseEvent();
}
*/