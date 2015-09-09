#include "Driver.hpp"
#include <iostream>
#include <sstream>
//#include "Exceptions.hpp"

using namespace usbl_evologics;

Driver::Driver()
: iodrivers_base::Driver(max_packet_size)
{
    mode = DATA;
}

Driver::~Driver()
{
}

//bool Driver::sendData(bool &mailCommand)
//{
//    bool ret = sendCommand(mailCommand);
//    if(mode == DATA)
//    {
//        ret |= sendRawData();
//    }
//    return ret;
//}

WaitResponse Driver::sendCommand(void)
{
    if(!queueCommand.empty())
    {
        std::string command = queueCommand.front();

        std::cout << "Write Line: " << command << std::endl;
        std::string buffer = fillCommand(command);

        iodrivers_base::Driver::writePacket(reinterpret_cast<const uint8_t*>(buffer.c_str()), buffer.length());

        return modeManager(command);
    }
    return NO_RESPONSE_REQUIRED;
}

bool Driver::sendRawData(void)
{
    if(!queueRawData.empty())
    {   // TODO Need to check the size of the Raw data?
        // Maybe some data can be dropped if data's size is too big.
        std::string buffer = queueRawData.front();
        std::cout << "Send Raw Data "  << std::endl;
        bool result = iodrivers_base::Driver::writePacket(reinterpret_cast<const uint8_t*>(buffer.c_str()), buffer.length());
        if(result)
            // No need to wait for answer
            queueRawData.pop();
        return result;
    }
    return false;
}

void Driver::enqueueRawData(std::string const& raw_data)
{
    queueRawData.push(raw_data);
}

std::string Driver::fillCommand(std::string const &command)
{
    std::stringstream ss;
    std::string buffer;
    // Guard Time Escape Sequence (GTES) doesn't require <end-line>
    // GTES: (1s)<+++>(1s)
    if(command == "+++")
    {
        ss << command << std::flush;
        if(mode == DATA)
            sleep(1);
    }
    else
    {
        if (mode == DATA)
            // If in DATA, buffer = +++<AT command>
            ss << "+++" << std::flush;
        ss << addEndLine(command) << std::flush;
    }
    return ss.str();
}

std::string Driver::addEndLine(std::string const &command)
{
    std::stringstream ss;
    if (interface == SERIAL)
        ss << command << "\r" << std::flush;
    else // (interface == ETHERNET)
        ss << command << "\n" << std::flush;
    return ss.str();
}

int Driver::extractPacket(uint8_t const* buffer, size_t buffer_size) const
{
    std::string buffer_as_string = std::string(reinterpret_cast<char const*>(buffer), buffer_size);
    buffer_as_string = buffer_as_string.substr(0, buffer_size);

    // Both COMMAND and DATA mode, answer finish by \r\n.
    if(mode == DATA)
    {   // First, check for Notification or Response. Start with "+++AT"
        if(buffer_as_string.size() < 1 && buffer_as_string.find("+") != std::string::npos)
            return 0;
        else if(buffer_as_string.size() == 2 && buffer_as_string.find("++") != std::string::npos)
            return 0;
        else if(buffer_as_string.find("+++") != std::string::npos)
        {
            if(buffer_as_string.size() < 6)
                return 0;
            // Check for Notification
            else if(buffer_as_string.find("+++AT:") != std::string::npos)
            {   // Check for Notification.
                std::cout << "Check for Notification" << std::endl;
                int ret = checkRegularResponse(buffer_as_string);
                if(ret >= 0)
                    return ret;
            }
            else
            {   // Check for particular Response.
                int ret = checkParticularResponse(buffer_as_string);
                if(ret >= 0)
                    return ret;
                // Check for regular Response.
                ret = checkRegularResponse(buffer_as_string);
                if(ret >= 0)
                    return ret;
            }
        }
        // Treat as RAW DATA. Driver is transparent about it.
        return buffer_size;
    }
    // Check in COMMAND mode
    else
    {   // First, check for Notification
        int ret = checkNotificationCommandMode(buffer_as_string);
        if(ret >= 0)
            return ret;
        // Second, check for particular Response.
        ret = checkParticularResponse(buffer_as_string);
        if(ret >= 0)
            return ret;
        // Last, check for regular Response.
        ret = checkRegularResponse(buffer_as_string);
        if(ret >= 0)
            return ret;
        // Unexpected
        return -buffer_size;
    }
}

