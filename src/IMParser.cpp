#include "IMParser.hpp"
//#include "Exceptions.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
using namespace usbl_evologics;
IMParser::IMParser()
{}
IMParser::~IMParser()
{}


std::string IMParser::parsePosition(Position const& pose)
{
    std::stringstream ss;

    return ss.str();
}

std::string IMParser::goSurface(void)
{
    uint8_t information = SURFACE;
    return fillAUVCommand(information);
}

std::string IMParser::backDockStation(void)
{
    uint8_t information = DOCK;
    return fillAUVCommand(information);
}

std::string IMParser::doTask(int n)
{
    uint8_t information;
    if(n==0)
        information = TASK0;
    else if(n==1)
        information = TASK1;
    else if(n==2)
        information = TASK2;
    else if(n==3)
        information = TASK3;
    else if(n==4)
        information = TASK4;
    else if(n==5)
        information = TASK5;
    else if(n==6)
        information = TASK6;
    else if(n==7)
        information = TASK7;
    else if(n==8)
        information = TASK8;
    else if(n==9)
        information = TASK9;
    return fillAUVCommand(information);
}

std::string IMParser::fillAUVCommand(uint8_t information)
{
    std::stringstream ss;

    uint16_t size = 0x0301;

    ss << (uint8_t)0xAA;
    ss << (uint8_t)((size >> 2) & 0xff);
    ss << (uint8_t)(size & 0xff);
    ss << (uint8_t)AUVCOMMAND;

//    ss << putInit() << (uint16_t) 0x0001 <<  (uint8_t) AUVCOMMAND << information;
    ss << information;
    ss << genCRC(reinterpret_cast<const uint8_t*>(ss.str().c_str()), ss.str().size());
    return ss.str();
}

bool IMParser::checkCRC( uint8_t const* buffer, int size_buffer)
{
    uint8_t crc = genCRC(buffer, size_buffer-1);

    if(buffer[size_buffer-1] == crc)
        return true;
    return false;
}

uint8_t IMParser::genCRC(uint8_t const * buffer, int size_buffer)
{
    // CRC-8-Dallas/Maxim. Using polynomial 0x31 and reflected input and remainder
    boost::crc_basic<8> result(0x31, 0, 0, true, true );
    result.process_bytes(buffer, size_buffer);
    return result.checksum();
}

std::string IMParser::putInit(void)
{
    std::stringstream ss;
    ss << (uint8_t) 0xAA;
    return ss.str();
}
