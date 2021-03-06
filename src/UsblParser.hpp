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
    /** Check if buffer can be splitted in an establish amount.
     *
     * @param buffer to be analyzezd.
     * @param symbol of split.
     * @param parts of splitted buffer.
     * @return vector of string with parts size.
     */
    std::vector<std::string> splitValidate(std::string const &buffer,  const char* symbol, size_t const parts);


public:
    UsblParser();
    ~UsblParser();

    /** Print a buffer string that may contain hex that is not a character.
     *
     * Convert data that is not a character to its hex number. Used for debug.
     * @param buffer to be printed
     * @return string without command escape sequence.
     */
    static std::string printBuffer(const std::string& buffer);

    /** Print a buffer vector<uint8_t> that may contain hex that is not a character.
     *
     * Convert data that is not a character to its hex number. Used for debug.
     * @param buffer to be printed
     * @return string without command escape sequence.
     */
    static std::string printBuffer(const std::vector<uint8_t>& buffer);


    /** Check if buffer can be splitted at least in a establish amount.
     *
     * Ignore if buffer has more than 'parts' 'symbol'. They'd be present in the last element of vector.
     * To be used with Instant Messages.
     * @param buffer to be analyzezd.
     * @param symbol of split.
     * @param parts. minimal amount buffer can be spllited.
     * @return vector of string with parts size.
     */
    std::vector<std::string> splitMinimalValidate(std::string const &buffer,  const char* symbol, size_t const parts);

    /** Find a Notification in a buffer.
     *
     *  @param buffer to be analyzed.
     *  @return Kind of notification. If buffer is not a Notification, returns NO_NOTIFICATION.
     */
    Notification findNotification(std::string const &buffer) const;

    /** Validate the number of field of a Notification.
     *
     * In DATA mode: +++AT:<length>:<notification><end-of-line>
     * In COMMAND mode: <notification><end-of-line>
     * Check the number if fields in <notification>. Can be used in DATA or COMMAND mode.
     * Throw ValidationError in case of failure.
     * @param buffer Notification in DATA or COMMAND mode.
     * @param notification Kind of Notification in buffer.
     */
    void splitValidateNotification(std::string const &buffer, Notification const &notification);

    /** Check for a Response in buffer.
     *
     * Look for particular response: "OK", "ERROR" or "BUSY". If could not find these, return REQUEST_VALUE.
     * @param buffer to be analyzed.
     * @return Kind of response.
     */
    CommandResponse findResponse(std::string const &buffer);

    /** Get response or notification content in DATA mode.
     *
     *  In DATA mode:
     *  +++<AT command>:<length>:<command response><end-of-line>
     *  +++<AT>:<length>:<notification><end-line>
     *  Return data like in COMMAND mode.
     *  Throy ValidationError or ModeError in case of failure.
     *  @param buffer Notification or Response in DATA mode.
     *  @return <content><end-line> like in COMMAND mode.
     */
    std::string getAnswerContent(std::string const &buffer);

    /** Get notification content in DATA mode and validate with command.
     *
     *  In DATA mode:
     *  +++<AT command>:<length>:<command response><end-of-line>
     *  Return data like in COMMAND mode.
     *  Throy ValidationError or ModeError in case of failure.
     *  @param buffer Response in DATA mode.
     *  @param command to be validate.
     *  @return <content><end-line> like in COMMAND mode.
     */
    std::string getAnswerContent(std::string const &buffer, std::string const &command);

    /** Remove <end-of-line> "\r\n" from buffer
     *
     * @param buffer to be analyzezd.
     * @return string without <end-of-line> if it's present in buffer.
     */
    std::string removeEndLine(std::string const &buffer);

    /** Parse a Instant Message into string to be sent to device.
     *
     * @param im Instant Message.
     * @return string to be sent to device.
     */
    std::string parseSendIM(SendIM const &im);

    /** Parse a received Instant Message from buffer to ReceiveIM.
     *
     * Throw ParseError or ValidationError in case of failure.
     * @param buffer with Instant Message.
     * @return Received Instant Message.
     */
    ReceiveIM parseReceivedIM(std::string const &buffer);

    /** Parse a received pose from buffer to Position.
     *
     * Throw ValidationError in case of failure.
     * @param buffer with Pose.
     * @return Position.
     */
    Position parsePosition(std::string const &buffer);

    /** Parse a received direction from buffer to Direction.
     *
     * Throw ValidationError in case of failure.
     * @param buffer with direction.
     * @return Direction.
     */
    Direction parseDirection(std::string const &buffer);

    /** Check if Instant Message was delivered.
     *
     * Throw ParseError in case of failure.
     * @param buffer from device.
     * @return DELIVERED if delivery was successful,
     *  FAILED if remote device doesn't confirm receipt.
     *  CANCELED if ack is no longer waited.
     */
    DeliveryStatus parseIMReport(std::string const &buffer);

    /** Get the number of fields in a Notification.
     *
     * Notification are splitted by comma ",".
     * Each Notification has a determined amount of fields.
     * Throw ValidationError in case of failure.
     * @param notification.
     * @return number of fields.
     */
    int getNumberFields(Notification const &notification) const;

    /** Get the integer from a response buffer in COMMAND mode.
     *
     * Throw ParseError in case of failure.
     * @param buffer with integer as response.
     * @return integer number.
     */
    int getNumber(std::string const &buffer);

    /** Get the double from a response buffer in COMMAND mode.
     *
     * Throw ou_of_range in case of failure.
     * @param buffer with floating point number as response.
     * @return double number.
     */
    double getDouble(std::string const &buffer);

    /** Get a long long unsigned int from a response buffer in COMMAND mode.
     *
     * Throw ou_of_range in case of failure.
     * @param buffer with a counter number as response.
     * @return long long unsigned int number.
     */
    long long unsigned int getULLongInt(std::string const &buffer);

    /** Parse AcousticConnection Status of underwater link.
     *
     * Throw ParseError in case of failure.
     * @param buffer with Connection Status
     * @return AcousticConnection of underwater link
     */
    AcousticConnection parseConnectionStatus (std::string const &buffer);

    /** Parse Delivery Status of a Message.
     *
     * Throw ParseError in case of failure.
     * @param buffer with Delivery Status.
     * @return DeleviryStatus.
     */
    DeliveryStatus parseDeliveryStatus (std::string const &buffer);

    /** Parse current settings.
     *
     * Throw ParseError in case of failure.
     * @param buffer with list of current device settings.
     * @return
     */
    DeviceSettings parseCurrentSettings (std::string const &buffer);

    /** Parse Multipath structure
     *
     * @param buffer with list of last received acoustic signal's propagation.
     * @return vector of Multipath.
     */
    std::vector<MultiPath> parseMultipath (std::string const &buffer);


};
}
#endif
