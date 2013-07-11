#include <iostream>
#include <usbl_evologics/Driver.hpp>
#include "Driver.hpp"
#include <boost/algorithm/string.hpp>
using namespace usbl_evologics;
int main(int argc, char** argv)
{

    usbl_evologics::ReceiveInstantMessage im2;
    usbl_evologics::Driver driver;
    driver.open("file:///home/nikpec/usblinput");
//    driver.open("serial:///dev/ttyUSB0:19200");
//    driver.open("tcp://192.168.0.248:9200");
//    std::cout << "Driver opened" <<std::endl;
//    driver.setDriverCallbacks(cb);
//    usbl_evologics::SendInstantMessage im;
//    im.destination = 2;
//    uint8_t *tmp;
//    im.deliveryReport = true;
//    im.deliveryStatus = PENDING; 
//    im.len = 0;
//    im.buffer = tmp;
//    driver.sendInstantMessage(&im);

    //std::cout << "Set System Time" << std::endl;
    //driver.setSystemTime(50);
    //std::cout << "Ask for Time" << std::endl;
    //int time = driver.getSystemTime();
    //std::cout << "Time is " <<time << std::endl;
//    std::cout << "Ask for current Settings" << std::endl;
//    struct DeviceSettings ds;
//    ds = driver.getDeviceSettings();
    //std::cout << "The current lowGain i.e. is " << ds.lowGain <<std::endl;

//    driver.setSourceLevel(1);
//    driver.setImRetry(1);
//    driver.setLowGain(false);
//    driver.storeSettings();
    //driver.setSourceLevel(3);
//    ds = driver.getDeviceSettings();
//    std::cout << "The current source level i.e. is " << ds.sourceLevel <<std::endl;

    //std::cout << "Sending out hello world" << std::endl;
    //driver.sendBurstData(reinterpret_cast<const uint8_t*>("Hello Wordld!"), 13);
//    uint8_t *tmp = "Hello Wordld!";
    //driver.requestPosition(true);

    while(true){
        uint8_t buffer[1000];
        std::cout << "loop" << std::endl;
//        driver.sendBurstData(reinterpret_cast<const uint8_t*>("Hello Wordld!"), 7);
//        std::cout << im.deliveryStatus <<std::endl;
        std::cout << "Source Level: "<< driver.getSourceLevel()<< std::endl;
        if (driver.hasPacket()){
            std::cout << "Driver Has Packet" <<std::endl;
            std::cout << "read "<< driver.read(buffer, 1000) << " burst data" << std::endl;
        }
        
        std::cout << "In the Inbox are " << driver.getInboxSize() << "instant messages" << std::endl;
        if (driver.getInboxSize()){
            ReceiveInstantMessage rim = driver.dropInstantMessage();
            std::string buffer_as_string = std::string(reinterpret_cast<char const*>(rim.buffer));
            buffer_as_string = buffer_as_string.substr(0, rim.len);
            std::cout << "In der Message steht: " << buffer_as_string << std::endl;
        }
        std::cout << "AFTER DROP: In the Inbox are " << driver.getInboxSize() << "instant messages" << std::endl;
    }
    return 0;
}
