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


            bool findNotification(std::string const &buffer, Notification &notification);
            bool validateNotification(std::string const &buffer);
            bool splitValidateNotification(std::string const &buffer, Notification const &notification, std::string &output_msg);

            bool findResponse(std::string const &buffer, CommandResponse &response);
            bool validateResponse(std::string const &buffer, std::string const &command);


            std::string parseSendIM(SendedIM const &im);

            std::string parseReceivedIM(void);
            std::string parseReceivedPose(std::string const &buffer, Position &pose);

            int getNumberFields(Notification const & notification);


//            /*
//             * The function interpretes the string as a connection status and
//             * returns a enum with the status.
//             *
//             * @param[in] command The command to analysing
//             * @return The connection status in a enum
//             */
//            static ConnectionStatus parseConnectionStatus(std::string const command);
//
//            std::vector<int> getFreeBytes(std::string const command);
//
//            /*
//             * The funcion extracts the string in the response and returns the string.
//             * You have to interprete the string anymore.
//             * The function throws an exception, if the response isn't fit to the command.
//             *
//             * @param[in] data The data in a uint8 array to analysing
//             * @param[in] size The length of data
//             * @param[in] command The command to fit the data with this command
//             * @return The extracted string
//             */
//            static std::string parseString(uint8_t const* data, size_t const size, std::string const command);
//
//
//            /*
//             * The functions looks for any asynchronous message in the command.
//             * The function returns a enum with the type of the asynchrnous message.
//             *
//             * @param[in] command The string to analysing
//             * @return A enum with the type of the asynchronous message. If there is no asynchronous message, then NO_ASYNCHRONOUS
//             */
//            static AsynchronousMessages parseAsynchronousCommand(std::string const command);


//            /*
//             * The function parses a asynchronous Position Message USBLLONG
//             *
//             * @param[in] the position to anlysing
//             * @return A complete positon
//             * */
//            static Position parseUsbllong(std::string const positionstring);
//            /*
//             * The function parses a Delivery Report.
//             *
//             * @param[in] report The report to analysing
//             * @return A enum with the extracted delivery status
//             * */
//            static DeliveryStatus parseDeliveryReport(std::string const report);
//            /*
//             * The function interpretes the response as integer.
//             *
//             * @param[in] data The data in a uint8 array to analysing
//             * @param[in] size The length of data
//             * @return The extracted int
//             */
//            static int parseInt(uint8_t const* data, size_t const size);
//            /*
//             * The function interpretes the response as integer.
//             * The function also throws an exception, if this response isn't fit to the command
//             *
//             * @param[in] data The data in a uint8 array to analysing
//             * @param[in] size The length of data
//             * @param[in] command The command to fit the data with this command
//             * @return The extracted int
//             */
//            static int parseInt(uint8_t const* data, size_t const size, std::string const command);
//            /*
//             * The function interpretes the response as OK.
//             * If the respnse isn't an OK the function throws an Exception.
//             * @param[in] data The data in a uint8 array to analysing
//             * @param[in] size The length of data
//             */
//            static void parseOk(uint8_t const* data, size_t const size);
//            /*
//             * The function interpretes the string as a connection status and
//             * returns a enum with the status.
//             *
//             * @param[in] command The command to analysing
//             * @return The connection status in a enum
//             */
//            static ConnectionStatus parseConnectionStatus(std::string const command);
//            /*
//             * The function interpretes the string as a incoming instant message and returns
//             * the extracted instant message
//             *
//             * @param[in] command The command to analysing
//             * @return The instant message in a struct
//             */
//            static ReceiveInstantMessage parseIncomingIm(std::string const s);
//            /*
//             * The functions interpretes the string as a position and
//             * returns a struct with the position.
//             *
//             * @param[in] command The command to analysing
//             * @return The position in a struct
//             */
//            static Position parsePosition(std::string const command);
//            static Position parseRemotePosition(std::string const s);

//
//            static std::string parsePhyNumber(std::string const s);
//            static std::string parseMacNumber(std::string const s);
    };
}
#endif
