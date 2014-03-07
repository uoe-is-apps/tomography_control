

#include <iostream>
using namespace std;


class bad_frame_type_error : public std::logic_error
{
public:
    explicit bad_frame_type_error(const char *message);
};

class camera_init_error : public std::logic_error
{
public:
    explicit camera_init_error(const char *message);
};