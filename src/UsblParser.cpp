#include "UsblParser.hpp"
#include "Exceptions.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <boost/algorithm/string.hpp>
using namespace usbl_evologics;
UsblParser::UsblParser(){
}

UsblParser::~UsblParser(){
}

Notification UsblParser::findNotification(std::string const &buffer)
{
    if (buffer.find("USBLLONG")!=std::string::npos)
        return USBLLONG;
    else if (buffer.find("USBLANGLES")!=std::string::npos)
        return USBLANGLE;
    else if (buffer.find("DELIVEREDIM")!=std::string::npos ||
            buffer.find("FAILEDIM")!=std::string::npos )
        return DELIVERY_REPORT;
    else if (buffer.find("CANCELEDIM")!=std::string::npos ||
            buffer.find("CANCELEDIMS")!=std::string::npos  ||
            buffer.find("CANCELEDPBM")!=std::string::npos )
        return CANCELED_IM;
    else if (buffer.find("RECVIM")!=std::string::npos)
        return RECVIM;
    else if (buffer.find("RECVIMS")!=std::string::npos)
        return RECVIMS;
    else if (buffer.find("RECVPBM")!=std::string::npos)
        return RECVPBM;
    else if (buffer.find("BITRATE")!=std::string::npos  || buffer.find("SRCLEVEL")!=std::string::npos   ||
            buffer.find("PHYON")!=std::string::npos     || buffer.find("PHYOFF")!=std::string::npos     ||
            buffer.find("RECVSTART")!=std::string::npos || buffer.find("RECVFAILED")!=std::string::npos ||
            buffer.find("RECVEND")!=std::string::npos 	|| buffer.find("SENDSTART")!=std::string::npos  ||
            buffer.find("SENDEND")!=std::string::npos 	|| buffer.find("RADDR")!=std::string::npos		||
            buffer.find("USBLPHYD")!=std::string::npos 	)
        return EXTRA_NOTIFICATION;
    else
        return NO_NOTIFICATION;
}

CommandResponse UsblParser::findResponse(std::string const &buffer)
{
    if (buffer.find("OK")!=std::string::npos)
        return COMMAND_RECEIVED;
    else if (buffer.find("ERROR")!=std::string::npos)
        return ERROR;
    else if (buffer.find("BUSY")!=std::string::npos)
        return BUSY;
    else
        return VALUE_REQUESTED;
}

// In DATA mode: +++AT:<length>:<notification><end-of-line>
// IN COMMAND mode: <notification><end-of-line>
// Use in DATA mode only
void UsblParser::validateNotification(std::string const &buffer)
{
    std::vector<std::string> splitted;
    boost::split( splitted, buffer, boost::algorithm::is_any_of( ":" ) );
    if(splitted.at(0) != "+++AT" )
    {
        std::stringstream error_string;
        error_string << "UsblParser.cpp validateNotification: In DATA mode, could note find \"+++AT\" \" in buffer \""<<buffer<<"\" " << std::flush;
        throw ValidationError(error_string.str());
    }

    std::string::size_type sz;   // alias of size_t
    int length = std::stoi(splitted.at(1),&sz);
    if(length != splitted.at(2).size()-2)
        // <end-of-line> = \r\n; size=2
    {
        std::stringstream error_string;
        error_string << "UsblParser.cpp validateNotification: In DATA mode, the indicated length \""<< length <<"\", doesn't match the size of notification \""<< splitted.at(2).size()-2<<"\", in: "<< buffer<< std::flush;
        throw ValidationError(error_string.str());
    }
}

// In DATA mode: +++AT:<length>:<notification><end-of-line>
// IN COMMAND mode: <notification><end-of-line>
void UsblParser::splitValidateNotification(std::string const &buffer, Notification const &notification)
{
    std::string aux_string;
    // DATA mode
    if (buffer.find("+++AT:") != std::string::npos)
    {	// get <notification> part of string
        // <end-of-line> doesn't affect the analysis
        std::vector<std::string> splitted = splitValidate(buffer, ":", 3);
        aux_string = splitted[2];
    }
    else
        aux_string = buffer;

    // Analysis of number of fields in <notification>
    splitValidate(aux_string, ",", getNumberFields(notification));
}


