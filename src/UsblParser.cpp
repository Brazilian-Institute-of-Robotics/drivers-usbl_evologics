#include "UsblParser.hpp"
#include "Exceptions.hpp"
#include <iostream>
#include <sstream>
#include <boost/algorithm/string.hpp>
using namespace usbl_evologics;
UsblParser::UsblParser(){
}

int UsblParser::isPacket(std::string const s){
//    std::cout << "Is Packet " << s << std::endl;
    std::size_t escape_sequenz_position;
    switch (s.length()) {
        case 0:
            return 0;
        case 1:
            escape_sequenz_position = s.find("+");
            if (escape_sequenz_position != std::string::npos){
                return 0;
            }
            else{
                return -1;
            }
        case 2:
            escape_sequenz_position = s.find("++");
            if (escape_sequenz_position != std::string::npos){
                return 0;
            }
            else{
                return -2;
            }
    }
    escape_sequenz_position = s.find("+++");
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
                    std::cout << "Validation Error!!" << std::endl;
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
            if (!splitted.at(0).find("+++") == 0){
                //The first part is more then +++, it's can't be a command
                return s.size()*-1;
            }
        }
        size_t len;
        //Check the second part
        if (splitted.size() >= 2){
            if (splitted.at(1).length() == 0){
                return 0;
            }
            std::stringstream ss(splitted.at(1));
            if (!(ss >> len)){
                //the second part isn't exact a int
                return s.size()*-1;
            }
        }
        //Check the third part
        //TODO this check doesn't work correct with ":" in the data part.
        //It's important to check the size of every part of the splitted vector. (with a index 2 or bigger)
        if (splitted.size() >= 3){
            boost::algorithm::trim(splitted.at(2));
            if (len < splitted.at(2).length()){
                //Data Part is too long it's can't be a valid command
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
    if (s.find("CANCELEDIM") != std::string::npos){
        return CANCELEDIM;
    }
    if (s.find("USBLLONG") != std::string::npos){
        return USBLLONG;
    }
    if (s.find("USBLANGLE") != std::string::npos){
        return USBLLONG;
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
        error_string << "Expected a response to "<< command << " but read " << command << std::flush;
        throw ParseError(error_string.str());
    }
    return getInt(splitted.at(1));
}
int UsblParser::getInt(std::string const s){
    int value;
    std::string s_tmp = s;
    boost::algorithm::trim_if(s_tmp, boost::is_any_of("[*]"));
    std::stringstream ss(s_tmp);
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
    if (splitted.at(1).compare("OK") == 0 || splitted.at(1).compare("[*]OK") == 0){
        return;
    } else if (s.find("ERROR") != std::string::npos){
        throw DeviceError(s);
    }
    std::stringstream error_string;
    error_string << "Waiting for a OK, but read " << s << std::flush;
    throw ParseError(error_string.str());
}

ConnectionStatus UsblParser::parseConnectionStatus(std::string const s){
    if (s.find("OFFLINE")!= std::string::npos){
        if (s.find("OFFLINE CONNECTION FAILED") != std::string::npos){
            return OFFLINE_CONNECTION_FAILED;
        }
        else if (s.find("OFFLINE TERMINATED") != std::string::npos){
            return OFFLINE_TERMINATED;
        }
        else if (s.find("OFFLINE ALARM") != std::string::npos){
            return OFFLINE_ALARM;
        }
        else {
            return OFFLINE;
        }
    } else if (s.find("INITIATION") != std::string::npos){
        if (s.find("INITIATION LISTEN") != std::string::npos){
            return INITIATION_LISTEN;
        }
        else if (s.find("INITIATION ESTABLISH") != std::string::npos){
            return INITIATION_ESTABLISH;
        }
        else if (s.find("INITIATION DISCONNECT") != std::string::npos){
            return INITIATION_DISCONNECT;
        }
    } else if (s.find("ONLINE")!= std::string::npos){
        return ONLINE;
    } else if (s.find("BACKOFF")!= std::string::npos){
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
    //im.len = atoi(splitted.at(1).c_str());
    im.buffer.resize(atoi(splitted.at(1).c_str()));
    im.source = atoi(splitted.at(2).c_str());
    im.destination = atoi(splitted.at(3).c_str());
    const char *buffer = splitted.at(10).c_str();

    std::string buffer_as_string = std::string(reinterpret_cast<char const*>(buffer));
    buffer_as_string = buffer_as_string.substr(0, im.buffer.size());
    for (size_t i = 0; i < im.buffer.size(); i++){
        im.buffer[i] = (uint8_t) buffer[i];
    }

    //buffer_as_string = std::string(reinterpret_cast<char const*>(im.buffer));
    //buffer_as_string = buffer_as_string.substr(0, im.len);

    buffer_as_string = std::string(im.buffer.begin(), im.buffer.end());
    //buffer_as_string = buffer_as_string.substr(0, im.buf);
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
        //old: Now there are ":" in Messages there can be more than three parts by splitting by ":"
        //std::vector<std::string> splitted = splitValidate(s, ":", 3);
        std::vector<std::string> splitted;
        boost::split( splitted, s, boost::algorithm::is_any_of( ":" ) );
        if (splitted.size() < 3){
            std::stringstream error_string;
            error_string << "Tried to split the string \""<<s<<"\" at \""<< ":" <<" in " << "3" << " parts or more, but get " << splitted.size() << " parts" << std::flush;
            std::cout << error_string << std::endl;
            throw ValidationError(error_string.str());
        } else if (splitted.size() > 3) {
            //This is the special case that the char ":" is in the data
            for (int i=3; i<splitted.size(); i++){
                splitted[2] = splitted[2] + ":" +  splitted[i];
            }
            splitted.resize(3);
        }
        //After this block the size of splitted is exactly 3
        boost::algorithm::trim(splitted.at(2));
        if ((int)splitted.at(2).length() != getInt(splitted.at(1))){
            std::stringstream error_string;
            error_string << "Length of the data part is incorrect." << std::endl <<
                "    The Data Part of the Message is: \"" << splitted.at(2) << "\" and has the length " << (int) splitted.at(2).length() << std::endl <<
                "    The expected length is " << getInt(splitted.at(1)) << std::endl;
            throw ValidationError(error_string.str());
        }
        ret.push_back(splitted.at(0).substr(3, splitted.at(0).size()));
        ret.push_back(splitted.at(2));
        return ret;
    } else {
        throw ValidationError("No Escape Sequence");
    }
}

std::string UsblParser::parsePhyNumber(std::string const s){
    std::string phy = splitValidate(s, ",", 2)[0];
    phy = splitValidate(phy, ":", 2)[1];
    return phy;
}

std::string UsblParser::parseMacNumber(std::string const s){
    std::string mac = splitValidate(s, ",", 2)[1];
    //TODO validate the part
    mac = splitValidate(mac, ":", 2)[1];
    return mac;
}
