#ifndef _USBLDRIVER_USBLPARSER_HPP_
#define _USBLDRIVER_USBLPARSER_HPP_

#include <iodrivers_base/Driver.hpp>
#include "DriverTypes.hpp"
#include <string.h>
#include <iostream>
namespace usbl_evologics
{
    class UsblParser
    {
        private:
            bool splitValidate(std::string s, const char* symbol, size_t const parts, std::vector<std::string> &splitted);
//            static std::vector<std::string> validateResponse(std::string const s);
//            static int getInt(std::string s);


        public:
            UsblParser();
            ~UsblParser();

//            int isPacket(std::string const &buffer, OperationMode const &mode);

            bool findNotification(std::string const &buffer, Notification &notification);
            bool validateNotification(std::string const &buffer);
            bool splitValidateNotification(std::string const &buffer, Notification const &notification, std::string &output_msg);


            bool findResponse(std::string const &buffer, CommandResponse &response);
            bool validateResponse(std::string const &buffer, std::string const &command);
            bool validateParticularResponse(std::string const &buffer);


            std::string parseSendIM(SendIM const &im);

            std::string parseReceivedIM(std::string const &buffer, ReceiveIM &im);
            std::string parsePosition(std::string const &buffer, Position &pose);
            std::string parseIMReport(std::string const &buffer);
            std::string parseRequestedValue(std::string const &buffer, std::string const & command);

            int getNumberFields(Notification const & notification);

    };
}
#endif
