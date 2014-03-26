

#include <iostream>
using namespace std;


class bad_camera_type_error : public std::logic_error
{
public:
    explicit bad_camera_type_error(const char *message);
};

class bad_directory_error : public std::logic_error
{
public:
    explicit bad_directory_error(const char *message);
};

class bad_frame_saving_options_error : public std::logic_error
{
public:
    explicit bad_frame_saving_options_error(const char *message);
};

class bad_frame_type_error : public std::logic_error
{
public:
    explicit bad_frame_type_error(const char *message);
};

class bad_serial_port_error : public std::logic_error
{
public:
    explicit bad_serial_port_error(const char *message);
};

class camera_acquisition_error : public std::logic_error
{
public:
    explicit camera_acquisition_error(const char *message);
};

class camera_init_error : public std::logic_error
{
public:
    explicit camera_init_error(const char *message);
};

class camera_setting_error : public std::logic_error
{
public:
    explicit camera_setting_error(const char *message);
};

class file_error : public std::logic_error
{
public:
    explicit file_error(const char *message);
};