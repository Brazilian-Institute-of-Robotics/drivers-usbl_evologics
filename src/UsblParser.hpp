#ifndef _USBLDRIVER_USBLPARSER_HPP_
#define _USBLDRIVER_USBLPARSER_HPP_

#include <iodrivers_base/Driver.hpp>
#include "DriverTypes.hpp"
namespace usbl_evologics
{
    class UsblParser
    {
        private:
            void parseDeliveryReport(std::string s);
            void parseIncommingIm(std::string s);
            std::vector<std::string> splitValidate(std::string s, const char* symbol, int parts);
            std::vector<std::string> validateResponse(std::string s);

            UsblDriverCallbacks *mCallbacks;
            InterfaceStatus* mInterfaceStatus;

        public:
            UsblParser(InterfaceStatus* interfaceStatus);
            int isPacket(std::string s);
            bool parseAsynchronousCommand(std::string s);
            int parseInt(uint8_t const* data, size_t size);
            int parseInt(uint8_t const* data, size_t size, std::string command);
            void parseOk(uint8_t const* data, size_t size);
            ConnectionStatus parseConnectionStatus(std::string s);
            Position parsePosition(std::string s);
            std::string parseString(uint8_t const* data, size_t size, std::string command);
            void setCallbacks(UsblDriverCallbacks *cb);
    };
}
#endif