Answer Driver::readAnswer(void)
{
    Answer answer;
    answer.notification = NO_NOTIFICATION;
    answer.response = NO_RESPONSE;
    std::string buffer_as_string;
    Notification notification;
    CommandResponse response;

    int buffer_size = readInternal(buffer_as_string);

    //	std::cout<< "readAnswer. buffer.size "<< buffer_size <<" "<<buffer_as_string.size() << std::endl;
    //	std::cout<< buffer_size << " "<< buffer_as_string << std::endl;

    //	raw_data = buffer_as_string;

    if((answer.notification = isNotification(buffer_as_string)) != NO_NOTIFICATION)
    {
        interpretNotification(buffer_as_string, notification);
        std::cout<< "return2" << std::endl;
        answer.type = NOTIFICATION;
        return answer;
    }
    else if((answer.response = isResponse(buffer_as_string)) != NO_RESPONSE)
    {
        std::string command = queueCommand.front();
        interpretResponse(buffer_as_string, command, response);

        queueCommand.pop();
        std::cout<< "pop: " << command << std::endl;

        std::cout<< "return4" << std::endl;
        answer.type = RESPONSE;
        return answer;
    }
    else
    {
        answer.type = RAW_DATA;
        answer.raw_data = buffer_as_string;
        return answer;
    }
    std::cout<< "return5" << std::endl;
}

// Auxiliary function used by readAnswer().
int Driver::readInternal(std::string &buffer)
{
    uint8_t* read_buffer = new uint8_t[max_packet_size];
    int readpacket = iodrivers_base::Driver::readPacket(read_buffer, max_packet_size, 3000, 3000);

    if (readpacket > 0)
    {
        buffer.resize(readpacket);
        std::string str(read_buffer, read_buffer+readpacket);
        buffer = str;
    }
    delete[] read_buffer;
    return readpacket;
}

// Auxiliary function used by extractPacket().
// Response to command AT&V (getCurrentSetting) uses multiples
//  '\r\n' and a final '\r\n\r\n'. Damn EvoLogics.
// Maybe there are other command's responses that use the same pattern.
int Driver::checkParticularResponse(std::string const& buffer) const
{
    std::string command = queueCommand.front();
    if(command.find("AT&V") != std::string::npos)
    {
        std::cout << "checkParticularResponse" << std::endl;
        int eol = buffer.find("\r\n\r\n");
        if(eol != std::string::npos){
            std::cout<< "size particular response: " << eol+4 << std::endl;
            return eol+4;
        }
        else
            return 0;
    }
    return -1;
}

// Auxiliary function used by extractPacket().
// Usual response has an end-line by \r\n
int Driver::checkRegularResponse(std::string const& buffer) const
{
    std::cout << "checkRegularResponse" << std::endl;
    int eol = buffer.find("\r\n");
    if(eol != std::string::npos)
        // Add \r\n to buffer
        return eol+1;
    else
        return 0;
    return -1;
}

// Auxiliary function used by extractPacket().
// Check if a specific Notification string is present in buffer, just in COMMAND mode.
int Driver::checkNotificationCommandMode(std::string const& buffer) const
{
    if (buffer.size() < 4)
        return 0;
    else
    {   // All 4 letters notification
        if (buffer.find("RECV") !=std::string::npos ||
                buffer.find("DELI") !=std::string::npos ||
                buffer.find("FAIL") !=std::string::npos ||
                buffer.find("CANC") !=std::string::npos ||
                buffer.find("EXPI") !=std::string::npos ||
                buffer.find("SEND") !=std::string::npos ||
                buffer.find("USBL") !=std::string::npos ||
                buffer.find("BITR") !=std::string::npos ||
                buffer.find("SRCL") !=std::string::npos ||
                buffer.find("PHYO") !=std::string::npos ||
                buffer.find("RADD") !=std::string::npos )
            return checkRegularResponse(buffer);
        else
            return -1;
    }
}

