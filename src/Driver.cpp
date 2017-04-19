#include "Driver.hpp"
#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;
using namespace usbl_evologics;

Driver::Driver(const OperationMode &init_mode)
: iodrivers_base::Driver(max_packet_size), mode(init_mode)
{
}

Driver::Driver()
: Driver(DATA)
{
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
void Driver::sendRawData(vector<uint8_t> const &raw_data)
{
    iodrivers_base::Driver::writePacket(raw_data.data(), raw_data.size());
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
    // Max observed Particular Response: AT&V (get parameters) with 345 in length
    else if(buffer.size() > 400)
        throw runtime_error("CheckParticularResponse: Received a too big buffer of size \"" +to_string(buffer.size())+ "\". Check buffer \"" +usblParser.printBuffer(buffer)+ "\"");
    return 0;
}

// Check the size of regular response.
int Driver::checkRegularResponse(string const& buffer) const
{
    // Find <end-of-line>
    string::size_type eol = buffer.find("\r\n");
    if(eol != string::npos)
        // Add \r\n to buffer.
        return eol+2;
    // Max observed Response: USBLLONG (pose) with 118 in length
    else if(buffer.size() > 150)
        throw runtime_error("CheckRegularResponse: Received a too big buffer of size \"" +to_string(buffer.size())+ "\". Check buffer \"" +usblParser.printBuffer(buffer)+ "\"");
    return 0;
}

int Driver::extractATPacket(string const& buffer) const
{
    // Smallest packet possible is +++AT:0:\r\n
    if (buffer.size() < 10)
        return 0;
    if (buffer.substr(0, 5) != "+++AT")
        throw runtime_error("extractATPacket: Buffer does not start with \"+++AT\". Check buffer, \"" +usblParser.printBuffer(buffer)+ "\"");
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
                LOG_WARN("extractATPacket: Size Error. Found length \"%u\" doesn't match with buffer size of \"%s\" . Waiting more data in buffer", length, buffer.c_str());
                return 0;
            }
            // Check the presence of end-of-line (\r\n).
            if (buffer.substr(length-2, 2) != "\r\n")
                throw runtime_error("extractATPacket: Could not find <end-of-line> at position \"" +to_string((length-2))+ "\" of the end of buffer  \""+usblParser.printBuffer(buffer)+"\"");

            return length;
        }
        else if (buffer.size() > 16)
            throw runtime_error("extractATPacket: Assuming max lenght of 999, could not find second \":\" before 16 bytes in buffer, \""+usblParser.printBuffer(buffer)+"\"");
        return 0;
    }
    // Check max command size. +++AT?CLOCK:
    else if (buffer.size() > 12)
        throw runtime_error("extractATPacket: Could not find any \":\" before 12 bytes in buffer \""+usblParser.printBuffer(buffer)+"\"");
    return 0;
}

