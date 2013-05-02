#include "UsblParser.hpp"
#include "Driver.hpp"
#include <iostream>
#include <boost/algorithm/string.hpp>
using namespace usbl_evologics;
UsblParser::UsblParser(InterfaceStatus* interfaceStatus){
    mInterfaceStatus = interfaceStatus;
}
void UsblParser::parseCommand(uint8_t const* data, size_t size){
    //std::cout << "PARSE LINE: " << data << std::endl; 
    char const* buffer_as_string = reinterpret_cast<char const*>(data);
    std::string s = std::string(buffer_as_string, size);
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
