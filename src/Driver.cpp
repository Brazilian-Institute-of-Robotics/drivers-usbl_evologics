#include "Driver.hpp"
#include <iostream>
#include <sstream>
#include "base/Logging.hpp"
//#include "Exceptions.hpp"

using namespace std;
using namespace usbl_evologics;

Driver::Driver()
: iodrivers_base::Driver(max_packet_size)
{
    mode = DATA;
}

Driver::~Driver()
{
}

// Send a command to device.
void Driver::sendCommand(string const &command)
{
    // Fill buffer (header and end-line) according operational mode and interface type.
    string buffer = fillCommand(command);
    iodrivers_base::Driver::writePacket(reinterpret_cast<const uint8_t*>(buffer.c_str()), buffer.length());

    // Manage the operational mode, DATA or COMMAND, according to command.
    modeManager(command);
}

// Send raw data to remote device.
void Driver::sendRawData(string const &raw_data)
{
    iodrivers_base::Driver::writePacket(reinterpret_cast<const uint8_t*>(raw_data.c_str()), raw_data.length());
}

// Filled command string to be sent to device.
string Driver::fillCommand(string const &command)
{
    stringstream ss;
    // Guard Time Escape Sequence (GTES) doesn't require <end-line>
    // GTES: (1s)<+++>(1s)
    if(command == "+++")
    {
        ss << command << flush;
        if(mode == DATA)
            sleep(1);
    }
    else
    {
        if (mode == DATA)
            // If in DATA, buffer = +++<AT command>
            ss << "+++" << flush;
        ss << addEndLine(command) << flush;
    }
    return ss.str();
}

// Add a end line, according interface type.
string Driver::addEndLine(string const &command)
{
    stringstream ss;
    if (interface == SERIAL)
        ss << command << "\r" << flush;
    else // (interface == ETHERNET)
        ss << command << "\n" << flush;
    return ss.str();
}

int Driver::extractRawDataPacket(string const& buffer) const
{
    return buffer.size();
}


// Check the size of a particular response.
int Driver::checkParticularResponse(string const& buffer) const
{
    string::size_type eol = buffer.find("\r\n\r\n");
    if(eol != string::npos)
        // Add \r\n\r\n to buffer.
        return eol+4;
    else
        return 0;
}

// Check the size of regular response.
int Driver::checkRegularResponse(string const& buffer) const
{

    string::size_type eol = buffer.find("\r\n");
    if(eol != string::npos)
        // Add \r\n to buffer.
        return eol+2;
    else
        return 0;
}

int Driver::extractATPacket(string const& buffer) const
{
    // Smallest packet possible is +++AT:0:\r\n
    if (buffer.size() < 10)
        return 0;
    if (buffer.substr(0, 5) != "+++AT")
        return -3;
    // Get length.
    // +++<AT command>:<length>:<command response><end-of-line>
    // +++AT:<length>:<notification><end-of-line>
    string::size_type npos = string::npos;
    string::size_type length = 0;
    if ((npos = buffer.find(":")) != string::npos)
    {
        length += npos;
        string msg = buffer;
        msg = msg.substr(npos+1, msg.size()-npos);
        if ((npos = msg.find(":")) != string::npos)
        {
            length += npos;
            // add 2 colon ":".
            length += 2;
            // Convert <length> to integer
            length += stoi(msg.substr(0, npos),&npos);
            // add <end-of-line>
            length += 2;
            if(length > buffer.size())
            {
                LOG_WARN("Size Error. Found length %u doesn't match with buffer size of %s. Waiting more data in buffer ", length, buffer.c_str());
                return 0;
            }
            // Check the presence of end-of-line (\r\n).
            if (buffer.substr(length-2, length) != "\r\n")
                throw runtime_error("Could not find <end-of-line> at position \""+ to_string(length-2) + "\"of the end of buffer  \"" + buffer + "\"");
            return length;
        }
        else if (buffer.size() > 16)
            throw runtime_error("Assuming max lenght of 999, could not find second \":\" before 16 bytes in buffer \"" + buffer + "\"");
        return 0;
    }
    // Check max command size. +++AT?CLOCK:
    else if (buffer.size() > 12)
        throw runtime_error("Could not find any \":\" before 12 bytes in buffer " + buffer);
    return 0;
}

