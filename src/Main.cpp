#include <iostream>
#include <usbl_evologics/Driver.hpp>
#include "Driver.hpp"
#include "Exceptions.hpp"
#include <boost/algorithm/string.hpp>
using namespace usbl_evologics;


int main(int argc, char** argv)
{
    Driver *driver;
    driver = new Driver();
    driver->setInterface(ETHERNET);

    if(driver->getInterface() == ETHERNET)
    	//driver->openTCP(192.168.0.191, 9200);
    	driver->openTCP("localhost", 631);
    else
    	driver->openSerial("modem_ff", 19200);

    SendedIM im;
    im.destination = 2;
    im.deliveryReport = false;
    std::string s = "test12345";
    //std::copy( s.begin(), s.end(), std::back_inserter(im.buffer));
    im.buffer.resize(5);
    im.buffer[0] = 0xAA;
    im.buffer[1] = 0x01;
    im.buffer[2] = 0x22;
    im.buffer[3] = 0x12;
    im.buffer[4] = 0x02;

    driver->sendInstantMessage(im);
    driver->getCurrentSetting();
    driver->getConnetionStatus();


    bool send = true;
    bool doIt = true;

    while(doIt)
    	doIt = driver->sendCommand(send);


    return 0;
}
;
