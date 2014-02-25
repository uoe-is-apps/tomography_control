
#include "ICamera.h"

class PerkinElmerXrd : public ICamera
{
public:
	PerkinElmerXrd();
	~PerkinElmerXrd();

	int m_nWidth;			// width of image
	int m_nHeight;			// height of image

	virtual void SetupCamera(float exposureTime);
	virtual void CaptureFrame(char* filename);
	virtual void CaptureDarkImage(char* filename);
	virtual void CaptureFlatField(char* filename);

protected:
	void WriteTiff(char* filename, unsigned short *buffer);

	CEvent		m_endAcquisitionEvent;
	CEvent		m_endFrameEvent;
	// HACQDESC	m_hAcqDesc;
	int			m_nChannelNr;
	unsigned short *m_acquisitionBuffer;
	DWORD		*m_offsetBuffer;
	BOOL		m_detectorInitialised;
};