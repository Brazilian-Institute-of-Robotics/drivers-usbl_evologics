#include "UsblParser.hpp"
#include "Exceptions.hpp"
#include <iostream>
#include <sstream>
#include <boost/algorithm/string.hpp>
using namespace usbl_evologics;
UsblParser::UsblParser(){
}

int UsblParser::isPacket(std::string const s){
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
                try{
                    validateResponse(s.substr(0, eol+1));
                } catch (ValidationError& error){
                    //It's not a valid command, so just data
                    return (eol+1)*-1;
                }
                return eol+1;
            }
        }
        std::vector<std::string> splitted;
        boost::split( splitted, s, boost::algorithm::is_any_of(":") );
        //Check the first part
        if (splitted.size() >= 1){
            if (!splitted.at(0).compare("+++") == 0){
                //The first part is more then +++, it's can't be a command
                return s.size()*-1;
            }
        }
        size_t len;
        //Check the second part
        if (splitted.size() >= 2){
            std::stringstream ss(splitted.at(1));
            if (!(ss >> len)){
                //the second part isn't exact a int
                return s.size()*-1;
            }
        }
        //Check the third part
        if (splitted.size() >= 3){
            if (len < splitted.at(2).length()){
                //Last part is too long it's can't be a valid command
                return s.size()*-1;
            }
        }
        //It's can be a command waiting for more data
        return 0;
    } 
}

AsynchronousMessages UsblParser::parseAsynchronousCommand(std::string const s){
    if (s.find("DELIVEREDIM") != std::string::npos || s.find("FAILEDIM") != std::string::npos){
        parseDeliveryReport(s);
        return DELIVERY_REPORT;
    } 
    if (s.find("RECVIM") != std::string::npos){
        parseIncomingIm(s);
        return INSTANT_MESSAGE;
    }
    return NO_ASYNCHRONOUS;
}

int UsblParser::parseInt(uint8_t const* data, size_t const size){
    char const* buffer_as_string = reinterpret_cast<char const*>(data);
    std::string s = std::string(buffer_as_string, size);
    std::vector<std::string> splitted = validateResponse(s); 
    return getInt(splitted.at(1)); 
}

int UsblParser::parseInt(uint8_t const* data, size_t const size, std::string const command){
    char const* buffer_as_string = reinterpret_cast<char const*>(data);
    std::string s = std::string(buffer_as_string, size);
    std::vector<std::string> splitted = validateResponse(s); 
    if (splitted.at(0).compare(command) != 0){
        std::stringstream error_string;
        error_string << "Expected a response to "<< command << " but read " << splitted.at(0) << std::flush;
        throw ParseError(error_string.str());
    }
    return getInt(splitted.at(1));
}
int UsblParser::getInt(std::string const s){
    int value;
    std::stringstream ss(s);
    if (!(ss >> value)){
        std::stringstream error_string;
        error_string << "Expected an integer response, but read " << s << std::flush;
        throw ParseError(error_string.str());
    }
    return value;
}

void UsblParser::parseOk(uint8_t const* data, size_t const size){
    char const* buffer_as_string = reinterpret_cast<char const*>(data);
    std::string s = std::string(buffer_as_string, size);
    std::vector<std::string> splitted = validateResponse(s);
    if (splitted.at(1).compare("OK") == 0 || splitted.at(1).compare("*OK") == 0){
        return;
    } else if (s.find("ERROR") != std::string::npos){
        throw DeviceError(s);
    }
    std::stringstream error_string;
    error_string << "Waiting for a OK, but read " << s << std::flush;
    throw ParseError(error_string.str());
}

ConnectionStatus UsblParser::parseConnectionStatus(std::string const s){
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
        std::stringstream error_string;
        error_string << "Waiting for Connection Status but read " << s<< std::flush;
        throw ParseError(error_string.str());
    }
    return OFFLINE;
}

Position UsblParser::parsePosition(std::string const s){
    std::vector<std::string> splitted = splitValidate(s, ",", 5);
    Position pos;
    pos.time = atoi(splitted.at(0).c_str());
    pos.x = atof(splitted.at(2).c_str());
    pos.y = atof(splitted.at(3).c_str());
    pos.z = atof(splitted.at(4).c_str());
    return pos;
}

std::string UsblParser::parseString(uint8_t const* data, size_t const size, std::string const command){
    char const* buffer_as_string = reinterpret_cast<char const*>(data);
    std::string s = std::string(buffer_as_string, size);
    std::vector<std::string> splitted = validateResponse(s); 
    if (splitted.at(0).compare(command) != 0){
        std::stringstream error_string;
        error_string << "Waiting for a response to " << command << " but read " << splitted.at(0)<< std::flush;
        throw ParseError(error_string.str());
    }
    return splitted.at(1);
}

//Privats
DeliveryStatus UsblParser::parseDeliveryReport(std::string const s){
    std::vector<std::string> splitted = splitValidate(s, ",", 2);
    if (splitted.at(0).find("DELIVEREDIM") != std::string::npos) {
        return DELIVERED;
    } else if (splitted.at(0).find("FAILEDIM") != std::string::npos){
        return FAILED;
    }
    return FAILED;
    
} 

ReceiveInstantMessage UsblParser::parseIncomingIm(std::string const s){
    std::vector<std::string> splitted = splitValidate(s, ",", 11);
    ReceiveInstantMessage im;
    im.len = atoi(splitted.at(1).c_str());
    im.source = atoi(splitted.at(2).c_str());
    im.destination = atoi(splitted.at(3).c_str());
    const char *buffer = splitted.at(10).c_str();

    std::string buffer_as_string = std::string(reinterpret_cast<char const*>(buffer));
    buffer_as_string = buffer_as_string.substr(0, im.len);
    for (size_t i = 0; i < im.len; i++){
        im.buffer[i] = (uint8_t) buffer[i];
    }
    buffer_as_string = std::string(reinterpret_cast<char const*>(im.buffer));
    buffer_as_string = buffer_as_string.substr(0, im.len);

    return im; 

}

std::vector<std::string> UsblParser::splitValidate(std::string const s, const char* symbol, size_t const parts){
    std::vector<std::string> splitted;
    boost::split( splitted, s, boost::algorithm::is_any_of( symbol ) );
    if (splitted.size() != parts){
        std::stringstream error_string;
        error_string << "Tried to split the string \""<<s<<"\" at \""<< symbol <<" in " << parts << " parts, but get " << splitted.size() << " parts" << std::flush;
        throw ValidationError(error_string.str());
    }
    return splitted;
}

std::vector<std::string> UsblParser::validateResponse(std::string const s){
    std::vector<std::string> ret;
    if (s.find("+++") == 0){
        std::vector<std::string> splitted = splitValidate(s, ":", 3);
        boost::algorithm::trim(splitted.at(2));
        if (splitted.at(2).length() != getInt(splitted.at(1))){
            throw ValidationError("Length of the data part is incorrect");
        }
        ret.push_back(splitted.at(0).substr(3, splitted.at(0).size()));
        ret.push_back(splitted.at(2));
        return ret;
    } else {
        throw ValidationError("No Escape Sequence");
    }
}
