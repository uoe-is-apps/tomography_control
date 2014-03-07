

#include <iostream>
using namespace std;


class bad_frame_type_error : public std::logic_error
{
public:
    explicit bad_frame_type_error(const char *message);
};