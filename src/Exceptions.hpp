#ifndef USBL_EVOLOGICS_EXCEPTIONS_HPP
#define USBL_EVOLOGICS_EXCEPTIONS_HPP
#include <stdexcept>
namespace usbl_evologics{

struct ValidationError : std::runtime_error
{
    explicit ValidationError(std::string const& desc)
    : std::runtime_error(desc) {}
};

struct ParseError : std::runtime_error
{
    explicit ParseError(std::string const& desc)
    : std::runtime_error(desc) {}
};

struct DeviceError : std::runtime_error
{
    explicit DeviceError(std::string const& desc)
    : std::runtime_error(desc) {}
};

struct WrongInputValue : std::runtime_error
{
    explicit WrongInputValue(std::string const& desc)
    : std::runtime_error(desc) {}
};

struct InstantMessagingError : std::runtime_error
{
    explicit InstantMessagingError(std::string const& desc)
    : std::runtime_error(desc) {}
};

struct ModeError : std::runtime_error
{
    explicit ModeError(std::string const& desc)
    : std::runtime_error(desc) {}
};

}

#endif