// In DATA mode: +++AT:<length>:<notification><end-of-line>
// IN COMMAND mode: <notification><end-of-line>
// Return a valid kind of Notification. If not a notification return NO_NOTIFICATION.
// Throw ValidationError or ModeError in case of failure.
Notification Driver::isNotification(std::string const &buffer)
{
//    bool ret = false;
//    if(validNotification(buffer))
//        return usblParser.findNotification(buffer);
//    else
//        return NO_NOTIFICATION;
    Notification notification = usblParser.findNotification(buffer);
    if(notification != NO_NOTIFICATION)
    {
        if(mode == DATA)
            validNotification(buffer);
        fullValidation(buffer, notification);
    }
    return notification;
}

// In DATA mode: +++<AT command>:<length>:<command response><end-of-line>
// IN COMMAND mode: <response><end-of-line>
// Return a valid kind of Response. If not a response return NO_RESPONSE.
// Throw ValidationError or ModeError in case of failure.
CommandResponse Driver::isResponse(std::string const &buffer)
{
//    if(validResponse(buffer))
//        return usblParser.findResponse(buffer);
//    else
//        return NO_RESPONSE;
    CommandResponse response = usblParser.findResponse(buffer);
    // In DATA mode, all response starts by "+++AT". If there is no initial string, it isn't a response.
    if(mode == DATA && buffer.find("+++AT")==std::string::npos)
        response == NO_RESPONSE;
    else if(mode == DATA)
        // validResponse only works in DATA mode.
        validResponse(buffer);
    return response;
}

// In DATA mode: +++AT:<length>:<notification><end-of-line>
// IN COMMAND mode: <notification><end-of-line>
// Check for a valid notification in DATA mode only.
// Used by isNotification().
// Throw ValidationError or ModeError in case of failure.
void Driver::validNotification(std::string const &buffer)
{
    if(mode == DATA)
    {
        usblParser.validateNotification(buffer);
    }
    // Not possible to use UsblParser::validateNotification() in COMMAND mode.
    else
    {
        std::stringstream error_string;
        error_string << "USBL Driver.cpp validNotification: Function only can be called in DATA mode, not in COMMAND mode. Actual mode: \"" << mode << "\"" <<std::flush;
        throw ModeError(error_string.str());
    }
}

// In DATA mode: +++AT:<length>:<notification><end-of-line>
// IN COMMAND mode: <notification><end-of-line>
// Check for a valid notification both in DATA and COMMAND mode.
// Used by isNotification().
// Throw ValidationError in case of failure.
void Driver::fullValidation(std::string const &buffer, Notification const &notification)
{
    usblParser.splitValidateNotification(buffer, notification);
}

// In DATA mode: +++<AT command>:<length>:<command response><end-of-line>
// IN COMMAND mode: <response><end-of-line>
// Check for a valid response in DATA mode only.
// Used by isResponse().
// Throw ValidationError or ModeError in case of failure.
void Driver::validResponse(std::string const &buffer)
{
    if(mode == DATA)
    {
        std::string command = queueCommand.front();
        //		std::cout<< "validResponse "<< command << std::endl;
        usblParser.validateResponse(buffer, command);
    }
    // Not possible to use UsblParser::validateResponse() in COMMAND mode.
    else
    {
        std::stringstream error_string;
        error_string << "USBL Driver.cpp validResponse: Function only can be called in DATA mode, not in COMMAND mode. Actual mode: \"" << mode << "\"" <<std::flush;
        throw ModeError(error_string.str());
    }
}

std::string Driver::interpretNotification(std::string const& buffer, Notification const &notification)
{
    std::string ret;

    switch (notification) {
    case USBLLONG:
        usbl_pose = usblParser.parsePosition(buffer);
        break;
    case USBLANGLE:
        ret = "NOT POSSIBLE TO GET POSE";
        break;
    case RECVIM:
        receiveIM = usblParser.parseReceivedIM(buffer);
        break;
    case RECVIMS:
        //usblParser.parseReceivedIMS();
        ret = "NEW Synchronous IM. Not implemented";
        break;
    case RECVPBM:
        //usblParser.parseReceivedPBM();
        ret = "NEW PiggyBack M. Not implemented";
        break;
    case DELIVERY_REPORT:
        ret = usblParser.parseIMReport(buffer);
        break;
    case CANCELED_IM:
        ret = usblParser.parseIMReport(buffer);
        break;
    case EXTRA_NOTIFICATION:
        ret = "Extra Notification. Not implemented";
        break;
    }
    return ret;
}

