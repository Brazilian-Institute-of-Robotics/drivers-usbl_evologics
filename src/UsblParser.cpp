#include "UsblParser.hpp"
#include "Exceptions.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace usbl_evologics;

UsblParser::UsblParser(){
}

UsblParser::~UsblParser(){
}

// Find a Notification in a buffer.
Notification UsblParser::findNotification(string const &buffer)
{
    if (buffer.find("USBLLONG")!=string::npos)
        return USBLLONG;
    else if (buffer.find("USBLANGLES")!=string::npos)
        return USBLANGLE;
    else if (buffer.find("DELIVEREDIM")!=string::npos ||
            buffer.find("FAILEDIM")!=string::npos )
        return DELIVERY_REPORT;
    else if (buffer.find("CANCELEDIM")!=string::npos ||
            buffer.find("CANCELEDIMS")!=string::npos  ||
            buffer.find("CANCELEDPBM")!=string::npos )
        return CANCELED_IM;
    else if (buffer.find("RECVIM")!=string::npos)
        return RECVIM;
    else if (buffer.find("RECVIMS")!=string::npos)
        return RECVIMS;
    else if (buffer.find("RECVPBM")!=string::npos)
        return RECVPBM;
    else if (buffer.find("BITRATE")!=string::npos  || buffer.find("SRCLEVEL")!=string::npos   ||
            buffer.find("PHYON")!=string::npos     || buffer.find("PHYOFF")!=string::npos     ||
            buffer.find("RECVSTART")!=string::npos || buffer.find("RECVFAILED")!=string::npos ||
            buffer.find("RECVEND")!=string::npos 	|| buffer.find("SENDSTART")!=string::npos  ||
            buffer.find("SENDEND")!=string::npos 	|| buffer.find("RADDR")!=string::npos		||
            buffer.find("USBLPHYD")!=string::npos 	)
        return EXTRA_NOTIFICATION;
    else
        return NO_NOTIFICATION;
}

// Check for a Response in buffer.
CommandResponse UsblParser::findResponse(string const &buffer)
{
    if (buffer.find("OK")!=string::npos)
        return COMMAND_RECEIVED;
    else if (buffer.find("ERROR")!=string::npos)
        return ERROR;
    else if (buffer.find("BUSY")!=string::npos)
        return BUSY;
    else
        return VALUE_REQUESTED;
}

// Validate a Notification buffer in DATA mode.
void UsblParser::validateNotification(string const &buffer)
{
    vector<string> splitted;
    boost::split( splitted, buffer, boost::algorithm::is_any_of( ":" ) );
    if(splitted.at(0) != "+++AT" )
        throw ValidationError("UsblParser.cpp validateNotification: In DATA mode, could note find \"+++AT\" \" in buffer \"" + buffer + "\" ");

    string::size_type length = stoi(splitted.at(1), &length);
    if(length != splitted.at(2).size()-2)
        // <end-of-line> = \r\n; size=2
        throw ValidationError("UsblParser.cpp validateNotification: In DATA mode, the indicated length \"" + to_string(length) + "\", doesn't match the size of notification \""+ to_string(splitted.at(2).size()-2) +"\", in: \"" + buffer + "\"");
}

// Validate the number of field of a Notification.
void UsblParser::splitValidateNotification(string const &buffer, Notification const &notification)
{
    string aux_string;
    // DATA mode
    if (buffer.find("+++AT:") != string::npos)
    {	// get <notification> part of string
        // <end-of-line> doesn't affect the analysis
        vector<string> splitted = splitValidate(buffer, ":", 3);
        aux_string = splitted[2];
    }
    else
        aux_string = buffer;

    // Analysis of number of fields in <notification>
    splitValidate(aux_string, ",", getNumberFields(notification));
}

// Validate a Response buffer in DATA mode.
void UsblParser::validateResponse(string const &buffer, string const &command)
{
    // Check for the answer of command AT&V (Get Current Set) that isn't like the general case
    if(command.find("AT&V") != string::npos)
        validateParticularResponse(buffer);

    else
    {
        // Check for general case
        vector<string> splitted = splitValidate(buffer, ":", 3);

        // Check if <ATcommand> is in the command string
        // Remove +++ from splitted[0]
        splitted[0].erase(remove(splitted[0].begin(), splitted[0].end(), '+'), splitted[0].end());
        // Found <ATcommmad> in the command string
        if(command.find(splitted.at(0)) == string::npos )
            throw ValidationError("UsblParser.cpp validateResponse: In DATA mode, could not find command \"" + splitted.at(0) + "\" in the command buffer \"" + command + "\" ");

        // Check the indicated <length> with the <command response>.size()
        string::size_type length = stoi(splitted.at(1), &length);
        if(length != splitted.at(2).size()-2)
            // <end-of-line> = \n\r; size=2
            throw ValidationError("UsblParser.cpp validateResponse: In DATA mode, the indicated length \"" + to_string(length) + "\", doesn't match the size of notification \"" + to_string(splitted.at(2).size()-2) + "\", in: \"" + buffer + "\"");
    }
}