int Driver::extractRawFromATPackets(string const& buffer) const
{
    // TIES: Time Independent Escape Sequence
    const char* TIES_HEADER = "+++AT";
    const int   TIES_HEADER_SIZE = 5;

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
                // extract raw data present before the start of +++AT packet
                else
                    return extractRawDataPacket(buffer.substr(0, ties_start));
            }
            ++ties_start;
        }
        else if (string(TIES_HEADER, buffer_size - ties_start) == buffer.substr(ties_start, buffer_size - ties_start))
        {
            // Here, we have something that might be the start of a TIES header at the end of the buffer
            return extractRawDataPacket(buffer.substr(0, ties_start));
        }
        ++ties_start;
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
    // If data is neither notification or response, it's raw data.
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
string Driver::waitResponse(string const &expected_prefix, std::string const& command, CommandResponse expected, bool ignore_unexpected_responses)
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

	if (response_info.response == NO_RESPONSE)
	    continue;
	else if (mode == DATA && !expected_prefix.empty() && (response_info.buffer.substr(3, expected_prefix.size()) != expected_prefix))
	{
	    if (ignore_unexpected_responses)
		continue;
	    else
	        throw DeviceError("USBL Driver.cpp waitResponse: Expected response " + usblParser.printBuffer(response_info.buffer) + " to start with " + expected_prefix);
	}
        else if(response_info.response == ERROR)
            throw DeviceError("USBL Driver.cpp waitResponse: For the command: \""+ usblParser.printBuffer(command) +"\", device return the follow ERROR msg: \"" + usblParser.printBuffer(response_info.buffer) + "\"");
        else if(response_info.response == BUSY)
             throw BusyError("USBL Driver.cpp waitResponse: For the command: \""+ usblParser.printBuffer(command) +"\", device return the follow BUSY msg: \"" + usblParser.printBuffer(response_info.buffer) + "\". Try it latter.");
    }
    // Check for time_out
    if(response_info.response != expected)
        throw runtime_error("USBL Driver.cpp waitResponse: For the command: \""+ usblParser.printBuffer(command) +"\", device didn't send a response in " + to_string(time_out.toSeconds()) + " seconds time-out");

    // In DATA mode, validate response and return content without header.
    if(mode == DATA)
    {   // Buffer validation of indicated length was displaced for extract packet.
        // No need to do it here again.
        // Check if response and command match.
        return usblParser.getAnswerContent(response_info.buffer, command);
    }
    return response_info.buffer;
}

// Wait for a OK response.
void Driver::waitResponseOK(string const& expected_prefix, string const &command)
{
    waitResponse(expected_prefix, command, COMMAND_RECEIVED);
}

// Wait for a integer response.
int Driver::waitResponseInt(string const& expected_prefix, string const &command)
{
    return usblParser.getNumber(waitResponseString(expected_prefix, command));
}

// Wait for a floating point response.
double Driver::waitResponseDouble(string const& expected_prefix, string const &command)
{
    return usblParser.getDouble(waitResponseString(expected_prefix, command));
}

// Wait for a integer (that may be very long) response.
long long unsigned int Driver::waitResponseULLongInt(string const& expected_prefix, string const &command)
{
    return usblParser.getULLongInt(waitResponseString(expected_prefix, command));
}

// Wait for string response.
string Driver::waitResponseString(string const& expected_prefix, string const &command)
{
    return waitResponse(expected_prefix, command, VALUE_REQUESTED);
}