std::string Driver::interpretResponse(std::string const& buffer, std::string const& command, CommandResponse const &response)
{
    std::string ret;
    std::stringstream ss;
    switch (response) {
    case ERROR:
        ss << "OK. ";
        ss << buffer << std::flush;
        ret = ss.str();
        break;
    case BUSY:
        ss << "TRY AGAIN. ";
        ss << buffer << std::flush;
        ret = ss.str();
        break;
    case COMMAND_RECEIVED:
        modeMsgManager(command);
        ss << "OK" << buffer << std::flush;
        ret = ss.str();
        break;
    case VALUE_REQUESTED:
        ss << "OK. " << buffer << ". ";
        ss << usblParser.parseRequestedValue(buffer, command) << std::flush;;
        ret = ss.str();
        break;
    }
    return ret;
}


WaitResponse Driver::modeManager(std::string const &command)
{
    // Command ATO. Switch from DATA to COMMAND mode doesn't require answer
    if(command.find("ATO")!=std::string::npos && mode == COMMAND)
    {
        // Proceed just after send the command
        queueCommand.pop();
        mode = DATA;
        return NO_RESPONSE_REQUIRED;
    }
    // Switch to COMMAND mode. (1s)<+++>(1s). Require OK response.
    else if(command.find("+++")!=std::string::npos && mode == DATA )
    {
        sleep(1);
        // switch to COMMAND mode after receive a OK answer
        //mode = COMMAND;
    }
    // Switch to COMMAND mode. Require OK response.
    else if(command.find("ATC")!=std::string::npos && mode == DATA)
    {
        //mode = COMMAND;
    }
    return RESPONSE_REQUIRED;
}

void Driver::modeMsgManager(std::string const &command)
{
    // Command ATO. Switch from DATA to COMMAND mode doesn't require answer
    if(command.find("ATO")!=std::string::npos && mode == COMMAND)
    {
        // Proceed just after send the command
        //queueCommand.pop();
        //mode = DATA;
    }
    // Switch to COMMAND mode. (1s)<+++>(1s). Require OK response.
    else if(command.find("+++")!=std::string::npos && mode == DATA)// && response == COMMAND_RECEIVED)
    {
        mode = COMMAND;
    }
    // Switch to COMMAND mode. Require OK response.
    else if(command.find("ATC")!=std::string::npos && mode == DATA)// && response == COMMAND_RECEIVED)
    {
        mode = COMMAND;
    }
}


InterfaceType Driver::getInterface(void)
{
    return interface;
}

void Driver::sendInstantMessage(SendIM const &im)
{
    queueCommand.push(usblParser.parseSendIM(im));
}

ReceiveIM Driver::receiveInstantMessage(std::string const &buffer)
{
    return usblParser.parseReceivedIM(buffer);
}

base::samples::RigidBodyState Driver::getNewPose(void)
{
    base::samples::RigidBodyState new_pose;
    new_pose.position[0] = usbl_pose.x;
    new_pose.position[1] = usbl_pose.y;
    new_pose.position[2] = usbl_pose.z;

    return new_pose;
}

void Driver::sendIMPose(base::samples::RigidBodyState const &send_pose)
{

}

void Driver::goSurface(void)
{
    SendIM im;
    im.deliveryReport = true;
    im.destination = 1;
    std::cout <<"test1" <<std::endl;
    std::string str = imParser.goSurface();
    std::vector<uint8_t> aux_buffer(str.begin(), str.end());
    std::cout <<"test2" <<std::endl;
    im.buffer = aux_buffer;
    sendInstantMessage(im);
    std::cout <<"test3" <<std::endl;
}

// Return amount of command to be sent to device.
int Driver::getSizeQueueCommand(void)
{
    return queueCommand.size();
}

// Return amount of raw data to be sent to remote device.
int Driver::getSizeQueueRawData(void)
{
    return queueRawData.size();
}

void Driver::setInterface(InterfaceType	deviceInterface)
{
    interface = deviceInterface;
}

void Driver::getConnetionStatus(void)
{
    queueCommand.push("AT?S");
}

void Driver::getCurrentSetting(void)
{
    queueCommand.push("AT&V");
}

void Driver::getIMDeliveryStatus(void)
{
    queueCommand.push("AT?DI");
}

void Driver::GTES(void)
{
    queueCommand.push("+++");
}