// Validate a Particular Response in DATA mode.
void UsblParser::validateParticularResponse(string const &buffer)
{
    string msg = buffer;

    if(msg.find("AT&V") != string::npos)
    {
        string::size_type npos = string::npos;
        if ((npos = msg.find(":")) != string::npos)
        {
            //Remove +++<ATcommand>:
            msg = msg.substr(npos+1, msg.size()-npos);
            if ((npos = msg.find(":")) != string::npos)
            {
                //Convert <length> to int
                string::size_type length;
                length = stoi(msg.substr(0, npos),&npos);
                //Remove <length>:
                msg = msg.substr(npos+1, msg.size()-npos);
                // <end-of-line> = \n\r; size=2
                if(length != msg.size()-2)
                    throw ValidationError("UsblParser.cpp validateParticularResponse: In DATA mode, the indicated length \"" + to_string(length) + "\", doesn't match the size of notification \"" + to_string(msg.size()-2) + "\", in: \"" + buffer + "\"");
            }
            else
                throw ValidationError("UsblParser.cpp validateParticularResponse: In DATA mode, could not find \":\" in \"" + msg + "\", from buffer \"" + buffer +"\"");
        }
    }
}

// Get response or notification content in DATA mode.
string UsblParser::getAnswerContent(string const &buffer)
{
    string msg = buffer;
    string::size_type npos = string::npos;
    if(msg.find("+++AT") == string::npos)
        throw ModeError("USBL UsblParser.cpp getAnswerContent: Function only can be called in DATA mode, not in COMMAND mode. Problem with answer: \""+ buffer +"\"");
    if ((npos = msg.find(":")) != string::npos)
    {
        //Remove +++<AT >:
        msg = msg.substr(npos+1, msg.size()-npos);
        if ((npos = msg.find(":")) != string::npos)
        {
            //Remove <length>:
            msg = msg.substr(npos+1, msg.size()-npos);
            // <end-of-line> = \n\r; size=2
            return msg;
        }
        else
            throw ValidationError("UsblParser.cpp getAnswerContent: In DATA mode, could not find \":\" in \"" + msg + "\", from buffer \"" + buffer +"\"");
    }
    return msg;
}

// Parse a Instant Message into string to be sent to device.
string UsblParser::parseSendIM(SendIM const &im)
{
    stringstream ss;
    ss << "AT*SENDIM," << to_string(im.buffer.size())<< "," << to_string(im.destination) << ",";
    if(im.deliveryReport)
        ss << "ack,";
    else
        ss << "noack,";
    // convert buffer from vector to string
    string string_buffer(im.buffer.begin(), im.buffer.end());
    ss << string_buffer;

    return ss.str();
}

// Parse a received Instant Message from buffer to ReceiveIM.
ReceiveIM UsblParser::parseReceivedIM(string const &buffer)
{
    ReceiveIM im;
    vector<string> splitted = splitValidate(buffer, ",", getNumberFields(RECVIM));
    string::size_type sz;     // alias of size_t

    im.time = base::Time::now();
    im.source = stoi(splitted[2],&sz);
    im.destination = stoi(splitted[3],&sz);
    if(splitted[4] == "ack")
        im.deliveryReport = true;
    else
        im.deliveryReport = false;
    im.duration = base::Time::fromMicroseconds(stod(splitted[5],&sz));
    im.rssi = stoi(splitted[6],&sz);
    im.integrity = stoi(splitted[7],&sz);
    im.velocity = stod(splitted[8],&sz);

    vector<uint8_t> msg(splitted[9].begin(), splitted[9].end());
    // Remove <end-line> (\r\n) from buffer
    msg.pop_back(); msg.pop_back();
    im.buffer = msg;

    string::size_type size = stoi(splitted[1],&sz);
    if(size != im.buffer.size())
        throw ParseError("UsblParser.cpp parseReceivedIM: Tried to split a receiving Instant Message, but the message \"" + splitted[9] +"\" has \"" + to_string(size) + " characters and not \"" + splitted[1] + "\" as predicted.");
    return im;
}

