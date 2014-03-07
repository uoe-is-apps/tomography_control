
#pragma once

#include "stdafx.h"
#include "Tomography Control.h"
#include "Exceptions.h"


bad_frame_type_error::bad_frame_type_error(const char *message) : logic_error(message)
{

}

camera_init_error::camera_init_error(const char *message) : logic_error(message)
{

}
