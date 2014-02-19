TCControl & CamExe
------------------

Hybrid Testpoint & C++ Program to control the table and the Camera for the X-Ray CT in the Edinburgh CT Centre for Material Research.

Testpoint controls table directly, camera is controlled by calling the executable CamExe.exe in the Folder
C:\CamExe\Debug\CamExe.exe

Make sure that the following files are in the given paths under the following names:

Cam file:
C:\ShadoCam\SB4KPX.CAM

Pixel map
C:\ShadoCam\SensorGapPM

Initialization file written by Testpoint
C:\ShadoCam\IniFile.txt
This file has to content:
- 1st line:
  Name and Path of the current Image
- 2nd line:
  Exposure Time
- 3rd line:
  Number of frames per stop
- 4th line:
  Method for image cummulation
    1: save each image separately
    2: average all images and save just final image
    3: sum all images and save just final image
