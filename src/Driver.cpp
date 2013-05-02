#include "Driver.hpp"
#include <iostream>

using namespace usbl_evologics;
Driver::Driver()
    : iodrivers_base::Driver(1000000)
{
    mParser = new UsblParser(&mInterfaceStatus);
}
void Driver::open(std::string const& uri){
    buffer.resize(10000000);
    openURI(uri);
    mInterfaceStatus.interfaceMode = CONFIG_MODE;
    //setInterfaceToConfigMode();
}
void Driver::read(){
    std::cout << "READ" << std::endl;
    int packet_size = readPacket(&buffer[0], buffer.size());
    if (packet_size){
        if (mInterfaceStatus.interfaceMode != BURST_MODE){
            mParser->parseCommand(&buffer[0], packet_size);
        } else {
            //TODO rausrotzen
        }
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
void Driver::sendInstantMessage(struct InstantMessage *instantMessage){
    setInterfaceToConfigMode();
    //TODO instantMessage senden
    std::cout << "Pending Messages" << mInterfaceStatus.instantMessages.size() << std::endl;
    mInterfaceStatus.instantMessages.push_back(instantMessage);
    mInterfaceStatus.pending = PENDING_OK;
    waitSynchronousMessage();
}
void Driver::setInterfaceToBurstMode(){
    if (mInterfaceStatus.interfaceMode != BURST_MODE) {
        //TODO "ATO" senden
        mInterfaceStatus.pending = PENDING_OK;
        waitSynchronousMessage();
    }
}
void Driver::waitSynchronousMessage(){
    while (mInterfaceStatus.pending != NO_PENDING){
        //TODO timeout
        read();
        if (mInterfaceStatus.pending == ERROR){
            mInterfaceStatus.pending = NO_PENDING;
            std::cout << "RAISE ERROR" << std::endl;
            //TODO raise Exception
        }
    }

}
void Driver::setInterfaceToConfigMode(){
    if (mInterfaceStatus.interfaceMode != CONFIG_MODE) {
        //TODO "+++ATC" senden
        mInterfaceStatus.pending = PENDING_OK;
        waitSynchronousMessage();
    }
}
void Driver::sendBurstData(uint8_t const *buffer, size_t buffer_size)
{
    setInterfaceToBurstMode();
    //TODO just sending
}
struct Position Driver::requestPosition(bool x){
    setInterfaceToConfigMode();
    if (x){
        //TODO send +++UPX
    } else {
        //TODO send +++UP
    }
    mInterfaceStatus.pending = PENDING_POSITION;
    waitSynchronousMessage();
    return mInterfaceStatus.position;
}
struct Position Driver::getPosition(){
    return mInterfaceStatus.position;
}
