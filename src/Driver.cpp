#include "Driver.hpp"
#include <iostream>
#include <sstream>
#include "Exceptions.hpp"
using namespace usbl_evologics;
Driver::Driver()
    : iodrivers_base::Driver(1000)
{
}

ConnectionStatus Driver::getConnectionStatus(){
    sendWithLineEnding("+++AT?S");
    return (ConnectionStatus) (UsblParser::parseConnectionStatus(waitSynchronousString("AT?S"))); 
}

DeviceSettings Driver::getDeviceSettings(){
    DeviceSettings ds;
    ds.sourceLevel = getSourceLevel();
    ds.sourceLevelControl = getSourceLevelControl();
    ds.lowGain = getLowGain();
    ds.carrierWaveformId = getCarrierWaveformId();
    ds.localAddress = getLocalAddress();
    ds.clusterSize = getClusterSize();
    ds.packetTime = getPacketTime();
    ds.retryCount = getRetryCount();
    ds.retryTimeout = getRetryTimeout();
    ds.speedSound = getSpeedSound(); 
    ds.imRetry = getImRetry();
    ds.idleTimeout = getIdleTimeout();
    return ds;
}

DeviceStats Driver::getDeviceStatus(){
    DeviceStats  device_stats;
    device_stats.dropCount = getDropCounter();
    device_stats.overflowCount = getOverflowCounter();
    device_stats.localRemoteBitrate = getLocalRemoteBitrate();
    device_stats.remoteLocalBitrate = getRemoteLocalBitrate();
    device_stats.receivedSignalStrengthIndicator = getReceivedSignalStrengthIndicator();
    device_stats.signalIntegrityLevel = getSignalIntegrityLevel();
    device_stats.propagationTime = getPropagationTime();
    device_stats.relativeVelocity = getRelativeVelocity();
    return device_stats;
}

int Driver::getIntValue(std::string value_name){
    std::stringstream ss;
    ss << "+++" << value_name;
    std::string s = ss.str();
    sendWithLineEnding(s);
    return waitSynchronousInt(s.substr(3, s.size()));
}
DeliveryStatus Driver::getInstantMessageDeliveryStatus(){
    return currentInstantMessage.deliveryStatus;
}

Position Driver::getPosition(bool x){
    std::string position_string;
    if (x){
        sendWithLineEnding("+++AT?UPX");
        position_string = waitSynchronousString("AT?UPX");
    } else {
        sendWithLineEnding("+++AT?UP");
        position_string = waitSynchronousString("AT?UP");
    }
    return UsblParser::parsePosition(position_string);
}

int Driver::getSystemTime(){
    sendWithLineEnding("+++AT?UT");
    return waitSynchronousInt();
}
void Driver::open(std::string const& uri){
    buffer.resize(50);
    openURI(uri);
    if (uri.find("serial") != std::string::npos)
        interfaceType = SERIAL;
    else
        interfaceType = ETHERNET;
    resetDevice(SEND_BUFFER);
    requestVersionNumbers();
}

size_t Driver::read(uint8_t *buffer, size_t size){
    size_t packet_size = readPacket(buffer, size, 3000, 3000);
    std::string buffer_as_string = std::string(reinterpret_cast<char const*>(buffer));
    if (packet_size){
        std::cout << packet_size << std::endl;
        if (UsblParser::isPacket(buffer_as_string) > 0){
            if (handleAsynchronousCommand(buffer_as_string)){
                return 0;
            }
        }
        return packet_size;
    }
    return 0;
}

void Driver::sendBurstData(uint8_t const *buffer, size_t buffer_size){
    this->writePacket(buffer, buffer_size);
}

