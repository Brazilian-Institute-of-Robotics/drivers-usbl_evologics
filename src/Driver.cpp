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
    //setInterfaceToConfigMode();
}

void Driver::read(){
    std::cout << "READ" << std::endl;
    int packet_size = readPacket(&buffer[0], buffer.size());
    if (packet_size){
        mParser->parseCommand(&buffer[0], packet_size);
    }
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