// Check if a Notification string is present in buffer.
int Driver::checkNotificationCommandMode(string const& buffer) const
{
    if (buffer.size() < 4)
        return 0;

    // All 4 letters notification
    // If Notification is a Received Message, the message part of string may contain malicious characters.
    // RECVxxx,<length>,<source address>,<destination address>,...<data><end-line>
    // <length> is size of <data>
    if (buffer.substr(0,4).find("RECV") !=string::npos)
    {
        if(buffer.size() < 7)
            return 0;
        Notification notification = usblParser.findNotification(buffer);
        if( notification == RECVIM ||
                notification == RECVIMS ||
                notification == RECVPBM )
            return checkIMNotification(buffer);
        else
            // It is a extra notification that starts with RECV (RECVSTART, RECVEND, ...)
            return checkRegularResponse(buffer);
    }
    // All other notifications.
    else if( buffer.find("DELI") !=string::npos ||
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

// Check if am Instant Message Notification string is present in buffer.
int Driver::checkIMNotification(string const& buffer) const
{
    if(buffer.size() < 7)
        return 0;
    Notification notification = usblParser.findNotification(buffer);
    if( notification != RECVIM && notification != RECVIMS && notification != RECVPBM )
        throw runtime_error("usbl Driver.cpp checkIMNotification: Notification should be a message, instead got \"" + usblParser.printBuffer(buffer) + "\"");

    // Get the amount of comma expected for a notification
    int ncomma = usblParser.getNumberFields(notification)-1;
    string::size_type npos = string::npos;
    string::size_type comma_1;
    size_t length = 0;
    size_t size_buffer = 0;
    for(int i=0; i<ncomma; i++)
    {
        // A comma in the last character of buffer. Wait for more data.
        if(size_buffer == buffer.size())
            return 0;
        // No more comma from here in buffer. Wait for more.
        if((npos = buffer.substr(size_buffer,buffer.size()-size_buffer).find(",")) == string::npos)
        {
            // The biggest fields in notification is a 32bits timestamp, which max value of 2^32 has 10 digits
            if((buffer.size()-size_buffer) > 10)
                return -1;
            return 0;
        }
        // Increase the correct part of buffer size
        size_buffer += (npos+1);
        // Get the first comma that encapsulate <length>
        if(i==0)
            comma_1 = npos;
        // Get length with the second comma that encapsulate <length>
        if(i==1)
            length += stoi(buffer.substr(comma_1+1, npos-comma_1),&npos);
    }
    // Buffer should have the size of the last comma plus the length of <data> and <end-line>
    length += (size_buffer+2);
    if(length > buffer.size())
    {
        LOG_WARN("Size Error. Found length %u doesn't match with buffer size of %s. Waiting more data in buffer ", length, buffer.c_str());
        return 0;
    }
    // Check for the <end-line>
    if(buffer.substr(length-2, 2) != "\r\n")
        throw runtime_error("Could not find <end-of-line> at position \""+ to_string(buffer.size()-3) + "\"of the end of buffer  \"" + usblParser.printBuffer(buffer) + "\"");
    return length;
}

// Check kind of notification.
Notification Driver::isNotification(string const &buffer)
{
    if(buffer.size() < 4)
        return NO_NOTIFICATION;
    string content;
    if(mode == DATA)
    {
        // in DATA mode +++AT:<length>:notification\r\n
        if(buffer.substr(0,6) != "+++AT:")
            return NO_NOTIFICATION;

        // Get content of Notification.
        content = usblParser.splitMinimalValidate(buffer, ":", 3)[2];
    }
    else
        content = buffer;
    // If buffer is Message Notification , the message part of buffer may contains malicious data.
    // If notification is a received Message, First 4 letters of Notification string are 'RECV'
    if(content.substr(0,4) == "RECV")
        // Notification is a Received Message, so there is a comma after notification string
        content = usblParser.splitMinimalValidate(content, ",", 2)[0];

    Notification notification = usblParser.findNotification(content);

    if(notification != NO_NOTIFICATION)
    {
        // Buffer validation of indicated length was displaced for extract packet.
        // No need to do it here again.
        // Check if notification has the predicted quantity of commas.
        notificationValidation(buffer, notification);
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

// Check a valid notification.
void Driver::notificationValidation(string const &buffer, Notification const &notification)
{
    usblParser.splitValidateNotification(buffer, notification);
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
    }
    // Switch to COMMAND mode. Require OK response.
    else if(command.find("ATC")!=string::npos && mode == DATA)
    {
        // Receive answer in COMMAND mode.
        mode = COMMAND;
    }
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
    else if(command.find("+++")!=string::npos && mode != COMMAND)
    {
        // If there was a error, keep in DATA mode.
        mode = DATA;
    }
    // Switch to COMMAND mode. Require OK response.
    else if(command.find("ATC")!=string::npos && mode != COMMAND)
    {
        // If there was a error, keep in DATA mode.
        mode = DATA;
    }
}

// Send Instant Message to remote device.
void Driver::sendInstantMessage(SendIM const &im)
{
    string command = usblParser.parseSendIM(im);
    sendCommand(command);
    waitResponseOK("AT*SENDIM", command);
}

// Get Instant Message parsed as string
string Driver::getStringOfIM(SendIM const &im)
{
    return fillCommand(usblParser.parseSendIM(im));
}

// Parse a received Instant Message.
ReceiveIM Driver::receiveInstantMessage(string const &buffer)
{
    return usblParser.parseReceivedIM(buffer);
}

// Get the RigidBodyState pose of remote device.
base::samples::RigidBodyState Driver::getPose(Position const &pose)
{
    base::samples::RigidBodyState new_pose;
    new_pose.time = pose.measurementTime;
    new_pose.position[0] = pose.x;
    new_pose.position[1] = pose.y;
    new_pose.position[2] = pose.z;
    base::Vector3d euler;
    euler[0] = pose.roll;
    euler[1] = pose.pitch;
    euler[2] = pose.yaw;
    new_pose.orientation = eulerToQuaternion(euler);

    return new_pose;
}

// Get the Position
// pose of remote device.
Position Driver::getPose(string const &buffer)
{
    return usblParser.parsePosition(buffer);
}

// Get the Direction of remote device.
Direction Driver::getDirection(string const &buffer)
{
    return usblParser.parseDirection(buffer);
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
AcousticConnection Driver::getConnectionStatus(void)
{
    string command = "AT?S";
    sendCommand(command);
    return usblParser.parseConnectionStatus(waitResponseString(command, command));
}

// TODO parse input.
// Get Current Setting parameters.
DeviceSettings Driver::getCurrentSetting(void)
{
    string command = "AT&V";
    sendCommand(command);
    return usblParser.parseCurrentSettings(waitResponseString(command, command));
}

// get Instant Message Delivery status.
DeliveryStatus Driver::getIMDeliveryStatus(void)
{
    string command = "AT?DI";
    sendCommand(command);
    return usblParser.parseDeliveryStatus(waitResponseString(command, command));
}

// Delivery report notification for Instant Message.
DeliveryStatus Driver::getIMDeliveryReport(string const &buffer)
{
   return usblParser.parseIMReport(buffer);
}

// Switch to COMMAND mode.
void Driver::GTES(void)
{
    string command = "+++";
    sendCommand(command);
    waitResponseOK("", command);
    modeMsgManager(command);
}

// Switch to COMMAND mode.
void Driver::switchToCommandMode(void)
{
    string command = "ATC";
    sendCommand(command);
    waitResponseOK("", command);
    modeMsgManager(command);
}

// Switch to DATA mode.
void Driver::switchToDataMode(void)
{
    string command = "ATO";
    sendCommand(command);
    usleep(1.5e6);
    modeMsgManager(command);
}

void Driver::resetDevice(ResetType const &type, bool ignore_unexpected_responses)
{
    string command = "ATZ" + to_string(type);
    sendCommand(command);

    // No command response
    if(type != DEVICE)
        waitResponse("ATZ", command, COMMAND_RECEIVED, ignore_unexpected_responses);

    if (type != INSTANT_MESSAGES)
        queueRawData = queue<string>();
    if (type != ACOUSTIC_CONNECTION)
        queueNotification = queue<NotificationInfo>();
}

// Pop out RawData from queueRawData.
std::vector<uint8_t> Driver::getRawData(void)
{
    if(!queueRawData.empty())
    {
        std::vector<uint8_t> ret(queueRawData.front().begin(), queueRawData.front().end());
        queueRawData.pop();
        return ret;
    }
    throw runtime_error("Driver.cpp getRawData: queueRawData is empty.");
}

// verify if queueRawData has raw data.
bool Driver::hasRawData(void)
{
    return !queueRawData.empty();
}

// Pop out Notification from queueNotification.
NotificationInfo Driver::getNotification(void)
{
    if(!queueNotification.empty())
    {
        NotificationInfo ret = queueNotification.front();
        queueNotification.pop();
        return ret;
    }
    throw runtime_error("Driver.cpp getNotification: queuenNotification is empty.");
}

// verify if queueNotification has any notification.
bool Driver::hasNotification(void)
{
    return !queueNotification.empty();
}

// Get mode of operation.
OperationMode Driver::getMode(void)
{
    return mode;
}

// Converts from euler angles to quaternions.
// TODO need validation and test.
base::Quaterniond Driver::eulerToQuaternion(const base::Vector3d &eulerAngles)
{
    base::Quaterniond quaternion =
            Eigen::AngleAxisd(eulerAngles(2), Eigen::Vector3d::UnitZ()) *
            Eigen::AngleAxisd(eulerAngles(1), Eigen::Vector3d::UnitY()) *
            Eigen::AngleAxisd(eulerAngles(0), Eigen::Vector3d::UnitX());

    return quaternion;
}

void Driver::sendCommandAndACK(string const& command, string const& parameters)
{
    sendCommand(command + parameters);
    waitResponseOK(command, command + parameters);
}

// Set the specific carrier Waveform ID.
void Driver::setCarrierWaveformID(int value)
{
    sendCommandAndACK("AT!C", to_string(value));
}

// Set number of packets in one train.
void Driver::setClusterSize(int value)
{
    sendCommandAndACK("AT!ZC", to_string(value));
}

// Define limits of devices in the network.
void Driver::setHighestAddress(int value)
{
    sendCommandAndACK("AT!AM", to_string(value));
}

// The timeout before closing an idle acoustic connection.
void Driver::setIdleTimeout(int value)
{
    sendCommandAndACK("AT!ZI", to_string(value));
}

// Instant Message retry
void Driver::setIMRetry(int value)
{
    sendCommandAndACK("AT!RI", to_string(value));
}

// Address of local device
void Driver::setLocalAddress(int value)
{
    sendCommandAndACK("AT!AL", to_string(value));
}

// Address of remote device
void Driver::setRemoteAddress(int value)
{
    sendCommandAndACK("AT!AR", to_string(value));
}

// Get address of remote device
int Driver::getRemoteAddress(void)
{
    string command = "AT?AR";
    sendCommand(command);
    return waitResponseInt(command, command);
}

// Get highest address
int Driver::getHighestAddress(void)
{
    string command = "AT?AM";
    sendCommand(command);
    return waitResponseInt(command, command);
}

// Automatic positioning output
int Driver::getPositioningDataOutput(void)
{
    string command = "AT?ZU";
    sendCommand(command);
    return waitResponseInt(command, command);
}

// Enable or disable automatic positioning output
void Driver::setPositioningDataOutput(bool pose_on)
{
    sendCommandAndACK("AT!ZU", (pose_on ? "1" : "0"));
}

// Set input amplifier gain
void Driver::setLowGain(bool low_gain)
{
    sendCommandAndACK("AT!G", (low_gain ? "1" : "0"));
}

// Set maximum duration of a data packet.
void Driver::setPacketTime(int value)
{
    sendCommandAndACK("AT!ZP", to_string(value));
}

// Set if device will receive instant message addressed to others devices
void Driver::setPromiscuosMode(bool promiscuos_mode)
{
    sendCommandAndACK("AT!RP", (promiscuos_mode ? "1" : "0"));
}

// Set number of connection establishment retries.
void Driver::setRetryCount(int value)
{
    sendCommandAndACK("AT!RC", to_string(value));
}

// Set time of wait for establish an acoustic connection
void Driver::setRetryTimeout(int value)
{
    sendCommandAndACK("AT!RT", to_string(value));
}

// Set Source Level
void Driver::setSourceLevel(SourceLevel source_level)
{
    sendCommandAndACK("AT!L", to_string(source_level));
}

// Set if source level of local device can be changed remotely over a acoustic connection.
void Driver::setSourceLevelControl(bool source_level_control)
{
    sendCommandAndACK("AT!LC", (source_level_control ? "1" : "0"));
}

// Get source level of device
SourceLevel Driver::getSourceLevel(void)
{
    string command = "AT?L";
    sendCommand(command);
    return (SourceLevel)waitResponseInt(command, command);
}

// Get source level control of device
bool Driver::getSourceLevelControl(void)
{
    string command = "AT?LC";
    sendCommand(command);
    return (waitResponseInt(command, command) == 1);
}

// Set speed of sound on water
void Driver::setSpeedSound(int value)
{
    sendCommandAndACK("AT!CA", to_string(value));
}

// Set active interval of acoustic channel monitoring.
void Driver::setWakeUpActiveTime(int value)
{
    sendCommandAndACK("AT!DA", to_string(value));
}

// Set hold timeout after completed data transmission.
void Driver::setWakeUpHoldTimeout(int value)
{
    sendCommandAndACK("AT!ZH", to_string(value));
}

// Set period of the acoustic channel monitoring cycle.
void Driver::setWakeUpPeriod(int value)
{
    sendCommandAndACK("AT!DT", to_string(value));
}

// Set transmission buffer size of actual data channel.
void Driver::setPoolSize(int value)
{
    resetDevice(SEND_BUFFER);
    sendCommandAndACK("AT@ZL", to_string(value));
}

// Reset Drop Counter.
void Driver::resetDropCounter(void)
{
    sendCommandAndACK("AT@ZD");
}

// Reset Overflow Counter.
void Driver::resetOverflowCounter(void)
{
    sendCommandAndACK("AT@ZO");
}

// Get firmware information of device.
VersionNumbers Driver::getFirmwareInformation(void)
{
    VersionNumbers info;

    string command = "ATI" + to_string(VERSION_NUMBER);
    sendCommand(command);
    info.firmwareVersion = waitResponseString("ATI", command);

    command = "ATI" + to_string(PHY_MAC);
    sendCommand(command);
    info.accousticVersion = waitResponseString("ATI", command);

    command = "ATI" + to_string(MANUFACTURER);
    sendCommand(command);
    info.manufacturer = waitResponseString("ATI", command);

    return info;
}

// Get last transmission's raw bitrate value of local-to-remote direction.
int Driver::getLocalToRemoteBitrate(void)
{
    string command = "AT?BL";
    sendCommand(command);
    return waitResponseInt(command, command);
}

// Get last transmission's raw bitrate value of remote-to-local direction.
int Driver::getRemoteToLocalBitrate(void)
{
    string command = "AT?BR";
    sendCommand(command);
    return waitResponseInt(command, command);
}

// Get Received Signal Strength Indicator.
double Driver::getRSSI(void)
{
    string command = "AT?E";
    sendCommand(command);
    return waitResponseDouble(command, command);
}

// Get Signal Integrity.
int Driver::getSignalIntegrity(void)
{
    string command = "AT?I";
    sendCommand(command);
    return waitResponseInt(command, command);
}

// Get acoustic signal's propagation time between communicating devices.
int Driver::getPropagationTime(void)
{
    string command = "AT?T";
    sendCommand(command);
    return waitResponseInt(command, command);
}

// Get relative velocity between communicating devices.
double Driver::getRelativeVelocity(void)
{
    string command = "AT?V";
    sendCommand(command);
    return waitResponseDouble(command, command);
}

// Get Multipath propagation structure.
std::vector<MultiPath> Driver::getMultipath(void)
{
    string command = "AT?P";
    sendCommand(command);
    return usblParser.parseMultipath(waitResponseString(command, command));
}

// Get dropCounter of actual channel
int Driver::getDropCounter(void)
{
    string command = "AT?ZD";
    sendCommand(command);
    return waitResponseInt(command, command);
}

// Get overflowCounter of actual channel
int Driver::getOverflowCounter(void)
{
    string command = "AT?ZO";
    sendCommand(command);
    return waitResponseInt(command, command);
}

// Get channel number of current interface.
int Driver::getChannelNumber(void)
{
    string command = "AT?ZS";
    sendCommand(command);
    return waitResponseInt(command, command);
}

// Get overall delivered raw data
long long unsigned int Driver::getRawDataDeliveryCounter(void)
{
    string command = "AT?ZE";
    sendCommand(command);
    return waitResponseULLongInt(command, command);
}

// Set System Time for current time
void Driver::setSystemTimeNow(void)
{
    double time_now = base::Time::now().toSeconds();
    sendCommandAndACK("AT!UT", to_string(time_now));
}

// Set operation mode of device
void Driver::setOperationMode(OperationMode const &new_mode)
{
    if(mode != new_mode)
    {
        if(new_mode == DATA)
            switchToDataMode();
        else if(new_mode == COMMAND)
            switchToCommandMode();
        else
        {
            stringstream ss;
            ss << "in Driver.cpp setOperationMode, can not identify mode \"" << new_mode << "\"" << flush;
            throw WrongInputValue(ss.str());
        }
    }
}

// Store current setting profile
void Driver::storeCurrentSettings(void)
{
    sendCommandAndACK("AT&W");
}

// Restore factory settings and reset device.
void Driver::RestoreFactorySettings(void)
{
    string command = "AT&F";
    sendCommand(command);
    if(mode == COMMAND)
        switchToDataMode();
    return;
}

// Get communication parameters
AcousticChannel Driver::getAcousticChannelparameters(void)
{
    AcousticChannel channel;
    channel.time = base::Time::now();
    channel.rssi = getRSSI();
    channel.localBitrate = getLocalToRemoteBitrate();
    channel.remoteBitrate = getRemoteToLocalBitrate();
    channel.propagationTime = getPropagationTime();
    channel.relativeVelocity = getRelativeVelocity();
    channel.signalIntegrity = getSignalIntegrity();
    channel.multiPath = getMultipath();
    channel.channelNumber = getChannelNumber();
    channel.dropCount = getDropCounter();
    channel.overflowCounter = getOverflowCounter();
    channel.delivered_raw_data = getRawDataDeliveryCounter();
    return channel;
}

// Update parameters on device.
void Driver::updateDeviceParameters(DeviceSettings const &desired_setting, DeviceSettings const &actual_setting)
{
    if(desired_setting.carrierWaveformId != actual_setting.carrierWaveformId)
         setCarrierWaveformID(desired_setting.carrierWaveformId);
     if(desired_setting.clusterSize != actual_setting.clusterSize)
         setClusterSize(desired_setting.clusterSize);
     if(desired_setting.highestAddress != actual_setting.highestAddress)
         setHighestAddress(desired_setting.highestAddress);
     if(desired_setting.idleTimeout != actual_setting.idleTimeout)
         setIdleTimeout(desired_setting.idleTimeout);
     if(desired_setting.imRetry != actual_setting.imRetry)
         setIMRetry(desired_setting.imRetry);
     if(desired_setting.localAddress != actual_setting.localAddress)
         setLocalAddress(desired_setting.localAddress);
     if(desired_setting.lowGain != actual_setting.lowGain)
         setLowGain(desired_setting.lowGain);
     if(desired_setting.packetTime != actual_setting.packetTime)
         setPacketTime(desired_setting.packetTime);
     if(desired_setting.promiscuosMode != actual_setting.promiscuosMode)
         setPromiscuosMode(desired_setting.promiscuosMode);
     if(desired_setting.remoteAddress != actual_setting.remoteAddress)
         setRemoteAddress(desired_setting.remoteAddress);
     if(desired_setting.retryCount != actual_setting.retryCount)
         setRetryCount(desired_setting.retryCount);
     if(desired_setting.retryTimeout != actual_setting.retryTimeout)
         setRetryTimeout(desired_setting.retryTimeout);
     if(desired_setting.speedSound != actual_setting.speedSound)
         setSpeedSound(desired_setting.speedSound);
     if(desired_setting.wuActiveTime != actual_setting.wuActiveTime)
         setWakeUpActiveTime(desired_setting.wuActiveTime);
     if(desired_setting.wuHoldTimeout != actual_setting.wuHoldTimeout)
         setWakeUpHoldTimeout(desired_setting.wuHoldTimeout);
     if(desired_setting.wuPeriod != actual_setting.wuPeriod)
         setWakeUpPeriod(desired_setting.wuPeriod);
     if(!actual_setting.poolSize.empty() && !desired_setting.poolSize.empty())
     {
         // Only takes in account the first and actual channel
         if(desired_setting.poolSize.at(0) != actual_setting.poolSize.at(0))
             setPoolSize(desired_setting.poolSize.at(0));
     }
}