void Driver::sendInstantMessageInternal(SendInstantMessage &instantMessage){
    std::stringstream ss;
    ss << "+++AT*SENDIM," <<instantMessage.buffer.size()<<","<<instantMessage.destination<<",";
    if (instantMessage.deliveryReport){
        ss<<"ack,";
    } else {
        ss<<"noack,";
    }
    std::cout << "first part. buffer site "<< instantMessage.buffer.size() <<std::endl;
    std::string buffer_as_string = std::string( instantMessage.buffer.begin(), instantMessage.buffer.end());
    std::cout << "buffer" << std::endl;
    ss << buffer_as_string;
    std::string s = ss.str();
    sendWithLineEnding(s);
    std::cout << "gesendet und warte ..." << std::endl;
    waitSynchronousOk();
    std::cout << "Ok" << std::endl;
    instantMessage.deliveryStatus = PENDING;
}

void Driver::sendInstantMessage(SendInstantMessage instantMessage){
    if (currentInstantMessage.deliveryStatus == PENDING) {
        throw InstantMessagingError("Sending Instant Message while sending another.");
    }
    std::cout << "Wrote a Instant Message in Buffer" << std::endl;
    currentInstantMessage = instantMessage;
    sendInstantMessageInternal(currentInstantMessage);
    std::cout << "ENDE" << std::endl;
    //Looking for the stupid CANCELEDIM Message (two syncronous Messages are possible)
    //This shouldn't happen if you send only one message at time. But Maybe the Device was not fully resetted. 
    if (hasPacket()){
        std::vector<uint8_t> buffer;
        buffer.resize(1000);
        readInternal(&buffer[0], buffer.size());
    }
    if (currentInstantMessage.deliveryStatus == CANCELED){
        throw InstantMessagingError("CANCELED IM"); 
        //Make it sence to reset the device in this case?
    }
}
size_t Driver::getInboxSize(){
    return receivedInstantMessages.size();
}
ReceiveInstantMessage Driver::dropInstantMessage(){
    ReceiveInstantMessage im = receivedInstantMessages.at(0);
    receivedInstantMessages.erase(receivedInstantMessages.begin());
    return im;
}
void Driver::setDeviceSettings(DeviceSettings device_settings){
    setSourceLevel(device_settings.sourceLevel);
    setSourceLevelControl(device_settings.sourceLevelControl);
    setLowGain(device_settings.lowGain);
    setCarrierWaveformId(device_settings.carrierWaveformId);
    setLocalAddress(device_settings.localAddress);
    setRemoteAddress(device_settings.remoteAddress);
    setClusterSize(device_settings.clusterSize);
    setPacketTime(device_settings.packetTime);
    setRetryCount(device_settings.retryCount);
    setIdleTimeout(device_settings.idleTimeout);
    setSpeedSound(device_settings.speedSound);
    setImRetry(device_settings.imRetry);
}

void Driver::setSystemTime(int time){
    setValue("AT!UT", time);
}

void Driver::storeSettings(){
    sendWithLineEnding("+++AT&W");
    waitSynchronousOk();
}

int Driver::waitSynchronousInt(){
    std::vector<uint8_t> buffer;
    buffer.resize(1000);
    //TODO timeout
    while (true){
        if (size_t packet_size = readInternal(&buffer[0], buffer.size())){
            return UsblParser::parseInt(&buffer[0], packet_size);
        }
    }
}

int Driver::waitSynchronousInt(std::string command){
    std::vector<uint8_t> buffer;
    buffer.resize(1000);
    while (true){
        if (size_t packet_size = readInternal(&buffer[0], buffer.size())){
            return UsblParser::parseInt(&buffer[0], packet_size, command);
        }
    }
}

void Driver::waitSynchronousOk(){
    std::vector<uint8_t> buffer;
    buffer.resize(1000);
    //TODO timeout
    while (true){
        if (size_t packet_size = readInternal(&buffer[0], buffer.size())){
            UsblParser::parseOk(&buffer[0], packet_size);
            return;
        }
    } 
}

std::string Driver::waitSynchronousString(std::string command){
    std::vector<uint8_t> buffer;
    buffer.resize(1000);
    while (true){
        if (size_t packet_size = readInternal(&buffer[0], buffer.size())){
            return UsblParser::parseString(&buffer[0], packet_size, command);
        }
    }
}

