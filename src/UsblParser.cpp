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


//int UsblParser::isPacket(std::string const &buffer, OperationMode const &mode)
//{
//	int ret;
//	if(mode == DATA)
//	{
//		if(buffer.size() == 1 && buffer.find("+") != std::string::npos)
//			return 0;
//		else if(buffer.size() == 2 && buffer.find("++") != std::string::npos)
//			return 0;
//		else if(buffer.find("+++") != std::string::npos)
//		{
//
//		}
//	}
//
//	return ret;
//}


bool UsblParser::findNotification(std::string const &buffer, Notification &notification)
{
	if (buffer.find("USBLLONG")!=std::string::npos)
	{
		notification = USBLLONG;
		return true;
	}
	else if (buffer.find("USBLANGLES")!=std::string::npos)
	{
		notification = USBLANGLE;
		return true;
	}
	else if (buffer.find("DELIVEREDIM")!=std::string::npos ||
			buffer.find("FAILEDIM")!=std::string::npos )
	{
		notification = DELIVERY_REPORT;
		return true;
	}
	else if (buffer.find("CANCELEDIM")!=std::string::npos ||
	        buffer.find("CANCELEDIMS")!=std::string::npos  ||
	        buffer.find("CANCELEDPBM")!=std::string::npos )
	{
        notification = CANCELED_IM;
        return true;
	}
	else if (buffer.find("RECVIM")!=std::string::npos)
	{
		notification = RECVIM;
		return true;
	}
	else if (buffer.find("RECVIMS")!=std::string::npos)
	{
		notification = RECVIMS;
		return true;
	}
	else if (buffer.find("RECVPBM")!=std::string::npos)
	{
		notification = RECVPBM;
		return true;
	}
	else if (buffer.find("BITRATE")!=std::string::npos 	||
			buffer.find("SRCLEVEL")!=std::string::npos 	||
			buffer.find("PHYON")!=std::string::npos 	||
			buffer.find("PHYOFF")!=std::string::npos 	||
			buffer.find("RECVSTART")!=std::string::npos ||
			buffer.find("RECVFAILED")!=std::string::npos ||
			buffer.find("RECVEND")!=std::string::npos 	||
			buffer.find("SENDSTART")!=std::string::npos ||
			buffer.find("SENDEND")!=std::string::npos 	||
			buffer.find("RADDR")!=std::string::npos		||
			buffer.find("USBLPHYD")!=std::string::npos 	)
	{
		notification = EXTRA_NOTIFICATION;
		return true;
	}
	else
		return false;
}

bool UsblParser::findResponse(std::string const &buffer, CommandResponse &response)
{
	if (buffer.find("OK")!=std::string::npos)
	{
		response = COMMAND_RECEIVED;
		return true;
	}
	else if (buffer.find("ERROR")!=std::string::npos)
	{
		response = ERROR;
		return true;
	}
	else if (buffer.find("BUSY")!=std::string::npos)
	{
		response = BUSY;
		return true;
	}
	else
	{
		response = VALUE_REQUESTED;
		return true;
	}
}

// In DATA mode: +++AT:<length>:<notification><end-of-line>
// IN COMMAND mode: <notification><end-of-line>
// Use in DATA mode only
bool UsblParser::validateNotification(std::string const &buffer)
{
	bool ret = false;
    std::vector<std::string> splitted;
    boost::split( splitted, buffer, boost::algorithm::is_any_of( ":" ) );
    if(splitted.at(0) == "+++AT" )
    	ret = true;
    if(ret)
    {
    	  std::string::size_type sz;   // alias of size_t
    	  int length = std::stoi(splitted.at(1),&sz);
    	  if(length != splitted.at(2).size()-2)
    		  // <end-of-line> = \d\a; size=2
    		  ret = false;
    }
    return ret;
}