// In DATA mode: +++<AT command>:<length>:<command response><end-of-line>
// IN COMMAND mode: <response><end-of-line>
// Use in DATA mode only
void UsblParser::validateResponse(std::string const &buffer, std::string const &command)
{
    // Check for the answer of command AT&V (Get Current Set) that isn't like the general case
    if(command.find("AT&V") != std::string::npos)
        validateParticularResponse(buffer);

    else
    {
        // Check for general case
        std::vector<std::string> splitted = splitValidate(buffer, ":", 3);

        // Check if <ATcommand> is in the command string
        // Remove +++ from splitted[0]
        splitted[0].erase(std::remove(splitted[0].begin(), splitted[0].end(), '+'), splitted[0].end());
        // Found <ATcommmad> in the command string
        if(command.find(splitted.at(0)) == std::string::npos )
        {
            std::stringstream error_string;
            error_string << "UsblParser.cpp validateResponse: In DATA mode, could not find command \""<< splitted.at(0) <<"\" in the command buffer \""<< command<<"\" "<< std::flush;
            throw ValidationError(error_string.str());
        }

        // Check the indicated <length> with the <command response>.size()
        std::string::size_type sz;   // alias of size_t
        int length = std::stoi(splitted.at(1),&sz);
        if(length != splitted.at(2).size()-2)
            // <end-of-line> = \n\r; size=2
        {
            std::stringstream error_string;
            error_string << "UsblParser.cpp validateResponse: In DATA mode, the indicated length \""<< length <<"\", doesn't match the size of notification \""<< splitted.at(2).size()-2<<"\", in: \""<< buffer<< "\""<<std::flush;
            throw ValidationError(error_string.str());
        }
    }
}

// Use in DATA mode only, for AT&V command.
// +++AT&V:<length>:<requested data><end-line>
// <requested data> = <field1><: ><value><end-line><field2><: ><value><end-line>...
void UsblParser::validateParticularResponse(std::string const &buffer)
{
    std::string msg = buffer;

    if(msg.find("AT&V") != std::string::npos)
    {
        int npos = std::string::npos;
        if ((npos = msg.find(":")) != std::string::npos)
        {
            //Remove +++<ATcommand>:
            msg = msg.substr(npos+1, msg.size()-npos);
            if ((npos = msg.find(":")) != std::string::npos)
            {
                //Convert <length> to int
                std::string::size_type sz;   // alias of size_t
                int length;
                length = std::stoi(msg.substr(0, npos),&sz);
                //Remove <length>:
                msg = msg.substr(npos+1, msg.size()-npos);
                // <end-of-line> = \n\r; size=2
                if(length != msg.size()-2)
                {
                    std::stringstream error_string;
                    error_string << "UsblParser.cpp validateParticularResponse: In DATA mode, the indicated length \""<< length <<"\", doesn't match the size of notification \""<< msg.size()-2<<"\", in: \""<< buffer<<"\"" << std::flush;
                    throw ValidationError(error_string.str());
                }
            }
            else
            {
                std::stringstream error_string;
                error_string << "UsblParser.cpp validateParticularResponse: In DATA mode, could not find \":\" in \""<< msg <<"\", from buffer \""<< buffer<<"\""<< std::flush;
                throw ValidationError(error_string.str());
            }
        }
    }

}

//Parse Instanta Message to be sent to remote device
std::string UsblParser::parseSendIM(SendIM const &im)
{
    std::stringstream ss;
    ss << "AT*SENDIM," << std::to_string(im.buffer.size())<< "," << std::to_string(im.destination) << ",";
    if(im.deliveryReport)
        ss << "ack,";
    else
        ss << "noack,";
    // convert buffer from std::vector to std::string
    std::string string_buffer(im.buffer.begin(), im.buffer.end());
    ss << string_buffer;

    return ss.str();
}

// Output received Instant Message from remote device
ReceiveIM UsblParser::parseReceivedIM(std::string const &buffer)
{
    ReceiveIM im;
    std::vector<std::string> splitted = splitValidate(buffer, ",", getNumberFields(RECVIM));
    std::string::size_type sz;     // alias of size_t

    im.time	=	base::Time::now();
    im.source	=	std::stoi(splitted[2],&sz);
    im.destination	=	std::stoi(splitted[3],&sz);
    if(splitted[4] == "ack")
        im.deliveryReport	=	true;
    else
        im.deliveryReport	=	false;
    im.duration	=	 base::Time::fromMicroseconds(std::stod(splitted[5],&sz));
    im.rssi	=	std::stoi(splitted[6],&sz);
    im.integrity	=	std::stoi(splitted[7],&sz);
    im.velocity	=	std::stod(splitted[8],&sz);

    std::vector<uint8_t> msg(splitted[9].begin(), splitted[9].end());
    // Remove <end-line> (\r\n) from buffer
    msg.pop_back(); msg.pop_back();
    im.buffer = msg;

    int size = std::stoi(splitted[1],&sz);
    if(size != im.buffer.size())
    {
        std::stringstream error_string;
        error_string << "UsblParser.cpp parseReceivedIM: Tried to split a receiving Instant Message, but the message \""<<splitted[9]<<"\" has \""<< size <<" characters and not \"" << splitted[1] << "\" as predicted. "<< std::flush;
        throw ParseError(error_string.str());
    }

    return im;
}

