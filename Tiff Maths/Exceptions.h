#include <iostream>

using namespace std;

class image_error : public std::logic_error
{
public:
    explicit image_error(const char *message);
};