int Driver::extractRawFromATPackets(string const& buffer) const
{
    // TIES: Time Independent Escape Sequence
    const char* TIES_HEADER = "+++";
    const int   TIES_HEADER_SIZE = 3;

    string::size_type buffer_size = buffer.size();
    string::size_type ties_start = 0;
    while (ties_start < buffer_size)
    {
        // Look for the start of the first character of a TIES header
        while (ties_start < buffer_size && buffer[ties_start] != TIES_HEADER[0])
            ties_start++;

        // Check whether we actually have a header. If there's enough space left in the
        // buffer, it has to be exactly like a full header
        if (ties_start + TIES_HEADER_SIZE < buffer_size)
        {
            if (buffer.substr(ties_start, TIES_HEADER_SIZE) == TIES_HEADER)
            {
                if (ties_start == 0)
                    return extractATPacket(buffer);

                int raw_data_packet = extractRawDataPacket(buffer.substr(0, ties_start));
                if (raw_data_packet == 0)
                    throw UnexpectedRawPacket("found beginning of raw packet without an end");
                else return raw_data_packet;
            }
            ++ties_start;
        }
        else if (string(TIES_HEADER, buffer_size - ties_start) == buffer.substr(ties_start, buffer_size - ties_start))
        {
            // Here, we have something that might be the start of a TIES header at the end of the buffer
            return extractRawDataPacket(buffer.substr(0, ties_start));
        }
    }
    return extractRawDataPacket(buffer.substr(0, ties_start));
}

int Driver::extractPacket(uint8_t const* buffer, size_t buffer_size) const
{
    string buffer_as_string = string(reinterpret_cast<char const*>(buffer), buffer_size);

    // Both COMMAND and DATA mode, answer finish by \r\n.
    if(mode == DATA)
        return extractRawFromATPackets(buffer_as_string);
    // Check in COMMAND mode
    else
    {   // Check for a single number as response.
        if(buffer_as_string.size() < 4)
            return checkRegularResponse(buffer_as_string);
        // Check for Notification
        int ret = checkNotificationCommandMode(buffer_as_string);
        if(ret >= 0)
            return ret;
        // Second, check for particular Response.
        if(buffer_as_string.find("Sour") != string::npos)
            return checkParticularResponse(buffer_as_string);
        // Last, check for regular Response.
        return checkRegularResponse(buffer_as_string);
    }
}

// Read response from device.
ResponseInfo Driver::readResponse(void)
{
    Notification notification;
    CommandResponse response;
    ResponseInfo response_info;
    response_info.response = NO_RESPONSE;

    // Get string from device.
    string buffer_as_string = readInternal();
    // Check for Notification and enqueue data.
    if((notification = isNotification(buffer_as_string)) != NO_NOTIFICATION)
    {
        // interpretNotification(buffer_as_string, notification);
        NotificationInfo notification_info;
        notification_info.notification = notification;
        notification_info.buffer = buffer_as_string;
        queueNotification.push(notification_info);
        return response_info;
    }
    // Check for response and output it.
    else if((response = isResponse(buffer_as_string)) != NO_RESPONSE)
    {
        ResponseInfo response_info;
        response_info.response = response;
        response_info.buffer = buffer_as_string;
        return response_info;
    }
    // If data is neither notification or response, it's raw data. Enqueue it.
    queueRawData.push(buffer_as_string);
    return response_info;
}



// Read packets.
string Driver::readInternal(void)
{
    vector<uint8_t> read_buffer;
    read_buffer.resize(max_packet_size);
    int readpacket = iodrivers_base::Driver::readPacket(&read_buffer[0], max_packet_size);

    return string(reinterpret_cast<char const*>(&read_buffer[0]), readpacket);
}

