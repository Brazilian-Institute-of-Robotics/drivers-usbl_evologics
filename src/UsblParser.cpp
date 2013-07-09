#include "UsblParser.hpp"
#include <iostream>
#include <boost/algorithm/string.hpp>
using namespace usbl_evologics;
UsblParser::UsblParser(){
}

int UsblParser::isPacket(std::string s){
    std::size_t escape_sequenz_position = s.find("+++");
    if (escape_sequenz_position == std::string::npos){
        //there is definitly no command
        return -1*s.size();
    } else if (escape_sequenz_position != 0){
        //there is maybe a command but isn't starting at position 0
        return -1*escape_sequenz_position;
    } else {
        //there is a command starting at position 0
        for (size_t eol = 0; eol < s.size(); eol++){
            //The Buffer is until the Lineending a Command 
            if (s.at(eol) == '\n'){
                //TODO try except
                validateResponse(s.substr(0, eol+1));
                return eol+1;
            }
        }
        //TODO gucken obs noch ein command werden kann
        return 0;
    } 
}

AsynchronousMessages UsblParser::parseAsynchronousCommand(std::string s){
    if (s.find("DELIVEREDIM") != std::string::npos || s.find("FAILEDIM") != std::string::npos){
        parseDeliveryReport(s);
        return DELIVERTY_REPORT;
    } 
    if (s.find("RECVIM") != std::string::npos){
        parseIncommingIm(s);
        return INSTANT_MESSAGE;
    }
    return NO_ASYNCHRONOUS;
}

int UsblParser::parseInt(uint8_t const* data, size_t size){
    char const* buffer_as_string = reinterpret_cast<char const*>(data);
    std::string s = std::string(buffer_as_string, size);
    std::vector<std::string> splitted = validateResponse(s); 
    return atoi(splitted.at(1).c_str());
}

int UsblParser::parseInt(uint8_t const* data, size_t size, std::string command){
    char const* buffer_as_string = reinterpret_cast<char const*>(data);
    std::string s = std::string(buffer_as_string, size);
    std::vector<std::string> splitted = validateResponse(s); 
    if (splitted.at(0).compare(command) != 0){
        //TODO raise Error
        std::cout << "3raise error; expected command: "<< command << "real command: " << splitted.at(0)  << std::endl;
    }
    return atoi(splitted.at(1).c_str());
}

void UsblParser::parseOk(uint8_t const* data, size_t size){
    char const* buffer_as_string = reinterpret_cast<char const*>(data);
    std::string s = std::string(buffer_as_string, size);
    std::vector<std::string> splitted = validateResponse(s);
    if (splitted.at(1).compare("OK") == 0 || splitted.at(1).compare("*OK") == 0){
        return;
    } else if (s.find("ERROR") != std::string::npos){
        //TODO raise Error
    }
    //TODO raise Error
}

ConnectionStatus UsblParser::parseConnectionStatus(std::string s){
    if (s.compare("OFFLINE")== 0){
        return OFFLINE;
    } else if (s.compare("OFFLINE CONNECTION FAILED")== 0){
        return OFFLINE_CONNECTION_FAILED;
    } else if (s.compare("OFFLINE TERMINATED")== 0){
        return OFFLINE_TERMINATED;
    } else if (s.compare("OFFLINE ALARM")== 0){
        return OFFLINE_ALARM;
    } else if (s.compare("INITIATION LISTEN")== 0){
        return INITIATION_LISTEN;
    } else if (s.compare("INITIATION ESTABLISH")== 0){
        return INITIATION_ESTABLISH;
    } else if (s.compare("INITIATION DISCONNECT")== 0){
        return INITIATION_DISCONNECT;
    } else if (s.compare("ONLINE")== 0){
        return ONLINE;
    } else if (s.compare("BACKOFF")== 0){
        return BACKOFF;
    } else {
        //TODO
        std::cout << "raise error" << std::endl;
    }
    return OFFLINE;
}

Position UsblParser::parsePosition(std::string s){
    std::vector<std::string> splitted = splitValidate(s, ",", 5);
    Position pos;
    pos.time = atoi(splitted.at(0).c_str());
    pos.x = atof(splitted.at(2).c_str());
    pos.y = atof(splitted.at(3).c_str());
    pos.z = atof(splitted.at(4).c_str());
    return pos;
}

std::string UsblParser::parseString(uint8_t const* data, size_t size, std::string command){
    char const* buffer_as_string = reinterpret_cast<char const*>(data);
    std::string s = std::string(buffer_as_string, size);
    std::vector<std::string> splitted = validateResponse(s); 
    if (splitted.at(0).compare(command) != 0){
        //TODO raise Error
        std::cout << "3raise error; expected command: "<< command << "real command: " << splitted.at(0)  << std::endl;
    }
    return splitted.at(1);
}

//Privats
DeliveryStatus UsblParser::parseDeliveryReport(std::string s){
    std::vector<std::string> splitted = splitValidate(s, ",", 2);
    DeliveryStatus temp;
    if (splitted.at(0).find("DELIVEREDIM") != std::string::npos) {
        return DELIVERED;
    } else if (splitted.at(0).find("FAILEDIM") != std::string::npos){
        return FAILED;
    }
    
} 

ReceiveInstantMessage UsblParser::parseIncommingIm(std::string s){
    std::vector<std::string> splitted = splitValidate(s, ",", 11);
    ReceiveInstantMessage im;
    im.len = atoi(splitted.at(1).c_str());
    im.source = atoi(splitted.at(2).c_str());
    im.destination = atoi(splitted.at(3).c_str());
    const char *buffer = splitted.at(10).c_str();
    //TODO copy buffer
    return im; 

}

std::vector<std::string> UsblParser::splitValidate(std::string s, const char* symbol, int parts){
    std::vector<std::string> splitted;
    boost::split( splitted, s, boost::algorithm::is_any_of( symbol ) );
    if (splitted.size() != parts){
        std::cout << "2raise Error"<< std::endl;
    }
    return splitted;
}

std::vector<std::string> UsblParser::validateResponse(std::string s){
    std::vector<std::string> ret;
    if (s.find("+++") == 0){
        std::vector<std::string> splitted = splitValidate(s, ":", 3);
        boost::algorithm::trim(splitted.at(2));
        if (splitted.at(2).length() != atoi(splitted.at(1).c_str())){
            //TODO raise Error
            std::cout<< "raise format error "<< splitted.at(2) << "     " << atoi(splitted.at(1).c_str())  <<std::endl;
            return ret;
        }
        ret.push_back(splitted.at(0).substr(3, splitted.at(0).size()));
        ret.push_back(splitted.at(2));
        return ret;
    } else {
        //TODO raise Error
        std::cout << "1raise Error" << std::endl;
        return ret;
    }
}