//Set Settings
void Driver::setCarrierWaveformId(int id){
    validateValue(id, 0, 3);
    setValue("AT!C", id);
}
void Driver::setClusterSize(int size){
    validateValue(size, 1, 255);
    setValue("AT!ZC", size);
}
void Driver::setIdleTimeout(int timeout){
    validateValue(timeout, 0, 3600);
    setValue("AT!ZI", timeout);
}
void Driver::setImRetry(int retries){
    validateValue(retries, 0, 255);
    setValue("AT!RI", retries);
}
void Driver::setLocalAddress(int address){
    validateValue(address, 0, 254); 
    setValue("AT!AL", address);
}
void Driver::setLowGain(bool low_gain){
    if (low_gain){
        setValue("AT!G", 1);
    } else {
        setValue("AT!G", 0);
    }
}
void Driver::setPacketTime(int time){
    validateValue(time, 50, 100000);
    setValue("AT!ZP", time);
}
void Driver::setRemoteAddress(int address){
    validateValue(address, 0, 254);
    setValue("AT!AR", address);
}
void Driver::setRetryCount(int count){
    validateValue(count, 0, 255);
    setValue("AT!RC", count);
}
void Driver::setRetryTimeout(int timeout){
    validateValue(timeout, 500, 12000);
    setValue("AT!RT", timeout);
}
void Driver::setSourceLevel(int source_level){
    validateValue(source_level, 0, 3);
    setValue("AT!L", source_level);
}
void Driver::setSourceLevelControl(bool source_level_control){
    if (source_level_control){
        setValue("AT!LC", 1);
    } else {
        setValue("AT!LC", 0);
    }
}
void Driver::setSpeedSound(int speed){
    validateValue(speed, 1300, 1700);
    setValue("AT!CA", speed);
}

//Get Settings
int Driver::getCarrierWaveformId(){
    return getIntValue("AT?ZS");
}
int Driver::getClusterSize(){
    return getIntValue("AT?ZC");
}
int Driver::getIdleTimeout(){
    return getIntValue("AT?ZI");
}
int Driver::getImRetry(){
    return getIntValue("AT?RI");
}
int Driver::getLocalAddress(){
    return getIntValue("AT?AL");
}
int Driver::getLowGain(){
    return getIntValue("AT?G");
}
int Driver::getPacketTime(){
    return getIntValue("AT?ZP");
}
int Driver::getRemoteAddress(){
    return getIntValue("AT?AR");
}
int Driver::getRetryCount(){
    return getIntValue("AT?RC");
}
int Driver::getRetryTimeout(){
    return getIntValue("AT?RT");
}
int Driver::getSourceLevel(){
    return getIntValue("AT?L");
}
bool Driver::getSourceLevelControl(){
    return (bool) getIntValue("AT?LC");
}
int Driver::getSpeedSound(){
    return getIntValue("AT?CA");
}

//Get Stats
int Driver::getDropCounter(){
    return getIntValue("AT?ZD");
}
int Driver::getLocalRemoteBitrate(){
    return getIntValue("AT?BL");
}
int Driver::getOverflowCounter(){
    return getIntValue("AT?ZO");
}
int Driver::getPropagationTime(){
    return getIntValue("AT?T");
}
int Driver::getReceivedSignalStrengthIndicator(){
    return getIntValue("AT?E");
}
int Driver::getRelativeVelocity(){
    return getIntValue("AT?V");
}
int Driver::getRemoteLocalBitrate(){
    return getIntValue("AT?BR");
}
int Driver::getSignalIntegrityLevel(){
    return getIntValue("AT?I");
}

//Privats
void Driver::cancelIm(std::string s){
    currentInstantMessage.deliveryStatus = CANCELED;
}
int Driver::extractPacket(uint8_t const *buffer, size_t buffer_size) const
{
    std::string buffer_as_string = std::string(reinterpret_cast<char const*>(buffer), buffer_size);
    buffer_as_string = buffer_as_string.substr(0, buffer_size);
    int is_packet = UsblParser::isPacket(buffer_as_string);
    return abs(is_packet);
}

