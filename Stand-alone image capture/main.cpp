///////////////////////////////////////////////////////////////
// CamExe (Microsoft Visual C++ 6.0 on Windows 98)
//
// devloped for Edinburgh CT Centre for Material Research
// by Claudia Fricke, December 2009
// Update May 2013
// ------------------------------------------------------------
//
// CamExe.exe is written to be called by the Testpoint program
// TCControl.tsp, to control the ShadoCam_4K X-Ray camera.
//
// The program initializes all necessary libraries, variables,
// and objects, some are read from an initialization file.
//
// After the libaries and objects are then configurated,
// several images are grabbed, deinterlaced, and accumulated as
// averaged image into one output file, specified in Testpoint.
// Bad pixels will be corrected by the manufacturers pixel map.
// The output file will be saved in big endian format.
//
// Update: Only Grabbing and saving the images in CamExe as *.crude.
// A new post-process program carries out the image conversions.
// No averaging or summing up images during run possible anymore.
// 
// ------------------------------------------------------------
// The executable CamExe.exe has to be in the Folder
// C:\CamExe\Debug
//
// Make sure that the following files are in the given paths 
// under the following names:
//
// Cam file:
// C:\ShadoCam\SB4KPX.CAM
//
// Initialization file written by Testpoint
// C:\ShadoCam\IniFile.txt
// This file has to content:
// - 1st line:
//   Name and Path of the current Image
// - 2nd line:
//   Exposure Time
// - 3rd line:
//   Number of frames per stop
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>

#include <windows.h>
#include <conio.h>
#include <ctime>

#include <string.h>
#include <stdio.h>

#include "pxd.h"
#include "iframe.h"
#include "Scilib20.h"

using namespace std;

