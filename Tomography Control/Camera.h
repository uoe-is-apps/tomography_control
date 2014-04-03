#pragma once

#include "Tomography Control.h"
#include "xis/Acq.h"
#include "iframe.h"
#include "pxd.h"
#include "Scilib20.h"

#include "tiffio.h"
#include "tiff.h"

// 2 minute timeout on image capture
#define DEFAULT_CAPTURE_TIMEOUT (2 * 60 * 1000)

// 10% margin for differences between captured images
#define PIXEL_AVERAGE_TOLERANCE 0.1

// Camera configuration file for the PXD1000 frame grabber
#define SHAD_O_CAM_CONFIG_FILE "C:\\Program Files\\Imagenation PXD1000\\Bin\\DEFAULT.CAM"

enum FrameType { SINGLE, DARK, FLAT_FIELD };
enum FrameSavingOptions { INDIVIDUAL, AVERAGE, SUM };

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
	Camera(CString directory);
	virtual ~Camera();
	
	void AddFrameToBuffer(unsigned int *dest, unsigned short *src);
	double CalculatePixelAverage(unsigned short *frameBuffer);
	
	void CalculatePixelAverages(unsigned short *dest, unsigned int *src, unsigned short frameCount);
	void CalculatePixelSums(unsigned short *dest, unsigned int *src);

	/* Captures a series of frames and write them to disk.
	 *
	 * frames: number of frames to capture
	 * imageCount: counter for images captured in a single run, used to derive filename. Will be updated
	 * as images are written to disk.
	 * frameType: the type of frame, used to determine filename.
	 * window: the dialog window to notify of progress.
	 * timeoutAt: when to timeout (indicates too many frame re-captures due to beam failure).
	 */
	virtual void CaptureFrames(u_int frames, u_int *imageCount, FrameSavingOptions captureType,
		FrameType frameType, CWnd* window, CTime timeoutAt) = 0;

	/* Construct a filename for an image to be written to disk. Returns an pointer
	 * to a buffer containing the filename. This buffer is overwritten when this method
	 * is called next.
	 *
	 * frameType: the type of frame to capture (single, dark, flat_field)
	 * frame: the number of the frame, within a captured set
	 * fileEnding: the ending of the filename, to indicate file type (i.e. "tiff", "raw")
	 */
	char *GenerateImageFilename(FrameType frameType, u_int frame, char* fileEnding);

	/* Construct a filename for an image to be written to disk. Returns an pointer
	 * to a buffer containing the filename. This buffer is overwritten when this method
	 * is called next.
	 *
	 * frameType: the type of frame to capture (single, dark, flat_field)
	 * frame: the number of the frame, within a captured set
	 */
	virtual char *GenerateImageFilename(FrameType frameType, u_int frame) = 0;
	
	/* Construct the full path for an image file, based on the filename
	 * of the image. Returns an pointer to a buffer containing the file path.
	 * This buffer is overwritten when this method is called next.
	 *
	 * framename: the name of the file within the directory
	 */
	char *GenerateImagePath(char *filename);

	CString GetDirectory();
	virtual u_short GetImageHeight() = 0;
	virtual u_short GetImageWidth() = 0;
	virtual void SetupCamera() = 0;

	void WriteTiff(char* filename, unsigned short *buffer);

protected:
	CString m_directory; // Directory to write images to
	char m_filenameBuffer[MAX_PATH];
	char m_filepathBuffer[MAX_PATH];
};

class DummyCamera : public Camera
{
public:
	DummyCamera(CString directory, float exposureTime);
	~DummyCamera();
	
	virtual void CaptureFrames(u_int frames, u_int *imageCount, FrameSavingOptions captureType, FrameType frameType, CWnd* window, CTime timeoutAt);
	virtual char *GenerateImageFilename(FrameType frameType, u_int frame);
	virtual u_short GetImageHeight();
	virtual u_short GetImageWidth();
	virtual void SetupCamera();
	
	u_short m_nWidth;
	u_short m_nHeight;

protected:
	float m_exposureTimeSeconds;

	/* Buffer used when doing an acquisition for summed frames. */
	unsigned int *m_sumFrame;

	/* Buffer used when holding sum/average data for writing out. */
	unsigned short *m_sumAvgFrame;
};