// Parse a received pose from buffer to Position.
Position UsblParser::parsePosition(string const &buffer)
{
    Position pose;
    vector<string> splitted = splitValidate(buffer, ",", getNumberFields(USBLLONG));
    string::size_type sz;     // alias of size_t

    pose.time = base::Time::fromSeconds(stod(splitted[1],&sz));
    pose.measurementTime = base::Time::fromSeconds(stod(splitted[2],&sz));
    pose.remoteAddress = stoi(splitted[3],&sz);
    pose.x = stod(splitted[4],&sz);
    pose.y = stod(splitted[5],&sz);
    pose.z = stod(splitted[6],&sz);
    pose.E = stod(splitted[7],&sz);
    pose.N = stod(splitted[8],&sz);
    pose.U = stod(splitted[9],&sz);
    pose.roll = stod(splitted[10],&sz);
    pose.pitch = stod(splitted[11],&sz);
    pose.yaw = stod(splitted[12],&sz);
    pose.propagationTime = base::Time::fromSeconds(stod(splitted[13],&sz));
    pose.rssi = stoi(splitted[14],&sz);
    pose.integrity = stoi(splitted[14],&sz);
    pose.accuracy = stoi(splitted[14],&sz);

    return pose;
}

// Check if Instant Message was delivered.
bool UsblParser::parseIMReport(string const &buffer)
{
    string ret;
    vector<string> splitted = splitValidate(buffer, ",", getNumberFields(DELIVERY_REPORT));
    if (splitted[0].find("DELIVEREDIM") != string::npos)
        return true;
    else if (splitted[0].find("FAILEDIM") != string::npos)
        return false;
    else
        throw ParseError("UsblParser.cpp parseIMReport: DELIVERY_REPORT not as expected: \""+ splitted[0] + "\"");
}

// Check if buffer can be splitted in an establish amount.
vector<string> UsblParser::splitValidate(string const& buffer, const char* symbol, size_t const parts)
{
    vector<string> splitted;
    //    cout << "splitValidate "<< parts << endl;
    boost::split( splitted, buffer, boost::algorithm::is_any_of( symbol ) );
    //    cout << splitted.size() << endl;
    if (splitted.size() != parts)
        throw ValidationError("UsblParser.cpp splitValidate: Tried to split the string \"" + buffer + "\" at \"" + symbol + "\" in " + to_string(parts) + " parts, but get " + to_string(splitted.size()) + " parts");
    return splitted;

}

// Get the number of fields in a Notification.
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
        stringstream error_string;
        error_string << "UsblParser.cpp getNumberFields: Received  \""<< notification <<"\"" << flush;
        throw ValidationError(error_string.str());
        break;
    }
    stringstream error_string;
    error_string << "UsblParser.cpp getNumberFields: \""<< notification <<"\" is not determined" << flush;
    throw ValidationError(error_string.str());

}

// Get the integer from a response buffer in COMMAND mode.
int UsblParser::getNumber(string const &buffer)
{
    int value;
    string buffer_tmp = buffer;
    boost::algorithm::trim_if(buffer_tmp, boost::is_any_of("[*]"));
    stringstream ss(buffer_tmp);
    if (!(ss >> value))
        throw ParseError("UsblParser.cpp getNumber. Expected an integer response, but read \"" + buffer + "\"");
    return value;
}

// Parse Connection Status of underwater link.
ConnectionStatus UsblParser::parseConnectionStatus (string const &buffer)
{
    if (buffer.find("OFFLINE")!= string::npos)
    {
        if (buffer.find("OFFLINE CONNECTION FAILED") != string::npos)
            return OFFLINE_CONNECTION_FAILED;
        else if (buffer.find("OFFLINE TERMINATED") != string::npos)
            return OFFLINE_TERMINATED;
        else if (buffer.find("OFFLINE ALARM") != string::npos)
            return OFFLINE_ALARM;
        else if (buffer.find("OFFLINE READY") != string::npos)
            return OFFLINE_READY;
    }
    else if (buffer.find("INITIATION") != string::npos)
    {
        if (buffer.find("INITIATION LISTEN") != string::npos)
            return INITIATION_LISTEN;
        else if (buffer.find("INITIATION ESTABLISH") != string::npos)
            return INITIATION_ESTABLISH;
        else if (buffer.find("INITIATION DISCONNECT") != string::npos)
            return INITIATION_DISCONNECT;
    }
    else if (buffer.find("ONLINE")!= string::npos)
        return ONLINE;
    else if (buffer.find("BACKOFF")!= string::npos)
        return BACKOFF;
    else if (buffer.find("NOISE")!= string::npos)
        return NOISE;
    else if (buffer.find("DEAF")!= string::npos)
        return DEAF;

    throw ParseError("UsblParser.cpp parseConnectionStatus. Waiting for Connection Status but read \"" + buffer + "\"");
}

// Parse Delivery Status of a Message.
DeliveryStatus UsblParser::parseDeliveryStatus (string const &buffer)
{
    if (buffer.find("DELIVERING") != string::npos)
        return PENDING;
    else if (buffer.find("EMPTY") != string::npos)
        return EMPTY;
    else if (buffer.find("FAILED") != string::npos)
        return FAILED;
    else if (buffer.find("EXPIRED") != string::npos)
        return EXPIRED;

    throw ParseError("UsblParser.cpp parseDeliveryStatus. Waiting for Delivery Status but read \"" + buffer + "\"");
}
