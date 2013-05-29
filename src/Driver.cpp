#include "Driver.hpp"
#include <iostream>
#include <sstream>

using namespace usbl_evologics;
/*void UsblDriverCallbacks::gotInstantMessage(struct ReceiveInstantMessage *im){
    std::cout << "Default Instant Message Callback" << std::endl;
}

void UsblDriverCallbacks::gotBurstData(uint8_t const *data, size_t data_size){
    std::cout << "Default Burst Data Callback" << std::endl;
}*/
Driver::Driver()
    : iodrivers_base::Driver(50)
{
    mParser = new UsblParser(&mInterfaceStatus);
    mCallbacks = NULL;
}

void Driver::open(std::string const& uri){
    buffer.resize(50);
    openURI(uri);
    mInterfaceStatus.interfaceMode = CONFIG_MODE;
    //Ask the Device for the maximum address
    //setInterfaceToConfigMode();
}

void Driver::read(){
    std::cout << "READ" << std::endl;
    int packet_size = readPacket(&buffer[0], buffer.size());
    if (packet_size){
        mParser->parseCommand(&buffer[0], packet_size);
    }
}
void Driver::setSettings(struct DeviceSettings device_settings){
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

int Driver::extractPacket(uint8_t const *buffer, size_t buffer_size) const
{
    //If we aren't in the burstmode every line comes in as a command
    if (mInterfaceStatus.interfaceMode != BURST_MODE){
        char const* buffer_as_string = reinterpret_cast<char const*>(buffer);
        for (int eol = 0; eol < buffer_size; eol++){
            if (buffer_as_string[eol] == '\n'){
                return eol+1;
            }
        }
    }
    //If we are in the burstmode everything is just data
    else {
        return buffer_size;
    }
}

void Driver::sendInstantMessage(struct SendInstantMessage *instantMessage){
    setInterfaceToConfigMode();
    //TODO instantMessage senden
    std::stringstream ss;
    ss << "AT*SENDIM," <<instantMessage->len<<","<<instantMessage->destination<<",";
    if (instantMessage->deliveryReport){
        ss<<"ack,";
    } else {
        ss<<"noack,";
    }
    ss<<instantMessage<<instantMessage->buffer;
    std::string s = ss.str();
    this->writePacket(reinterpret_cast<const uint8_t*>(s.c_str()), s.length());
    std::cout << "Pending Messages" << mInterfaceStatus.instantMessages.size() << std::endl;
    mInterfaceStatus.instantMessages.push_back(instantMessage);
    mInterfaceStatus.pending = PENDING_OK;
    waitSynchronousMessage();
}

void Driver::setInterfaceToBurstMode(){
    if (mInterfaceStatus.interfaceMode != BURST_MODE) {
        //"ATO" senden
        this->writePacket(reinterpret_cast<const uint8_t*>("ATO"), 3);
        mInterfaceStatus.pending = PENDING_OK;
        waitSynchronousMessage();
        mInterfaceStatus.interfaceMode = BURST_MODE;
    }
}

void Driver::waitSynchronousMessage(){
    std::cout << "Wait for Syncronous Message" << std::endl;
    while (mInterfaceStatus.pending != NO_PENDING){
        //TODO timeout
        read();
        if (mInterfaceStatus.pending == ERROR){
            mInterfaceStatus.pending = NO_PENDING;
            std::cout << "RAISE ERROR" << std::endl;
            //TODO raise Exception
        }
    }
    std::cout << "end waiting" << std::endl;

}

void Driver::setInterfaceToConfigMode(){
    if (mInterfaceStatus.interfaceMode != CONFIG_MODE) {
        // "+++ATC" senden
        this->writePacket(reinterpret_cast<const uint8_t*>("+++ATC"), 6);
        mInterfaceStatus.pending = PENDING_OK;
        waitSynchronousMessage();
        mInterfaceStatus.interfaceMode = CONFIG_MODE;
    }
}

void Driver::sendBurstData(uint8_t const *buffer, size_t buffer_size)
{
    setInterfaceToBurstMode();
    //just sending
    this->writePacket(buffer, buffer_size);
}

struct Position Driver::requestPosition(bool x){
    setInterfaceToConfigMode();
    if (x){
        this->writePacket(reinterpret_cast<const uint8_t*>("AT?UPX"), 6);
        //send AT?UPX
    } else {
        this->writePacket(reinterpret_cast<const uint8_t*>("AT?UP"), 5);
        //send AT?UP
    }
    mInterfaceStatus.pending = PENDING_POSITION;
    waitSynchronousMessage();
    return mInterfaceStatus.position;
}

struct Position Driver::getPosition(){
    return mInterfaceStatus.position;
}

int Driver::getSystemTime(){
    setInterfaceToConfigMode();
    //send AT?UT
    this->writePacket(reinterpret_cast<const uint8_t*>("AT?UT"), 5);
    mInterfaceStatus.pending = PENDING_TIME;
    waitSynchronousMessage();
    return mInterfaceStatus.time;
}
struct DeviceSettings Driver::getDeviceSettings(){
    setInterfaceToConfigMode();
    this->writePacket(reinterpret_cast<const uint8_t*>("AT&V"), 4);
    mInterfaceStatus.pending = PENDING_SETTINGS;
    waitSynchronousMessage();
    return mInterfaceStatus.deviceSettings;
}
void Driver::setSystemTime(int time){
    setInterfaceToConfigMode();
    std::stringstream ss;
    ss << "AT!UT" << time;
    std::string s = ss.str();
    this->writePacket(reinterpret_cast<const uint8_t*>(s.c_str()), s.length());
    //send AT!UT<time>
    mInterfaceStatus.pending = PENDING_OK;
    waitSynchronousMessage();
    mInterfaceStatus.time = time;
}

void Driver::setDriverCallbacks(UsblDriverCallbacks *cb){
   mCallbacks = cb; 
   mParser->setCallbacks(cb);
}

void Driver::storeSettings(){
    setInterfaceToConfigMode();
    this->writePacket(reinterpret_cast<const uint8_t*>("AT&W"), 4);
    mInterfaceStatus.pending = PENDING_OK;
    waitSynchronousMessage();
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
void Driver::setLowGain(bool low_gain){
    if (low_gain){
        setValue("AT!G", 1);
    } else {
        setValue("AT!G", 0);
    }
}
void Driver::setCarrierWaveformId(int id){
    validateValue(id, 0, 3);
    setValue("AT!C", id);
}
void Driver::setLocalAddress(int address){
    validateValue(address, 0, 254); //TODO maximum address
    setValue("AT!AL", address);
}
void Driver::setRemoteAddress(int address){
    validateValue(address, 0, 254); //TODO maximum address
    setValue("AT!AR", address);
}
void Driver::setClusterSize(int size){
    validateValue(size, 1, 255);
    setValue("AT!ZC", size);
}
void Driver::setPacketTime(int time){
    validateValue(time, 50, 1000);
    setValue("AT!ZP", time);
}
void Driver::setRetryCount(int count){
    validateValue(count, 0, 255);
    setValue("AT!RC", count);
}
void Driver::setRetryTimeout(int timeout){
    validateValue(timeout, 500, 12000);
    setValue("AT!RT", timeout);
}
void Driver::setIdleTimeout(int timeout){
    validateValue(timeout, 0, 3600);
    setValue("AT!ZI", timeout);
}
void Driver::setSpeedSound(int speed){
    validateValue(speed, 1300, 1700);
    setValue("AT!CA", speed);
}
void Driver::setImRetry(int retries){
    validateValue(retries, 0, 255);
    setValue("AT!RI", retries);
}
void Driver::setValue(std::string value_name, int value){
    std::stringstream ss;
    ss << value_name <<value;
    std::string s = ss.str();
    this->writePacket(reinterpret_cast<const uint8_t*>(s.c_str()), s.length());
    mInterfaceStatus.pending = PENDING_OK;
    waitSynchronousMessage();
}
void Driver::validateValue(int value, int min, int max){
    if (! (value >= min && value <= max)){
        //TODO raise error
    }
}
