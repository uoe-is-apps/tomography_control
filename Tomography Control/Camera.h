#pragma once

#include "Tomography Control.h"
#include "xis/Acq.h"
#include "iframe.h"
#include "pxd.h"
#include "Scilib20.h"

#include "tiffio.h"
#include "tiff.h"

#define ERROR_BUFFER_SIZE 1024
#define SHAD_O_CAM_CONFIG_FILE "C:\\ShadoCam\\DEFAULT.CAM"

enum FrameType { SINGLE, DARK, FLAT_FIELD };
enum CaptureType { DEFAULT, AVERAGE, SUM };

/* Abstract class for cameras to extend. Provides basic functions for retrieving
 * information about the camera (width, height), filename creation, image sum/average
 * functions, etc.
 *
 * This class is not thread safe, and thread handling should be managed at a layer
 * above.
 */
class Camera
{
public:
	Camera::Camera(char* directory);

	/* Captures a series of frames and write them to disk.
	 *
	 * frames: number of frames to capture
	 * frameCount: counter for frames captured in a single run, used to derive filename. Will be updated
	 * as frames as captured.
	 * frameType: the type of frame, used to determine filename.
	 * window: the dialog window to notify of progress.
	 */
	virtual void CaptureFrames(u_int frames, u_int *frameCount, FrameType frameType, CWnd* window) = 0;
	/* Construct a filename for an image to be written to disk. Returns an pointer
	 * to a buffer containing the filename. This buffer is overwritten when this method
	 * is called next.
	 *
	 * frameType: the type of frame to capture (single, dark, flat_field)
	 * frame: the number of the frame, within a captured set
	 * fileEnding: the ending of the filename, to indicate file type (i.e. "tiff", "raw")
	 */
	char *GenerateImageFilename(FrameType frameType, u_int frame, char* fileEnding);
	virtual char *GenerateImageFilename(FrameType frameType, u_int frame) = 0;
	char *GetDirectory();
	virtual u_short GetImageHeight() = 0;
	virtual u_short GetImageWidth() = 0;
	virtual void SetupCamera(float exposureTime) = 0;

	void WriteTiff(char* filename, unsigned short *buffer);
	void WriteTiff(char* filename, unsigned int *buffer);

protected:
	char *m_directory; // Directory to write images to
	char filenameBuffer[FILENAME_BUFFER_SIZE];
};

class DummyCamera : public Camera
{
public:
	DummyCamera(char* directory);
	~DummyCamera();
	
	virtual void CaptureFrames(u_int frames, u_int *frameCount, FrameType frameType, CWnd* window);
	virtual char *GenerateImageFilename(FrameType frameType, u_int frame);
	virtual u_short GetImageHeight();
	virtual u_short GetImageWidth();
	virtual void SetupCamera(float exposureTime);
	
	u_short m_nWidth;
	u_short m_nHeight;

protected:
	float m_exposureTimeSeconds;
};

class PerkinElmerXrd : public Camera
{
public:
	PerkinElmerXrd(char* directory);
	~PerkinElmerXrd();
	
	virtual void CaptureFrames(u_int frames, u_int *frameCount, FrameType frameType, CWnd* window);
	virtual char *GenerateImageFilename(FrameType frameType, u_int frame);
	virtual u_short GetImageHeight();
	virtual u_short GetImageWidth();
	virtual void SetupCamera(float exposureTime);

protected:	
	u_int m_nWidth;			// width of image
	u_int m_nHeight;		// height of image
	
	/* Common buffer used when doing an acquisition for averaged frames. */
	unsigned short *m_avgBuffer;
	/* Common buffer used when doing an acquisition for summed frames. */
	unsigned int *m_sumBuffer;
	char		m_errorBuffer[ERROR_BUFFER_SIZE];
	float		m_exposureTimeSeconds;
	HACQDESC	m_hAcqDesc;
	int			m_nChannelNr;
	BOOL		m_detectorInitialised;
};

/* Structure for tracking a specific acquisition from the Perkin-Elmer
 * camera.
 */
struct PerkinElmerAcquisition {
	PerkinElmerXrd *camera;
	
	CWnd *window;
	CEvent endAcquisitionEvent;
	
	u_int *frameCount;
	FrameType frameType;
	unsigned short *acquisitionBuffer;
};

void CALLBACK OnEndAcquisitionPEX(HACQDESC hAcqDesc);
void CALLBACK OnEndFramePEX(HACQDESC hAcqDesc);

class ShadOCam : public Camera
{
public:
	/* Construct an abstraction layer around a Shad-o-Cam and image capture.
	 *
	 * "directory" - where to write images to.
	 * "camFilePath" - the location on disk of the camera configuration file ("xxxx.CAM")
	 */
	ShadOCam(char* directory, char* camFilePath);
	~ShadOCam();

	u_short m_nWidth;			// width of image
	u_short m_nHeight;			// height of image
	
	virtual void CaptureFrames(u_int frames, u_int *frameCount, FrameType frameType, CWnd* window);
	virtual char *GenerateImageFilename(FrameType frameType, u_int frame);
	virtual u_short GetImageHeight();
	virtual u_short GetImageWidth();
	virtual void SetupCamera(float exposureTime);

protected:
	char		*m_camFilePath;

	PXD m_pxd;				// pxd library structure
	FRAMELIB m_framelib;	// frame library structure

	long m_hFrameGrabber;	// adress of frame grabber

	CAMERA_TYPE* m_camType;	// pointer to camera object

	FRAME* m_currentFrame;	// pointer to FRAME object
	short* m_currentFramePtr;	// pointer to start of frame

	BOOL m_bPxdLoaded;
	BOOL m_bFramelibLoaded;
	BOOL m_bCamTypeLoaded;
	BOOL m_bFrameGrabberAllocated;
};