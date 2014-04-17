Tomography Control
==================

C++ application to control the table and the cameras for the X-Ray CT in the
Edinburgh CT Centre for Material Research.

To compile, this will require the SDKs for the Rad-icon Shad-o-cam, Perkin-Elmer XIS camera,
and PXD1000 frame grabber. These are not included for licensing reasons. It is generally
anticipated that this is used as a reference for how such an application can be designed,
rather than a complete ready to use example.

Make sure that the following files are in the given paths under the following names:

Camera configuration file for the PXD1000 frame grabber:
C:\Program Files\Imagenation PXD1000\Bin\DEFAULT.CAM

Pixel map
<user home directory>\My Documents\Pixel Map
