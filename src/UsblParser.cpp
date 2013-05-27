#include "UsblParser.hpp"
#include <iostream>
#include <boost/algorithm/string.hpp>
using namespace usbl_evologics;
UsblParser::UsblParser(InterfaceStatus* interfaceStatus){
    mCallbacks = NULL;
    mInterfaceStatus = interfaceStatus;
}
void UsblParser::setCallbacks(UsblDriverCallbacks* cb){
    mCallbacks = cb; 
}
void UsblParser::parseCommand(uint8_t const* data, size_t size){
    char const* buffer_as_string = reinterpret_cast<char const*>(data);
    std::string s = std::string(buffer_as_string, size);
    if (mInterfaceStatus->interfaceMode == CONFIG_MODE){
        std::cout << "PARSE LINE: " << s  << " in CONFIG_MODE" << std::endl;
        parseConfigCommand(s);
    } else {
        std::cout << "Got Burstdata: " << s << std::endl;
        if (mCallbacks) {
            mCallbacks->gotBurstData(data, size);
        } else {
            std::cout << "Warning unhandles Burstdata, because unsetted callbacks" << std::endl;
        }
    }
} 
void UsblParser::parseConfigCommand(std::string s){
    if (s.find("DELIVEREDIM") != std::string::npos || s.find("FAILEDIM") != std::string::npos){
        parseDeliveryReport(s);
        return;
    } 
    if (s.find("RECVIM") != std::string::npos){
        parseIncommingIm(s);
        return;
    }
    //Wenn wir auf ein OK warten
    if (mInterfaceStatus->pending == PENDING_OK){

        if (s.find("OK") != std::string::npos){
            mInterfaceStatus->pending = NO_PENDING;
        } else if (s.find("ERROR") != std::string::npos){
            mInterfaceStatus->pending = ERROR;
        }
    } else if (mInterfaceStatus->pending == PENDING_POSITION){
        parsePosition(s);
    }
}
void UsblParser::parsePosition(std::string s){
    //remove possible prefix
    if (s.find("+++AT?UP:") != std::string::npos){
        s = s.substr(9, s.size()-10);
    } else if (s.find("+++AT?UPX:") != std::string::npos){
        s = s.substr(10, s.size()-11);
    }
    //to split
    std::vector<std::string> splitted;
    boost::split( splitted, s, boost::algorithm::is_any_of( "," ) );
    if (splitted.size() == 5){
        mInterfaceStatus->position.time = atoi(splitted.at(0).c_str());
        mInterfaceStatus->position.x = atof(splitted.at(2).c_str());
        mInterfaceStatus->position.y = atof(splitted.at(3).c_str());
        mInterfaceStatus->position.z = atof(splitted.at(4).c_str());
        mInterfaceStatus->pending = NO_PENDING;
    } else {
        mInterfaceStatus->pending = ERROR;
    }
}
void UsblParser::parseDeliveryReport(std::string s){
    std::vector<std::string> splitted;
    boost::split( splitted, s, boost::algorithm::is_any_of( "," ) );
    if (splitted.size() != 2){
        std::cout << "raise Error"<< std::endl;
        return;
    }
    int remote = atoi(splitted.at(1).c_str());
    std::cout <<  "DELIVERY REPORT FROM HOST" << remote <<std::endl;
    enum DeliveryStatus temp;
    if (splitted.at(0).find("DELIVEREDIM") != std::string::npos) {
        temp = DELIVERED;
    } else if (splitted.at(0).find("FAILEDIM") != std::string::npos){
        temp = FAILED;
    }
    for (unsigned i = 0; i < mInterfaceStatus->instantMessages.size(); i++){
        if (remote == mInterfaceStatus->instantMessages.at(i)->destination &&
                mInterfaceStatus->instantMessages.at(i)->delivery_report){
            mInterfaceStatus->instantMessages.at(i)->deliveryStatus = temp;
            mInterfaceStatus->instantMessages.erase(mInterfaceStatus->instantMessages.begin()+i);
        }

    }
} 
void UsblParser::parseIncommingIm(std::string s){
    std::vector<std::string> splitted;
    boost::split( splitted, s, boost::algorithm::is_any_of( "," ) );
    if (splitted.size() != 11){
        std::cout << "raise Error"<< std::endl;
        return;
    }
    struct ReceiveInstantMessage im;
    im.len = atoi(splitted.at(1).c_str());
    im.source = atoi(splitted.at(2).c_str());
    im.destination = atoi(splitted.at(3).c_str());
    const char *buffer = splitted.at(10).c_str();
    /*for (int i = 0; i < im.len; i++){
        im.buffer[i] = (uint8_t) buffer[i];
    }*/

    std::cout << "INSTANT MESSAGE FROM HOST" << im.source << std::endl;
    if (mCallbacks) {
        mCallbacks->gotInstantMessage(&im);
    } else {
        std::cout << "Warning unhandled InstantMessage, because unsetted callbacks" << std::endl;
    }

}