// Read input data till get a response.
string Driver::waitResponse(string const &command, CommandResponse expected)
{
    ResponseInfo response_info;
    response_info.response = NO_RESPONSE;
    base::Time init_time = base::Time::now();
    // 1 second time-out. Arbitrary value.
    base::Time time_out = base::Time::fromSeconds(1);
    base::Time time_now = base::Time::now();

    while(response_info.response != expected && (time_now - init_time) <= time_out)
    {
        // Read till get expected response.
        response_info = readResponse();
        time_now = base::Time::now();

        if(response_info.response == ERROR)
            throw DeviceError("USBL Driver.cpp waitResponse: For the command: \""+ command +"\", device return the follow ERROR msg: \"" + response_info.buffer + "\"");
        if(response_info.response == BUSY)
             throw BusyError("USBL Driver.cpp waitResponse: For the command: \""+ command +"\", device return the follow BUSY msg: \"" + response_info.buffer + "\". Try it latter.");
    }
    if((time_now - init_time) > time_out)
        throw runtime_error("USBL Driver.cpp waitResponse: For the command: \""+ command +"\", device didn't send a response in " + to_string(time_out.toSeconds()) + " seconds time-out");
    // In DATA mode, validate response and return content without header.
    if(mode == DATA)
    {
        validResponse(response_info.buffer, command);
        return usblParser.getAnswerContent(response_info.buffer);
    }
    return response_info.buffer;
}

// Wait for a OK response.
void Driver::waitResponseOK(string const &command)
{
    waitResponse(command, COMMAND_RECEIVED);
}

// Wait for a integer response.
int Driver::waitResponseInt(string const &command)
{
    return usblParser.getNumber(waitResponseString(command));
}

// Wait for string response.
string Driver::waitResponseString(string const &command)
{
    return waitResponse(command, VALUE_REQUESTED);
}

// Check if a Notification string is present in buffer.
int Driver::checkNotificationCommandMode(string const& buffer) const
{
    if (buffer.size() < 4)
        return 0;
    else
    {   // All 4 letters notification
        if (buffer.find("RECV") !=string::npos ||
                buffer.find("DELI") !=string::npos ||
                buffer.find("FAIL") !=string::npos ||
                buffer.find("CANC") !=string::npos ||
                buffer.find("EXPI") !=string::npos ||
                buffer.find("SEND") !=string::npos ||
                buffer.find("USBL") !=string::npos ||
                buffer.find("BITR") !=string::npos ||
                buffer.find("SRCL") !=string::npos ||
                buffer.find("PHYO") !=string::npos ||
                buffer.find("RADD") !=string::npos )
            return checkRegularResponse(buffer);
        else
            return -1;
    }
}

// Check kind of notification.
Notification Driver::isNotification(string const &buffer)
{
    Notification notification = usblParser.findNotification(buffer);
    if(notification != NO_NOTIFICATION)
    {
        if(mode == DATA)
            validNotification(buffer);
        fullValidation(buffer, notification);
    }
    return notification;
}

// Check kind of response.
CommandResponse Driver::isResponse(string const &buffer)
{
    // In DATA mode, all response starts by "+++AT". If there is no initial string, it isn't a response.
    if(mode == DATA && buffer.find("+++AT")==string::npos)
        return NO_RESPONSE;
    return usblParser.findResponse(buffer);
}

// Check a valid notification in DATA mode.
void Driver::validNotification(string const &buffer)
{
    if(mode == DATA)
    {
        usblParser.validateNotification(buffer);
    }
    // Not possible to use UsblParser::validateNotification() in COMMAND mode.
    else
    {
        stringstream error_string;
        error_string << "USBL Driver.cpp validNotification: Function only can be called in DATA mode, not in COMMAND mode. Actual mode: \"" << mode << "\"" <<flush;
        throw ModeError(error_string.str());
    }
}

// Check a valid notification.
void Driver::fullValidation(string const &buffer, Notification const &notification)
{
    usblParser.splitValidateNotification(buffer, notification);
}

// Check for a valid response in DATA mode.
void Driver::validResponse(string const &buffer, string const &command)
{
    if(mode == DATA)
        usblParser.validateResponse(buffer, command);
    // Not possible to use UsblParser::validateResponse() in COMMAND mode.
    else
    {
        stringstream error_string;
        error_string << "USBL Driver.cpp validResponse: Function only can be called in DATA mode, not in COMMAND mode. Actual mode: \"" << mode << "\"" <<flush;
        throw ModeError(error_string.str());
    }
}