// In DATA mode: +++AT:<length>:<notification><end-of-line>
// IN COMMAND mode: <notification><end-of-line>
// Validate <notification>, by the number of known fields, independently if in COMMAND or DATA mode
bool UsblParser::splitValidateNotification(std::string const &buffer, Notification const &notification, std::string &output_msg)
{
	bool ret = false;
	std::vector<std::string> parts;
	std::string aux_string;
	// DATA mode
	if (buffer.find(":") != std::string::npos)
	{	// get <notification> part of string
		// <end-of-line> doesn't affect the analysis
		if(splitValidate(buffer, ":", 3, parts))
				aux_string = parts[2];
		else
		{
			output_msg = parts[0];
			return false;
		}
	}
	else
		aux_string = buffer;
	// Analysis of number of fields in <notification>
	ret = splitValidate(aux_string, ",", getNumberFields(notification), parts);
	if(!ret)
		output_msg = parts[0];
	return ret;
}


// In DATA mode: +++<AT command>:<length>:<command response><end-of-line>
// IN COMMAND mode: <response><end-of-line>
// Use in DATA mode only
bool UsblParser::validateResponse(std::string const &buffer, std::string const &command)
{
	bool ret = false;

	// Check for the answer of command AT&V, Get Current Set, that isn't like the general case
	if(command.find("AT&V") != std::string::npos)
	    return validateParticularResponse(buffer);

    std::vector<std::string> splitted;
    ret = splitValidate(buffer, ":", 3,splitted);
    // Check if <ATcommand> is in the command string
    // Remove +++ from splitted[0]
    splitted[0].erase(std::remove(splitted[0].begin(), splitted[0].end(), '+'), splitted[0].end());
    if(ret)
	{   // Found <ATcommmad> in the command string
    	if(command.find(splitted.at(0)) == std::string::npos )
    		ret = false;
	}
    if(ret)
    {   // Check the indicated <length> with the <command response>.size()
    	  std::string::size_type sz;   // alias of size_t
    	  int length = std::stoi(splitted.at(1),&sz);
    	  if(length != splitted.at(2).size()-2)
    	      // <end-of-line> = \n\r; size=2
    		  ret = false;
    }
    return ret;
}

// Use in DATA mode only, for AT&V command.
// +++AT&V:<length>:<requested data><end-line>
// <requested data> = <field1><: ><value><end-line><field2><: ><value><end-line>...
bool UsblParser::validateParticularResponse(std::string const &buffer)
{
    bool ret = false;
    std::string msg = buffer;
    std::cout <<"valiParticular: test0"<<std::endl;

    if(msg.find("AT&V") != std::string::npos)
    {
        std::cout <<"valiParticular: test1"<<std::endl;
        int npos = std::string::npos;
        if ((npos = msg.find(":")) != std::string::npos)
        {
            std::cout <<"valiParticular: test2"<<std::endl;
            //Remove +++<ATcommand>:
            msg = msg.substr(npos+1, msg.size()-npos);
            if ((npos = msg.find(":")) != std::string::npos)
            {
                std::cout <<"valiParticular: test3"<<std::endl;
                //Convert <length> to int
                std::string::size_type sz;   // alias of size_t
                int length;
                length = std::stoi(msg.substr(0, npos),&sz);
                //Remove <length>:
                msg = msg.substr(npos+1, msg.size()-npos);
                // <end-of-line> = \n\r; size=2
                if(length == msg.size()-2)
                {
                    std::cout <<"valiParticular: test4"<<std::endl;
                    ret = true;
                }
            }
        }
    }
    return ret;
}


std::string UsblParser::parseSendIM(SendedIM const &im)
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

	// convert whole command string
	string_buffer = ss.str();
	return string_buffer;
}

