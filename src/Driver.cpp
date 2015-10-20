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
            if (buffer.substr(length-2, 2) != "\r\n")
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
    {   // Buffer validation of indicated length was displaced for extract packet.
        // No need to do it here again.
        // Check if response and command match.
        return usblParser.getAnswerContent(response_info.buffer, command);
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

// Wait for a floating point response.
double Driver::waitResponseDouble(string const &command)
{
    return usblParser.getDouble(waitResponseString(command));
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
        // Buffer validation of indicated length was displaced for extract packet.
        // No need to do it here again.
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

// Check a valid notification.
void Driver::fullValidation(string const &buffer, Notification const &notification)
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
    waitResponseOK(command);
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

// Get the Position pose of remote device.
Position Driver::getPose(string const &buffer)
{
    return usblParser.parsePosition(buffer);
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
    return usblParser.parseConnectionStatus(waitResponseString(command));
}

// TODO parse input.
// Get Current Setting parameters.
DeviceSettings Driver::getCurrentSetting(void)
{
    string command = "AT&V";
    sendCommand(command);
    return usblParser.parseCurrentSettings(waitResponseString(command));
}

// get Instant Message Delivery status.
DeliveryStatus Driver::getIMDeliveryStatus(void)
{
    string command = "AT?DI";
    sendCommand(command);
    return usblParser.parseDeliveryStatus(waitResponseString(command));
}

// Delivery report notification for Instant Message.
bool Driver::getIMDeliveryReport(string const &buffer)
{
   return usblParser.parseIMReport(buffer);
}

// Switch to COMMAND mode.
void Driver::GTES(void)
{
    string command = "+++";
    sendCommand(command);
    waitResponseOK(command);
    modeMsgManager(command);
}

// Switch to COMMAND mode.
void Driver::switchToCommandMode(void)
{
    string command = "ATC";
    sendCommand(command);
    waitResponseOK(command);
    modeMsgManager(command);
}

// Switch to DATA mode.
void Driver::switchToDataMode(void)
{
    string command = "ATO";
    sendCommand(command);
    modeMsgManager(command);
}

// Reset device, drop data and/or instant message.
void Driver::resetDevice(ResetType const &type)
{
    stringstream command;
    command << "ATZ" << type;
    sendCommand(command.str());
    waitResponseOK(command.str());
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

// Set the specific carrier Waveform ID.
void Driver::setCarrierWaveformID(int value)
{
    string command = "AT!C";
    command.append(to_string(value));
    sendCommand(command);
    waitResponseOK(command);
}

// Set number of packets in one train.
void Driver::setClusterSize(int value)
{
    string command = "AT!ZC";
    command.append(to_string(value));
    sendCommand(command);
    waitResponseOK(command);
}

// Define limits of devices in the network.
void Driver::setHighestAddress(int value)
{
    string command = "AT!AM";
    command.append(to_string(value));
    sendCommand(command);
    waitResponseOK(command);
}

// The timeout before closing an idle acoustic connection.
void Driver::setIdleTimeout(int value)
{
    string command = "AT!ZI";
    command.append(to_string(value));
    sendCommand(command);
    waitResponseOK(command);
}

// Instant Message retry
void Driver::setIMRetry(int value)
{
    string command = "AT!RI";
    command.append(to_string(value));
    sendCommand(command);
    waitResponseOK(command);
}

// Address of local device
void Driver::setLocalAddress(int value)
{
    string command = "AT!AL";
    command.append(to_string(value));
    sendCommand(command);
    waitResponseOK(command);
}

// Address of remote device
void Driver::setRemoteAddress(int value)
{
    string command = "AT!AR";
    command.append(to_string(value));
    sendCommand(command);
    waitResponseOK(command);
}

// Get address of remote device
int Driver::getRemoteAddress(void)
{
    string command = "AT?AR";
    sendCommand(command);
    return waitResponseInt(command);
}

// Get highest address
int Driver::getHighestAddress(void)
{
    string command = "AT?AM";
    sendCommand(command);
    return waitResponseInt(command);
}

// Automatic positioning output
int Driver::getPositioningDataOutput(void)
{
    string command = "AT?ZU";
    sendCommand(command);
    return waitResponseInt(command);
}

// Enable or disable automatic positioning output
void Driver::setPositioningDataOutput(bool pose_on)
{
    string command = "AT!ZU";
    if(pose_on)
        command.append(to_string(1));
    else
        command.append(to_string(0));
    sendCommand(command);
    waitResponseOK(command);
}

// Set input amplifier gain
void Driver::setLowGain(bool low_gain)
{
    string command = "AT!G";
    if(low_gain)
        command.append(to_string(1));
    else
        command.append(to_string(0));
    sendCommand(command);
    waitResponseOK(command);
}

// Set maximum duration of a data packet.
void Driver::setPacketTime(int value)
{
    string command = "AT!ZP";
    command.append(to_string(value));
    sendCommand(command);
    waitResponseOK(command);
}

// Set if device will receive instant message addressed to others devices
void Driver::setPromiscuosMode(bool promiscuos_mode)
{
    string command = "AT!RP";
    if(promiscuos_mode)
        command.append(to_string(1));
    else
        command.append(to_string(0));
    sendCommand(command);
    waitResponseOK(command);
}

// Set number of connection establishment retries.
void Driver::setRetryCount(int value)
{
    string command = "AT!RC";
    command.append(to_string(value));
    sendCommand(command);
    waitResponseOK(command);
}

// Set time of wait for establish an acoustic connection
void Driver::setRetryTimeout(int value)
{
    string command = "AT!RT";
    command.append(to_string(value));
    sendCommand(command);
    waitResponseOK(command);
}

// Set Source Level
void Driver::setSourceLevel(SourceLevel source_level)
{
    string command = "AT!L";
    command.append(to_string((int)source_level));
    sendCommand(command);
    waitResponseOK(command);
}

// Set if source level of local device can be changed remotely over a acoustic connection.
void Driver::setSourceLevelcontrol(bool source_level_control)
{
    string command = "AT!LC";
    if(source_level_control)
        command.append(to_string(1));
    else
        command.append(to_string(0));
    sendCommand(command);
    waitResponseOK(command);
}

// Set speed of sound on water
void Driver::setSpeedSound(int value)
{
    string command = "AT!CA";
    command.append(to_string(value));
    sendCommand(command);
    waitResponseOK(command);
}

// Set active interval of acoustic channel monitoring.
void Driver::setWakeUpActiveTime(int value)
{
    string command = "AT!DA";
    command.append(to_string(value));
    sendCommand(command);
    waitResponseOK(command);
}

// Set hold timeout after completed data transmission.
void Driver::setWakeUpHoldTimeout(int value)
{
    string command = "AT!ZH";
    command.append(to_string(value));
    sendCommand(command);
    waitResponseOK(command);
}

// Set period of the acoustic channel monitoring cycle.
void Driver::setWakeUpPeriod(int value)
{
    string command = "AT!DT";
    command.append(to_string(value));
    sendCommand(command);
    waitResponseOK(command);
}

// Set transmission buffer size of actual data channel.
void Driver::setPoolSize(int value)
{
    resetDevice(SEND_BUFFER);
    string command = "AT@ZL";
    command.append(to_string(value));
    sendCommand(command);
    waitResponseOK(command);
}

// Reset Drop Counter.
void Driver::resetDropCounter(void)
{
    string command = "AT@ZD";
    sendCommand(command);
    waitResponseOK(command);
}

// Reset Overflow Counter.
void Driver::resetOverflowCounter(void)
{
    string command = "AT@ZO";
    sendCommand(command);
    waitResponseOK(command);
}

// Get firmware information of device.
VersionNumbers Driver::getFirmwareInformation(void)
{
    VersionNumbers info;
    string command = "ATI";
    stringstream ss;

    ss << command << VERSION_NUMBER;
    sendCommand(ss.str());
    info.firmwareVersion = waitResponseString(ss.str());
    // clean buffer
    ss.str(string());

    ss << command << PHY_MAC;
    sendCommand(ss.str());
    info.accousticVersion = waitResponseString(ss.str());
    // clean buffer
    ss.str(string());

    ss << command << MANUFACTURER;
    sendCommand(ss.str());
    info.manufacturer = waitResponseString(ss.str());
    // clean buffer
    ss.str(string());

    return info;
}

// Get last transmission's raw bitrate value of local-to-remote direction.
int Driver::getLocalToRemoteBitrate(void)
{
    string command = "AT?BL";
    sendCommand(command);
    return waitResponseInt(command);
}

// Get last transmission's raw bitrate value of remote-to-local direction.
int Driver::getRemoteToLocalBitrate(void)
{
    string command = "AT?BR";
    sendCommand(command);
    return waitResponseInt(command);
}

// Get Received Signal Strength Indicator.
double Driver::getRSSI(void)
{
    string command = "AT?E";
    sendCommand(command);
    return waitResponseDouble(command);
}

// Get Signal Integrity.
int Driver::getSignalIntegrity(void)
{
    string command = "AT?I";
    sendCommand(command);
    return waitResponseInt(command);
}

// Get acoustic signal's propagation time between communicating devices.
int Driver::getPropagationTime(void)
{
    string command = "AT?T";
    sendCommand(command);
    return waitResponseInt(command);
}

// Get relative velocity between communicating devices.
double Driver::getRelativeVelocity(void)
{
    string command = "AT?V";
    sendCommand(command);
    return waitResponseDouble(command);
}

// Get Multipath propagation structure.
std::vector<MultiPath> Driver::getMultipath(void)
{
    string command = "AT?P";
    sendCommand(command);
    return usblParser.parseMultipath(waitResponseString(command));
}

// Get dropCounter of actual channel
int Driver::getDropCounter(void)
{
    string command = "AT?ZD";
    sendCommand(command);
    return waitResponseInt(command);
}

// Get overflowCounter of actual channel
int Driver::getOverflowCounter(void)
{
    string command = "AT?ZO";
    sendCommand(command);
    return waitResponseInt(command);
}

// Get channel number of current interface.
int Driver::getChannelNumber(void)
{
    string command = "AT?ZS";
    sendCommand(command);
    return waitResponseInt(command);
}

// Set System Time for current time
void Driver::setSystemTimeNow(void)
{
    string command = "AT!UT";
    double time_now = base::Time::now().toSeconds();
    cout << "time_now "<< time_now <<endl;
    command.append(to_string(time_now));
    cout << "command string: " << command << endl;
    sendCommand(command);
    waitResponseOK(command);
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