// // TODO. Check the best way to interpreted every kind of notification and what it should return.
// Interpret notification.
void Driver::interpretNotification(string const& buffer, Notification const &notification)
{
    switch (notification) {
    case USBLLONG:
        usbl_pose = usblParser.parsePosition(buffer);
        break;
    case USBLANGLE:
        throw DeviceError("NOT POSSIBLE TO GET POSE. USBANGLE NOT IMPLEMENTED");
        break;
    case RECVIM:
        receiveIM = usblParser.parseReceivedIM(buffer);
        break;
    case RECVIMS:
        //usblParser.parseReceivedIMS();
        throw DeviceError("NEW Synchronous IM. Not implemented");
        break;
    case RECVPBM:
        //usblParser.parseReceivedPBM();
        throw DeviceError("NEW PiggyBack M. Not implemented");
        break;
    case DELIVERY_REPORT:
        usblParser.parseIMReport(buffer);
        break;
    case CANCELED_IM:
        usblParser.parseIMReport(buffer);
        break;
    case EXTRA_NOTIFICATION:
        throw DeviceError("Extra Notification. Not implemented");
        break;
    case NO_NOTIFICATION:
        throw DeviceError("NO Notification. Not implemented");
        break;
    }
}

// Manage mode operation according command sent.
void Driver::modeManager(string const &command)
{
    // Command ATO. Switch to DATA mode doesn't require answer
    if(command.find("ATO")!=string::npos && mode == COMMAND)
    {
        // Proceed just after send the command
        mode = DATA;
    }
    // Switch to COMMAND mode. (1s)<+++>(1s). Require OK response.
    else if(command.find("+++")!=string::npos && mode == DATA )
    {
        sleep(1);
        // Receive answer in COMMAND mode.
        mode = COMMAND;
        // switch to COMMAND mode after receive a OK answer
    }
    // Switch to COMMAND mode. Require OK response.
//    else if(command.find("ATC")!=string::npos && mode == DATA)
//    {    }
}

// Manage mode operation according command sent and response obtained.
void Driver::modeMsgManager(string const &command)
{
    // Command ATO. Switch from DATA to COMMAND mode doesn't require answer
    if(command.find("ATO")!=string::npos && mode == COMMAND)
    {
        // Proceed just after send the command
        //queueCommand.pop();
        //mode = DATA;
    }
    // Switch to COMMAND mode. (1s)<+++>(1s). Require OK response.
    else if(command.find("+++")!=string::npos && mode != COMMAND)// && response == COMMAND_RECEIVED)
    {
        // If there was a error, keep in DATA mode.
        mode = DATA;
    }
    // Switch to COMMAND mode. Require OK response.
    else if(command.find("ATC")!=string::npos && mode == DATA)// && response == COMMAND_RECEIVED)
    {
        mode = COMMAND;
    }
}

// Send Instant Message to remote device.
void Driver::sendInstantMessage(SendIM const &im)
{
    string command = usblParser.parseSendIM(im);
    sendCommand(command);
    waitResponseOK(command);
}

// Parse a received Instant Message.
ReceiveIM Driver::receiveInstantMessage(string const &buffer)
{
    return usblParser.parseReceivedIM(buffer);
}

// TODO to be implemented
// Get the newest pose of remote device.
base::samples::RigidBodyState Driver::getNewPose(void)
{
    base::samples::RigidBodyState new_pose;
    new_pose.position[0] = usbl_pose.x;
    new_pose.position[1] = usbl_pose.y;
    new_pose.position[2] = usbl_pose.z;

    return new_pose;
}

// Get interface type.
InterfaceType Driver::getInterface(void)
{
    return interface;
}

// Define the interface with device. ETHERNET or SERIAL.
void Driver::setInterface(InterfaceType	deviceInterface)
{
    interface = deviceInterface;
}

// Get Underwater Connection Status.
ConnectionStatus Driver::getConnetionStatus(void)
{
    string command = "AT?S";
    sendCommand(command);
    return usblParser.parseConnectionStatus(waitResponseString(command));
}

// TODO parse input.
// Get Current Setting parameters.
void Driver::getCurrentSetting(void)
{
    string command = "AT&V";
    sendCommand(command);
    string response = waitResponseString(command);
}

// get Instant Message Delivery status.
DeliveryStatus Driver::getIMDeliveryStatus(void)
{
    string command = "AT?DI";
    sendCommand(command);
    return usblParser.parseDeliveryStatus(waitResponseString(command));
}

// Switch to COMMAND mode.
void Driver::GTES(void)
{
    string command = "+++";
    sendCommand(command);
    waitResponseOK(command);
    modeMsgManager(command);
}
