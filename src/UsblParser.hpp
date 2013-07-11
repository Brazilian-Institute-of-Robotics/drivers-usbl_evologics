#ifndef _USBLDRIVER_USBLPARSER_HPP_
#define _USBLDRIVER_USBLPARSER_HPP_

#include <iodrivers_base/Driver.hpp>
#include "DriverTypes.hpp"
namespace usbl_evologics
{
    class UsblParser
    {
        private:
            static std::vector<std::string> splitValidate(std::string s, const char* symbol, size_t const parts);
            static std::vector<std::string> validateResponse(std::string const s);
            static int getInt(std::string s);


        public:
            UsblParser();
            /*
             * The function looks for a command in the string s.
             * The function return different int values:
             * Negative Value: There is no-commmand data in the front of the string with len value. The data can be skipped without analysing.
             * 0: There is no command, but it should be a command. (incomplete command)
             * positive value: There is a command in front of the string until value.
             */
            static int isPacket(std::string const s);
            /*
             * The functions looks for any asynchronous message in the command.
             * Returns True if the functions handled the command as asynchronous command.
             * Returns False if there is no asynchronous Message and you have to handle the command
             * anymore.
             */
            static AsynchronousMessages parseAsynchronousCommand(std::string const s);
            /*TODO comment*/
            static DeliveryStatus parseDeliveryReport(std::string const s);
            /*
             * The function interpretes the response as integer.
             */
            static int parseInt(uint8_t const* data, size_t const size);
            /*
             * The function interpretes the response as integer.
             * The function also throws an exception, if this response isn't fit to the command
             */
            static int parseInt(uint8_t const* data, size_t const size, std::string const command);
            /*
             * The function interpretes the response as OK.
             * If the respnse isn't an OK the function throws an Exception.
             */
            static void parseOk(uint8_t const* data, size_t const size);
            /*
             * The function interpretes the string as a connection status and
             * returns a enum with the status.
             */
            static ConnectionStatus parseConnectionStatus(std::string const s);
            static ReceiveInstantMessage parseIncommingIm(std::string const s);
            /*
             * The functions interpretes the string as a position and
             * returns a struct with the position.
             */
            static Position parsePosition(std::string const s);
            /*
             * The funcion extracts the string in the response and returns the string.
             * You have to interprete the string anymore.
             * The function throws an exception, if the response isn't fit to the command.
             */
            static std::string parseString(uint8_t const* data, size_t const size, std::string const command);
    };
}
#endif
