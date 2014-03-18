
#pragma once

#include "stdafx.h"
#include "Tomography Control.h"
#include "Exceptions.h"


bad_camera_type_error::bad_camera_type_error(const char *message) : logic_error(message)
{

}

bad_directory_error::bad_directory_error(const char *message) : logic_error(message)
{

}

bad_frame_type_error::bad_frame_type_error(const char *message) : logic_error(message)
{

}

bad_frame_saving_options_error::bad_frame_saving_options_error(const char *message) : logic_error(message)
{

}

bad_serial_port_error::bad_serial_port_error(const char *message) : logic_error(message)
{

}

camera_acquisition_error::camera_acquisition_error(const char *message) : logic_error(message)
{

}

camera_init_error::camera_init_error(const char *message) : logic_error(message)
{

}

file_error::file_error(const char *message) : logic_error(message)
{

}