std::string UsblParser::parseReceivedIM(std::string const &buffer, ReceivedIM &im)
{
	std::string ret;
	std::vector<std::string> fields;
	std::string::size_type sz;     // alias of size_t
	if (splitValidate(buffer, ",", getNumberFields(RECVIM), fields))
	{
		im.time	=	base::Time::now();
		im.source	=	std::stoi(fields[2],&sz);
		im.destination	=	std::stoi(fields[3],&sz);
		if(fields[4] == "ack")
			im.deliveryReport	=	true;
		else
			im.deliveryReport	=	false;
		im.duration	=	 base::Time::fromMicroseconds(std::stod(fields[5],&sz));
		im.rssi	=	std::stoi(fields[6],&sz);
		im.integrity	=	std::stoi(fields[7],&sz);
		im.velocity	=	std::stod(fields[8],&sz);

		std::vector<uint8_t> aux_buffer(fields[9].begin(), fields[9].end());
		// Remove <end-line> (\r\n) from buffer
		aux_buffer.pop_back(); aux_buffer.pop_back();
		im.buffer	=	aux_buffer;

		int size = std::stoi(fields[1],&sz);
		if(size == im.buffer.size())
			ret = "NEW IM";
		else
			ret = "Wrong size of buffer";
	}
	else
		ret = fields[0];

	return ret;
}

std::string UsblParser::parsePosition(std::string const &buffer, Position &pose)
{
	std::string ret;
	std::vector<std::string> fields;
	std::string::size_type sz;     // alias of size_t
	if (splitValidate(buffer, ",", getNumberFields(USBLLONG), fields))
	{
		pose.time	=	 base::Time::fromSeconds(std::stod(fields[1],&sz));
		pose.measurementTime	=	base::Time::fromSeconds(std::stod(fields[2],&sz));
		pose.remoteAddress	=	std::stoi(fields[3],&sz);
		pose.x	=	std::stod(fields[4],&sz);
		pose.y	=	std::stod(fields[5],&sz);
		pose.z	=	std::stod(fields[6],&sz);
		pose.E	=	std::stod(fields[7],&sz);
		pose.N	=	std::stod(fields[8],&sz);
		pose.U	=	std::stod(fields[9],&sz);
		pose.roll	=	std::stod(fields[10],&sz);
		pose.pitch	=	std::stod(fields[11],&sz);
		pose.yaw	=	std::stod(fields[12],&sz);
		pose.propagationTime	=	base::Time::fromSeconds(std::stod(fields[13],&sz));
		pose.rssi	=	std::stoi(fields[14],&sz);
		pose.integrity	=	std::stoi(fields[14],&sz);
		pose.accuracy	=	std::stoi(fields[14],&sz);
		ret = "NEW POSE";
	}
	else
		ret = fields[0];

	return ret;
}
std::string UsblParser::parseIMReport(std::string const &buffer)
{
	std::string ret;
	std::vector<std::string> fields;
	std::string::size_type sz;     // alias of size_t
	if (splitValidate(buffer, ",", getNumberFields(DELIVERY_REPORT), fields))
	{
		ret = fields[0];
	}
	else
		ret = fields[0];

	return ret;
}

std::string UsblParser::parseRequestedValue(std::string const &buffer, std::string const & command)
{
	return "OK";
}

bool UsblParser::splitValidate(std::string const s, const char* symbol, size_t const parts, std::vector<std::string> &splitted)
{
    std::vector<std::string> aux_splitted;
//    std::cout << "splitValidate "<< parts << std::endl;
    boost::split( aux_splitted, s, boost::algorithm::is_any_of( symbol ) );
//    std::cout << aux_splitted.size() << std::endl;
    if (aux_splitted.size() != parts)
    {
        std::stringstream error_string;
        error_string << "UsblParser.cpp splitValidate: Tried to split the string \""<<s<<"\" at \""<< symbol <<" in " << parts << " parts, but get " << splitted.size() << " parts" << std::flush;
        aux_splitted[0] = error_string.str();
        return false;
    }
    splitted = aux_splitted;
    return true;
}

int UsblParser::getNumberFields(Notification const & notification)
{
	int ret = 0;
	switch (notification) {
		case RECVIM:
		case RECVIMS:
			ret = 10;
		break;
		case RECVPBM:
			ret = 9;
		break;
		case DELIVERY_REPORT:
		case CANCELED_IM:
			ret = 2;
		break;
		case USBLLONG:
			ret = 17;
		break;
		case USBLANGLE:
			ret = 14;
		break;
		case EXTRA_NOTIFICATION:
			ret = 1;
		break;
	}
	return ret;
}


