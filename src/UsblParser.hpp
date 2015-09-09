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
    std::vector<std::string> splitValidate(std::string const &buffer, const char* symbol, size_t const parts);
    //            static std::vector<std::string> validateResponse(std::string const s);
    //            static int getInt(std::string s);

public:
    UsblParser();
    ~UsblParser();


    // Given buffer, determine if it's a Notification and returns it's kind.
    // If buffer is not a Notification, returns NO_NOTIFICATION.
    Notification findNotification(std::string const &buffer);

    // Check if buffer is a Notification in DATA mode.
    // In DATA mode: +++AT:<length>:<notification><end-of-line>
    // Throw ValidationError in case of failure.
    void validateNotification(std::string const &buffer);

    // In DATA mode: +++AT:<length>:<notification><end-of-line>
    // IN COMMAND mode: <notification><end-of-line>
    // Validate <notification>, by the number of known fields, independently if in COMMAND or DATA mode.
    // Throw ValidationError in case of failure.
    void splitValidateNotification(std::string const &buffer, Notification const &notification);

    // Given buffer, determine if it's a Response and returns it's kind.
    CommandResponse findResponse(std::string const &buffer);

    // Check if buffer is a valid Response in DATA mode.
    // In DATA mode: +++<AT command>:<length>:<command response><end-of-line>
    // Throw ValidationError in case of failure.
    void validateResponse(std::string const &buffer, std::string const &command);

    // Particular Response for command AT&V (Get Current Set)
    // Check if buffer is a Response in DATA mode.
    // in DATA mode: +++AT&V:<length>:<requested data><end-line>
    // <requested data> = <field1>: <value><end-line><field2>: <value><end-line>...
    // Throw ValidationError in case of failure.
    void validateParticularResponse(std::string const &buffer);

    // Parse a Instant Message into string to be sent to device.
    std::string parseSendIM(SendIM const &im);

    // Parse a received Instant Message from buffer to ReceiveIM.
    // Throw ParseError or ValidationError in case of failure.
    ReceiveIM parseReceivedIM(std::string const &buffer);

    // Parse a received pose from buffer to Position.
    // Throw ValidationError in case of failure.
    Position parsePosition(std::string const &buffer);

    // Return TRUE if Instant Message was received by remote device. Return FALSE if remote device does not confirm receipt.
    bool parseIMReport(std::string const &buffer);

    // TODO to be implemented
    std::string parseRequestedValue(std::string const &buffer, std::string const & command);

    // Return how many fields a notification is splitted by comma (,)
    int getNumberFields(Notification const &notification);

};
}
#endif
