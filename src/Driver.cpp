#include "Driver.hpp"
#include <iostream>
#include <sstream>

using namespace usbl_evologics;
Driver::Driver()
    : iodrivers_base::Driver(50)
{
    mParser = new UsblParser();
}

ConnectionStatus Driver::getConnectionStatus(){
    sendWithLineEnding("+++AT?S");
    return (ConnectionStatus) (mParser->parseConnectionStatus(waitSynchronousString("AT?S"))); 
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
}

int Driver::getIntValue(std::string value_name){
    std::stringstream ss;
    ss << "+++" << value_name;
    std::string s = ss.str();
    sendWithLineEnding(s);
    return waitSynchronousInt(s.substr(3, s.size()));
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
    return mParser->parsePosition(position_string);
}

int Driver::getSystemTime(){
    sendWithLineEnding("+++AT?UT");
    return waitSynchronousInt();
}
void Driver::open(std::string const& uri){
    buffer.resize(50);
    openURI(uri);
    interfaceType = ETHERNET;
}

size_t Driver::read(uint8_t *buffer, size_t size){
    return readInternal(buffer, size);
}

void Driver::sendBurstData(uint8_t const *buffer, size_t buffer_size){
    this->writePacket(buffer, buffer_size);
}

void Driver::sendInstantMessage(SendInstantMessage *instantMessage){
    std::stringstream ss;
    ss << "+++AT*SENDIM," <<instantMessage->len<<","<<instantMessage->destination<<",";
    if (instantMessage->deliveryReport){
        ss<<"ack,";
    } else {
        ss<<"noack,";
    }
    ss<<instantMessage->buffer;
    std::string s = ss.str();
    sendWithLineEnding(s);
    sendInstantMessages.push_back(instantMessage);
    waitSynchronousOk();
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
    std::stringstream ss;
    setValue("AT!UT", time);
    waitSynchronousOk();
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
            return mParser->parseInt(&buffer[0], packet_size);
        }
    }
}

int Driver::waitSynchronousInt(std::string command){
    std::vector<uint8_t> buffer;
    buffer.resize(1000);
    while (true){
        if (size_t packet_size = readInternal(&buffer[0], buffer.size())){
            return mParser->parseInt(&buffer[0], packet_size, command);
        }
    }
}

void Driver::waitSynchronousOk(){
    std::vector<uint8_t> buffer;
    buffer.resize(1000);
    //TODO timeout
    while (true){
        if (size_t packet_size = readInternal(&buffer[0], buffer.size())){
            mParser->parseOk(&buffer[0], packet_size);
            return;
        }
    } 
}

std::string Driver::waitSynchronousString(std::string command){
    std::vector<uint8_t> buffer;
    buffer.resize(1000);
    while (true){
        if (size_t packet_size = readInternal(&buffer[0], buffer.size())){
            return mParser->parseString(&buffer[0], packet_size, command);
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
    validateValue(address, 0, 254); //TODO maximum address
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
    validateValue(time, 50, 1000);
    setValue("AT!ZP", time);
}
void Driver::setRemoteAddress(int address){
    validateValue(address, 0, 254); //TODO maximum address
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
    return getIntValue("AT?C");
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
int Driver::extractPacket(uint8_t const *buffer, size_t buffer_size) const
{
    std::string buffer_as_string = std::string(reinterpret_cast<char const*>(buffer));
    std::cout << buffer_as_string << std::endl;
    buffer_as_string = buffer_as_string.substr(0, buffer_size);
    int is_packet = mParser->isPacket(buffer_as_string);
    if (is_packet < 0){
        std::cout << is_packet <<"   " << (-1*is_packet)<< std::endl;
        return (-1*is_packet);
    } else if (is_packet > 0) {
        return is_packet;
    } return 0;
}

void Driver::incommingDeliveryReport(std::string s){
    //TODO check the fit delivery report and instant message
    sendInstantMessages.at(0)->deliveryStatus = mParser->parseDeliveryReport(s);
    sendInstantMessages.erase(sendInstantMessages.begin());
}

void Driver::incommingInstantMessage(std::string s){
    ReceiveInstantMessage rim = mParser->parseIncommingIm(s);
    rim.time = getSystemTime();
    receivedInstantMessages.push_back(rim);
}
size_t Driver::readInternal(uint8_t *buffer, size_t buffer_size){
    size_t packet_size = readPacket(buffer, buffer_size);
    std::string buffer_as_string = std::string(reinterpret_cast<char const*>(buffer));
    if (packet_size){
        if (mParser->isPacket(buffer_as_string) > 0){
            switch (mParser->parseAsynchronousCommand(buffer_as_string)){
                case NO_ASYNCHRONOUS:
                    return packet_size;
                    break;
                case DELIVERTY_REPORT:
                    incommingDeliveryReport(buffer_as_string);
                    return 0;
                    break;
                case INSTANT_MESSAGE:
                    incommingInstantMessage(buffer_as_string);
                    return 0;
                    break;
            }
        } else {
            //Ignoring Burst Data if reading internal 
            return 0;
        }
    }
    return 0;
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
    //TODO wieder rein nehmen
    //this->writePacket(reinterpret_cast<const uint8_t*>(s.c_str()), s.length());
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
        //TODO raise error
    }
}





