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
    ss << im.buffer;

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

    // Remove <end-line> (\r\n) from buffer
    splitted[9].erase(splitted[9].end()-2, splitted[9].end());
    im.buffer = splitted[9];

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
    splitted.clear();
    boost::split( splitted, buffer, boost::is_any_of( symbol ), boost::token_compress_on );
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

// Get the double from a response buffer in COMMAND mode.
double UsblParser::getDouble(string const &buffer)
{
    string buffer_tmp = buffer;
    boost::algorithm::trim_if(buffer_tmp, boost::is_any_of("[*]"));
    std::string::size_type sz;     // alias of size_t
    return stod(buffer_tmp, &sz);
}

// Parse AcousticConnection Status of underwater link.
AcousticConnection UsblParser::parseConnectionStatus (string const &buffer)
{
    AcousticConnection connection;
    connection.time = base::Time::now();
    if (buffer.find("OFFLINE")!= string::npos)
    {
        if (buffer.find("OFFLINE CONNECTION FAILED") != string::npos)
            connection.status = OFFLINE_CONNECTION_FAILED;
        else if (buffer.find("OFFLINE TERMINATED") != string::npos)
            connection.status = OFFLINE_TERMINATED;
        else if (buffer.find("OFFLINE ALARM") != string::npos)
            connection.status = OFFLINE_ALARM;
        else if (buffer.find("OFFLINE READY") != string::npos)
            connection.status = OFFLINE_READY;
    }
    else if (buffer.find("INITIATION") != string::npos)
    {
        if (buffer.find("INITIATION LISTEN") != string::npos)
            connection.status = INITIATION_LISTEN;
        else if (buffer.find("INITIATION ESTABLISH") != string::npos)
            connection.status = INITIATION_ESTABLISH;
        else if (buffer.find("INITIATION DISCONNECT") != string::npos)
            connection.status = INITIATION_DISCONNECT;
    }
    else if (buffer.find("ONLINE")!= string::npos)
        connection.status = ONLINE;
    else if (buffer.find("BACKOFF")!= string::npos)
        connection.status = BACKOFF;
    else if (buffer.find("NOISE")!= string::npos)
        connection.status = NOISE;
    else if (buffer.find("DEAF")!= string::npos)
        connection.status = DEAF;
    else
        throw ParseError("UsblParser.cpp parseConnectionStatus. Waiting for Connection Status but read \"" + buffer + "\"");
    // Get amount of free buffer of channels
    vector<string> splitted;
    boost::split( splitted, buffer, boost::algorithm::is_any_of( " " ) );
    connection.freeBuffer.clear();
    for(int i=0; i<splitted.size(); i++)
    {
        if(isdigit(splitted.at(i)[0]))
            connection.freeBuffer.push_back(atoi(splitted.at(i).c_str()));
    }
    return connection;
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

// Parse current settings.
DeviceSettings UsblParser::parseCurrentSettings (string const &buffer)
{
    DeviceSettings settings;
    settings.time = base::Time::now();
    vector<string> splitted;
    boost::split( splitted, buffer, boost::algorithm::is_any_of( "\r\n" ), boost::token_compress_on );
    // Remove last empty string from vector
    splitted.pop_back();

    for( int i=0; i < splitted.size(); i++ )
    {
        vector<string> splitted2 = splitValidate(splitted.at(i), ":", 2);
        if(splitted2.at(0).find("Source Level Control") != string::npos)
        {
            if(atoi(splitted2.at(1).c_str()) == 0)
                settings.sourceLevelControl = false;
            else
                settings.sourceLevelControl = true;
        }
        else if(splitted2.at(0).find("Source Level") != string::npos)
            settings.sourceLevel = (SourceLevel) atoi(splitted2.at(1).c_str());
        else if(splitted2.at(0).find("Gain") != string::npos)
        {
            if(atoi(splitted2.at(1).c_str()) == 0)
                settings.lowGain = false;
            else
                settings.lowGain = true;
        }
        else if(splitted2.at(0).find("Carrier Waveform ID") != string::npos)
            settings.carrierWaveformId = atoi(splitted2.at(1).c_str());
        else if(splitted2.at(0).find("Local Address") != string::npos)
            settings.localAddress = atoi(splitted2.at(1).c_str());
        else if(splitted2.at(0).find("Cluster Size") != string::npos)
            settings.carrierWaveformId = atoi(splitted2.at(1).c_str());
        else if(splitted2.at(0).find("Highest Address") != string::npos)
            settings.highestAddress = atoi(splitted2.at(1).c_str());
        else if(splitted2.at(0).find("Cluster Size") != string::npos)
            settings.clusterSize = atoi(splitted2.at(1).c_str());
        else if(splitted2.at(0).find("Packet Time") != string::npos)
            settings.packetTime = atoi(splitted2.at(1).c_str());
        else if(splitted2.at(0).find("Retry Timeout") != string::npos)
            settings.retryTimeout = atoi(splitted2.at(1).c_str());
        else if(splitted2.at(0).find("Wake Up Active Time") != string::npos)
            settings.wuActiveTime = atoi(splitted2.at(1).c_str());
        else if(splitted2.at(0).find("Wake Up Period") != string::npos)
            settings.wuPeriod = atoi(splitted2.at(1).c_str());
        else if(splitted2.at(0).find("Promiscuous Mode") != string::npos)
        {
            if(atoi(splitted2.at(1).c_str()) == 0)
                settings.promiscuosMode = false;
            else
                settings.promiscuosMode = true;
        }
        else if(splitted2.at(0).find("Sound Speed") != string::npos)
            settings.speedSound = atoi(splitted2.at(1).c_str());
        // "Rerty". That is exactly what BIR's usbl send.
        else if(splitted2.at(0).find("IM Rerty Count") != string::npos)
            settings.imRetry = atoi(splitted2.at(1).c_str());
        else if(splitted2.at(0).find("Retry Count") != string::npos)
            settings.packetTime = atoi(splitted2.at(1).c_str());
        else if(splitted2.at(0).find("Idle Timeout") != string::npos)
            settings.idleTimeout = atoi(splitted2.at(1).c_str());
        else if(splitted2.at(0).find("Hold Timeout") != string::npos)
            settings.wuHoldTimeout = atoi(splitted2.at(1).c_str());
        else if(splitted2.at(0).find("Pool Size") != string::npos)
        {
            // Get values for each channels available
            vector<string> splitted3;
            boost::split( splitted3, splitted2.at(1), boost::algorithm::is_any_of( " " ) );
            settings.poolSize.clear();
            for(int i=0; i<splitted3.size(); i++)
            {
                if(isdigit(splitted3.at(i)[0]))
                    settings.poolSize.push_back(atoi(splitted3.at(i).c_str()));
            }
        }
        else
            throw ParseError("UsblParser.cpp parseCurrentSettings. Waiting for attribute to set but read \"" + splitted2.at(0) + "\" in buffer \"" + buffer +"\"");
    }
    return settings;
}

// Parse Multipath structure
// TODO need implementation
vector<MultiPath> UsblParser::parseMultipath (string const &buffer)
{
    vector<MultiPath> vec_multipath;
    // Ignore last "\n\r\n" from buffer.
    vector<string> splitted = splitValidate(buffer.substr(0,buffer.size()-3), "\n", 8);

    for( int i=0; i < splitted.size(); i++ )
    {
        string msg = splitted.at(i);
        string::size_type npos = string::npos;
        // Find first value
        if ((npos = msg.find_first_of("0123456789")) != string::npos)
        {
            MultiPath multipath;
            multipath.timeline = atoi(msg.c_str());
            // Find separator
            if ((npos = msg.find(" ")) != string::npos)
                msg = msg.substr(npos+1, msg.size()-npos);
            // Look for second value
            if ((npos = msg.find_first_of("0123456789")) != string::npos)
                multipath.signalIntegrity = atoi(msg.c_str());
            vec_multipath.push_back(multipath);
        }
    }
    return vec_multipath;
}