// Output Position of remote device
Position UsblParser::parsePosition(std::string const &buffer)
{
    Position pose;
    std::vector<std::string> splitted = splitValidate(buffer, ",", getNumberFields(USBLLONG));
    std::string::size_type sz;     // alias of size_t

    pose.time	=	 base::Time::fromSeconds(std::stod(splitted[1],&sz));
    pose.measurementTime	=	base::Time::fromSeconds(std::stod(splitted[2],&sz));
    pose.remoteAddress	=	std::stoi(splitted[3],&sz);
    pose.x	=	std::stod(splitted[4],&sz);
    pose.y	=	std::stod(splitted[5],&sz);
    pose.z	=	std::stod(splitted[6],&sz);
    pose.E	=	std::stod(splitted[7],&sz);
    pose.N	=	std::stod(splitted[8],&sz);
    pose.U	=	std::stod(splitted[9],&sz);
    pose.roll	=	std::stod(splitted[10],&sz);
    pose.pitch	=	std::stod(splitted[11],&sz);
    pose.yaw	=	std::stod(splitted[12],&sz);
    pose.propagationTime	=	base::Time::fromSeconds(std::stod(splitted[13],&sz));
    pose.rssi	=	std::stoi(splitted[14],&sz);
    pose.integrity	=	std::stoi(splitted[14],&sz);
    pose.accuracy	=	std::stoi(splitted[14],&sz);

    return pose;
}

// Defines if a Instant Message has it receipt confirmed
bool UsblParser::parseIMReport(std::string const &buffer)
{
    std::string ret;
    std::vector<std::string> splitted = splitValidate(buffer, ",", getNumberFields(DELIVERY_REPORT));
    if (splitted[0].find("DELIVEREDIM") != std::string::npos)
        return true;
    else if (splitted[0].find("FAILEDIM") != std::string::npos)
        return false;
    else
    {
        std::stringstream error_string;
        error_string << "UsblParser.cpp parseIMReport: DELIVERY_REPORT not as expected: \""<< splitted[0]<< "\"" <<std::flush;
        throw ParseError(error_string.str());
    }
}

// TODO To be implement
std::string UsblParser::parseRequestedValue(std::string const &buffer, std::string const & command)
{
    return "OK";
}

// Return splitted strings by defined character.
std::vector<std::string> UsblParser::splitValidate(std::string const& buffer, const char* symbol, size_t const parts)
{
    std::vector<std::string> splitted;
    //    std::cout << "splitValidate "<< parts << std::endl;
    boost::split( splitted, buffer, boost::algorithm::is_any_of( symbol ) );
    //    std::cout << splitted.size() << std::endl;
    if (splitted.size() != parts)
    {
        std::stringstream error_string;
        error_string << "UsblParser.cpp splitValidate: Tried to split the string \""<<buffer<<"\" at \""<< symbol <<"\" in " << parts << " parts, but get " << splitted.size() << " parts" << std::flush;
        throw ValidationError(error_string.str());
    }
    return splitted;

}

// Return the amount of fields splitted by comma (,) in a notification.
int UsblParser::getNumberFields(Notification const & notification)
{
    switch (notification) {
    case RECVIM:
    case RECVIMS:
        return 10;
        break;
    case RECVPBM:
        return 9;
        break;
    case DELIVERY_REPORT:
    case CANCELED_IM:
        return 2;
        break;
    case USBLLONG:
        return 17;
        break;
    case USBLANGLE:
        return 14;
        break;
    case EXTRA_NOTIFICATION:
        return 1;
        break;
    case NO_NOTIFICATION:
        std::stringstream error_string;
        error_string << "UsblParser.cpp getNumberFields: Received  \""<< notification <<"\"" << std::flush;
        throw ValidationError(error_string.str());
        break;
    }
    std::stringstream error_string;
    error_string << "UsblParser.cpp getNumberFields: \""<< notification <<"\" is not determined" << std::flush;
    throw ValidationError(error_string.str());

}