class PerkinElmerXrd : public Camera
{
public:
	PerkinElmerXrd(CString directory, u_int mode);
	~PerkinElmerXrd();
	
	virtual void CaptureFrames(u_int frames, u_int *imageCount, FrameSavingOptions captureType, FrameType frameType, CWnd* window, CTime timeoutAt);
	virtual char *GenerateImageFilename(FrameType frameType, u_int frame);
	virtual u_short GetImageHeight();
	virtual u_short GetImageWidth();
	virtual void SetupCamera();

	/* Buffer used when doing an acquisition for summed frames. */
	unsigned int *m_sumFrame;

	/* Buffer used when holding sum/average data for writing out. */
	unsigned short *m_sumAvgFrame;

protected:
	u_int m_nWidth;			// width of image
	u_int m_nHeight;		// height of image

	u_int m_cameraMode;
	
	char		m_errorBuffer[ERROR_BUFFER_SIZE];
	float		m_exposureTimeSeconds;
	HACQDESC	m_hAcqDesc;
	int			m_nChannelNr;
	BOOL		m_detectorInitialised;
};

void CALLBACK OnEndAcquisitionPEX(HACQDESC hAcqDesc);
void CALLBACK OnEndFramePEX(HACQDESC hAcqDesc);

class ShadOCam : public Camera
{
public:
	/* Construct an abstraction layer around a Shad-o-Cam and image capture.
	 *
	 * directory: where to write images to.
	 * camFilePath: the location on disk of the camera configuration file ("xxxx.CAM")
	 */
	ShadOCam(CString directory, float exposureTimeSeconds, char* camFilePath, char* pixMapFilePath);
	~ShadOCam();
	
	void AddFrameToBuffer(FRAME *dest, FRAME *currentFrame);
	void CalculatePixelAverages(FRAME *dest, FRAME *src, unsigned short frameCount);
	void CalculatePixelSums(FRAME *dest, FRAME *src);
	void ClearFrame(FRAME *dest);
	
	double CalculatePixelAverage(FRAME *frameBuffer);
	virtual void CaptureFrames(u_int frames, u_int *imageCount, FrameSavingOptions captureType, FrameType frameType, CWnd* window, CTime timeoutAt);
	virtual char *GenerateImageFilename(FrameType frameType, u_int frame);
	virtual u_short GetImageHeight();
	virtual u_short GetImageWidth();
	virtual void SetupCamera();

protected:
	char		m_errorBuffer[ERROR_BUFFER_SIZE];

	float		m_exposureTimeSeconds;

	u_short m_nWidth;			// width of image
	u_short m_nHeight;			// height of image

	char	m_camFilePath[MAX_PATH];	// Path of the camera configuration file (used by PXD)
	char	m_pixMapFilePath[MAX_PATH];

	PXD m_pxd;				// pxd library structure
	FRAMELIB m_framelib;	// frame library structure
	
	PIXMAPENTRY* m_pixMap;	// pointer to pixel map object
	int m_pixMapEntries;	// number of pixel corrections in pixel map

	long m_hFG;	// address of frame grabber

	CAMERA_TYPE* m_camType;	// pointer to camera object
	
	FRAME* m_currentFrame;	// pointer to FRAME object
	FRAME* m_sumFrame;		// pointer to sum frame for holding totals while acquiring data
	FRAME* m_avgSumFrame;	// pointer to average/sum frame for writing to disk

	BOOL m_bPxdLoaded;
	BOOL m_bFramelibLoaded;
	BOOL m_bCamTypeLoaded;
	BOOL m_bFrameGrabberAllocated;
};

/* Structure for tracking a specific acquisition from the Perkin-Elmer
 * camera. This is needed because the process is asynchronous, so this
 * data needs to be passed into the frame event handler.
 */
struct PerkinElmerAcquisition {
	PerkinElmerXrd *camera;
	CWnd *window;

	FrameSavingOptions captureType;
	FrameType frameType;

	CEvent endAcquisitionEvent;
	
	BOOL lastPixelAverageValid;
	double lastPixelAverage;
	u_int *imageCount;
	u_int frames;
	u_int capturedFrames;
	unsigned short *acquisitionBuffer;
};