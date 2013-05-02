#ifndef _DUMMYPROJECT_USBLPARSER_HPP_
#define _DUMMYPROJECT_USBLPARSER_HPP_

#include <iodrivers_base/Driver.hpp>
namespace usbl_evologics
{
    class UsblParser
    {
            struct InterfaceStatus* mInterfaceStatus;
        public:
            UsblParser(struct InterfaceStatus* interfaceStatus);
            void parseCommand(uint8_t const* data, size_t size);
            void parsePosition(std::string s);
            void parseDeliveryReport(std::string s);
    };
}
#endif
