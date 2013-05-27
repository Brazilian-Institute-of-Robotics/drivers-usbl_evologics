#ifndef _DUMMYPROJECT_USBLPARSER_HPP_
#define _DUMMYPROJECT_USBLPARSER_HPP_

#include <iodrivers_base/Driver.hpp>
#include "Driver.hpp"
namespace usbl_evologics
{
    class UsblDriverCallbacks;
    class UsblParser
    {
            struct InterfaceStatus* mInterfaceStatus;
            void parseConfigCommand(std::string s);
            UsblDriverCallbacks *mCallbacks;
        public:
            UsblParser(struct InterfaceStatus* interfaceStatus);
            void parseCommand(uint8_t const* data, size_t size);
            void parsePosition(std::string s);
            void parseDeliveryReport(std::string s);
            void parseIncommingIm(std::string s);
            void setCallbacks(UsblDriverCallbacks *cb);
    };
}
#endif