int main()
{
	/////////////////// Variables ////////////////////////

	clock_t start_prog(clock());

	PXD pxd;				// pxd library structure
	FRAMELIB framelib;		// frame library structure
	long hFG;				// adress of frame grabber
	FRAME* currentFrame;	// pointer to FRAME object
	short* currentFramePtr;	// pointer to start of frame
	FRAME* newFrame;
	long qh;				// handle for grab

	int nWidth  = 2048;		// width of image
	int nHeight = 2000;		// height of image

	CAMERA_TYPE* camType;	// pointer to camera object

	const long len = 65536; // length of LUT
	unsigned short nLUT[len];  // LUT

	char camFile[]="C:\\ShadoCam\\SB4KPX.CAM";		// camera setup file
	
	//help variables
	short* currentTempPtr;	// temp pointer
	int i;				// loop counters

	//read from file
	char output_file[200];		// filename for output *.crude-file
	char ext_output_file[200];	// extended filename for output
	float t = 0.5;				// "exposure" time
	short FramesPerStop = 1;	// counter for number of frames per stop

	//open initialization file from Testpoint
	ifstream ini_file("C:\\ShadoCam\\IniFile.txt");
	if (ini_file.is_open())
	{
		ini_file.getline(output_file,199,'\n'); //read name of output file for crude-image
		ini_file >> t;							//read "exposure time"
		ini_file >> FramesPerStop;
		ini_file.close();
	}
	else
	{
		MessageBox(NULL, "Not able to open initialization file.", "CamExe", MB_ICONERROR);
		return 0;
	}
	
	///////////////////// Configuration ///////////////////

	// initialize the Imagenation libraries
	if ( !imagenation_OpenLibrary("pxd_32.dll", &pxd, sizeof(PXD)) )  {
		MessageBox(NULL, "Frame grabber library not loaded.", "CamExe", MB_ICONERROR);
		return 0;  }
	if ( !imagenation_OpenLibrary("frame_32.dll", &framelib, sizeof(FRAMELIB)) )  {
		MessageBox(NULL, "Frame library not loaded.", "CamExe", MB_ICONERROR);
		return 0;  }

	// request access to frame grabber
	if ( !(hFG = pxd.AllocateFG(-1)) )  {
		MessageBox(NULL, "PXD frame grabber not found.", "CamExe", MB_ICONERROR);
		imagenation_CloseLibrary(&framelib);
		imagenation_CloseLibrary(&pxd);
		return 0;  }

	// initialize camera configuration
	if ( !(camType = pxd.LoadConfig(camFile)) )  {
		MessageBox(NULL, "Camera configuration not loaded.", "CamExe", MB_ICONERROR);
		pxd.FreeFG(hFG);
		imagenation_CloseLibrary(&framelib);
		imagenation_CloseLibrary(&pxd);
		return 0;  }

	//set exposure time
	float ft = pxd.GetFramePeriod(camType);
	pxd.SetFramePeriod(camType, t);
	ft = pxd.GetFramePeriod(camType);

	pxd.SetCameraConfig(hFG, camType);
	pxd.ContinuousStrobes(hFG, TRUE);  // turn on camera frame sync

	// initialize input LUT to shift image data down by two bits
	for (i = 0; i < len; i++) nLUT[i] = i>>2;
	pxd.SetInputLUT(hFG, 16, 0, 0, len, nLUT);

	// set up image destination buffers
	if ( !(currentFrame = pxd.AllocateBuffer (pxd.GetWidth(hFG), pxd.GetHeight(hFG), PBITS_Y16)) )  {
		MessageBox(NULL, "Unable to create image buffer 1.", "CamExe", MB_ICONERROR);
		pxd.FreeFG(hFG);
		imagenation_CloseLibrary(&framelib);
		imagenation_CloseLibrary(&pxd);
		return 0;  }
	if ( !(newFrame = pxd.AllocateBuffer (pxd.GetWidth(hFG), pxd.GetHeight(hFG), PBITS_Y16)) )  {
		MessageBox(NULL, "Unable to create image buffer 2.", "CamExe", MB_ICONERROR);
		pxd.FreeFG(hFG);
		imagenation_CloseLibrary(&framelib);
		imagenation_CloseLibrary(&pxd);
		return 0;  }

	// create pointers to image buffer
	// note: the configuration file sets up the image buffer to contain
	//       an extra column to the left and right of the actual image
	// Image Width  = 2050
	// Image Height = 2000
	currentFramePtr = (short *)framelib.FrameBuffer(currentFrame);
	currentFramePtr++; // point to first pixel in buffer1

	currentTempPtr = currentFramePtr;


	////////////////// Image Aquisition //////////////////////////
	clock_t start_aquisition(clock());

	if(FramesPerStop == 1)
	{
		// grab
      	qh = pxd.Grab(hFG, currentFrame, 0);
      	if (!qh)
      	{
      		MessageBox(NULL, "Unable to acquire image.", "CamExe", MB_ICONERROR);
      		pxd.FreeFG(hFG);
      		imagenation_CloseLibrary(&framelib);
		      imagenation_CloseLibrary(&pxd);
      		return 0;  }
		cout << "Grab 1"<<endl;
      
      	//save file
		strcat(output_file, ".crude");
	    framelib.WriteBin(currentFrame, output_file, 1);
   	}//end if(FramesPerStop == 1)
	
   	else //FramesPerStop > 1
   	{
     	for (i=1; i<=FramesPerStop; i++)
     	{
           	// grab
         	qh = pxd.Grab(hFG, currentFrame, 0);
         	if (!qh)
         	{
         		MessageBox(NULL, "Unable to acquire image.", "CamExe", MB_ICONERROR);
         		pxd.FreeFG(hFG);
         		imagenation_CloseLibrary(&framelib);
   		        imagenation_CloseLibrary(&pxd);
         		return 0;  }
			cout << "Grab "<<i<<endl;

         	//save file
			sprintf(ext_output_file, "%s_(%d).crude", output_file, i);
			framelib.WriteBin(currentFrame, ext_output_file, 1);
   	   } //end for
   	}//end else (FramesPerStop > 1)

	//release extra frame
	framelib.FreeFrame(newFrame);

	clock_t end_aquisition(clock());

	// release frame grabber resources
	framelib.FreeFrame(currentFrame);
	pxd.FreeConfig(camType);
	pxd.FreeFG(hFG);
	imagenation_CloseLibrary(&framelib);
	imagenation_CloseLibrary(&pxd);

	clock_t end_prog(clock());

	cout << "Program ran for " << end_prog - start_prog << "ms" <<endl;
	cout << "Image Aquisition took " << end_aquisition - start_aquisition << "ms" <<endl;

	return 1;
//	return 0;
	
}