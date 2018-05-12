#ifndef EXCEPTIONS
#define EXCEPTIONS
#include <stdexcept>

class FileIoError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class BadConfigFile : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

#endif // EXCEPTIONS