void Driver::incomingDeliveryReport(std::string s){
    if (currentInstantMessage.deliveryStatus != PENDING){
        std::cout << "warn: Get a Delivery Report, but has no pending Instant Message. Maybe the device was not fully resseted" << std::endl;
    }
    currentInstantMessage.deliveryStatus = UsblParser::parseDeliveryReport(s);
}

void Driver::incomingPosition(std::string s){
    //TODO initialize
    current_position = UsblParser::parseUsbllong(s);
}

void Driver::incomingInstantMessage(std::string s){
    ReceiveInstantMessage rim = UsblParser::parseIncomingIm(s);
    //TODO hier kann die uhrzeit nicht abgefragt werden alternative?
    //rim.time = getSystemTime();
    receivedInstantMessages.push_back(rim);
}
size_t Driver::readInternal(uint8_t *buffer, size_t buffer_size){
    size_t packet_size = readPacket(buffer, buffer_size, 3000, 3000);
    std::string buffer_as_string = std::string(reinterpret_cast<char const*>(buffer));
    if (packet_size){
        if (UsblParser::isPacket(buffer_as_string) > 0){
            if (!handleAsynchronousCommand(buffer_as_string)){
                //The Packet is not handled as asynchronous Command,
                //so there is a unhandled packet with size packet_size
                return packet_size;
            } else {
                //The packet is handled already as asynchronous Command
                //so there is no unhandled packet
                return 0;
            }

        } else {
            std::cout << " Buffer is burst data. Ignoring burst data when reading internal." << buffer << std::endl;
            return 0;
        }
    }
    return 0;
}
bool Driver::handleAsynchronousCommand(std::string buffer_as_string){
    switch (UsblParser::parseAsynchronousCommand(buffer_as_string)){
        case NO_ASYNCHRONOUS:
            return false;
        case DELIVERY_REPORT:
            std::cout << "There was a delivery report" << std::endl;
            incomingDeliveryReport(buffer_as_string);
            return true;
        case INSTANT_MESSAGE:
            std::cout << "There was a instant message" << std::endl;
            incomingInstantMessage(buffer_as_string);
            return true;
        case CANCELEDIM:
            std::cout << "There is a CANCLEDIM" << std::endl;
            cancelIm(buffer_as_string);
            return true;
        case USBLLONG:
            incomingPosition(buffer_as_string);
        case USBLANGLE:
            std::cout << "There is a USBL Asynch Message" << std::endl;
            //TODO Handle this, when possible (unknown protocol)
            return true;
    }
    return false;
}

void Driver::sendWithLineEnding(std::string line){
    std::stringstream ss;
    std::cout << "Write Line: " << line << std::endl;
    if (interfaceType == SERIAL){
        ss << line << "\r" << std::flush;
    } else {
        ss << line << "\n" << std::flush;
    }
    std::string s = ss.str();
    this->writePacket(reinterpret_cast<const uint8_t*>(s.c_str()), s.length());
}

void Driver::setValue(std::string value_name, int value){
    std::stringstream ss;
    ss << "+++" << value_name <<value;
    std::string s = ss.str();
    sendWithLineEnding(s);
    waitSynchronousOk();
}

void Driver::validateValue(int value, int min, int max){
    if (! (value >= min && value <= max)){
        std::stringstream error_string;
        error_string << "Expected value in a range from " << min << " to " << max << " but got " << value << std::flush;
        throw WrongInputValue(error_string.str());
    }
}
void Driver::resetDevice(ResetType reset){
    last_reset = base::Time::now();
    setValue("ATZ", (int) reset);
}
void Driver::requestVersionNumbers(){
    sendWithLineEnding("+++ATI0");
    versionNumbers.firmwareVersion = waitSynchronousString("ATI");
    sendWithLineEnding("+++ATI1");
    std::string phy_mac = waitSynchronousString("ATI");
    versionNumbers.accousticVersion = UsblParser::parsePhyNumber(phy_mac);
    versionNumbers.apiVersion = UsblParser::parseMacNumber(phy_mac);
}
VersionNumbers Driver::getVersionNumbers(){
    return versionNumbers;
}
