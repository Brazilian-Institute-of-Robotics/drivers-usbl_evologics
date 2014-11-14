#include <iostream>
#include <usbl_evologics/Driver.hpp>
#include "Driver.hpp"
#include "Exceptions.hpp"
#include <boost/algorithm/string.hpp>
using namespace usbl_evologics;
void writeSettings(usbl_evologics::Driver driver){
    //Device Settings Write and Read
    //driver.setCarrierWaveformId(3);
    //driver.setClusterSize(10);
    //driver.setIdleTimeout(120);
    //driver.setImRetry(100);
    //driver.setLocalAddress(2);
    //driver.setLowGain(false);
    //driver.setPacketTime(750);
    //driver.setRemoteAddress(1);
    //driver.setRetryCount(50);
    //driver.setRetryTimeout(1500);
    //driver.setSourceLevel(0);
    //driver.setSourceLevelControl(false);
    //driver.storeSettings();
    //driver.setSpeedSound(1500);
}
void readSettings(usbl_evologics::Driver *driver){
    //Device Settings Write and Read
    std::cout << "cwi" << driver->getCarrierWaveformId()<< std::endl;
    std::cout << "cs" << driver->getClusterSize()<< std::endl;
    std::cout << "it" << driver->getIdleTimeout()<< std::endl;
    std::cout << "ir" << driver->getImRetry()<< std::endl;
    std::cout << "la" << driver->getLocalAddress()<< std::endl;
    std::cout << "lg" << driver->getLowGain()<< std::endl;
    std::cout << "pt" << driver->getPacketTime()<< std::endl;
    std::cout << "ra" << driver->getRemoteAddress()<< std::endl;
    std::cout << "rc" << driver->getRetryCount()<< std::endl;
    std::cout << "rt" << driver->getRetryTimeout()<< std::endl;
    std::cout << "sl" << driver->getSourceLevel()<< std::endl;
    std::cout << "slc" << driver->getSourceLevelControl()<< std::endl;
    std::cout << "ss" << driver->getSpeedSound()<< std::endl;
}
std::string enumConnectionStatusStrings[] = {
    "OFFLINE",
    "OFFLINE_CONNECTION_FAILED",
    "OFFLINE_TERMINATED",
    "OFFLINE_ALARM",
    "INITIATION_LISTEN",
    "INITIATION_ESTABLISH",
    "INITIATION_DISCONNECT",
    "ONLINE",
    "BACKOFF"
};
std::string getConnectionStatusString(int enumVal){
    return enumConnectionStatusStrings[enumVal]; 
}

int main(int argc, char** argv)
{
    usbl_evologics::Driver driver;
    //Open Driver
    //driver.open("file:///home/nikpec/usblinput");
    if (argc == 2){
        std::cout << "ETHERNET" << std::endl;
        driver.open("tcp://192.168.0.253:9200");
    }
    else {
        std::cout << "SERIAL" <<  argc <<std::endl;
        driver.open("serial:///dev/ttyUSB0:19200");
    }
    readSettings(&driver);    
    //driver.setImRetry(1);
    //driver.setPacketTime(750);
    //driver.setLowGain(true);
    //driver.setSourceLevel(0);
    //driver.setSourceLevelControl(false);
    //driver.storeSettings();
    while (true){
        /*std::cout << "WHILE" << std::endl;
        std::cout << "The Connection Status is " << getConnectionStatusString(driver.getConnectionStatus()) << std::endl;
        std::cout << "The drop Counter is " << driver.getDropCounter() << std::endl;
        std::cout << "Local Remote Bitrate " << driver.getLocalRemoteBitrate() << std::endl;
        std::cout << "Overflows: " << driver.getOverflowCounter() << std::endl;
        std::cout << "Propagation Time: " << driver.getPropagationTime() << std::endl;
        std::cout << "Received Signal Strength Indicator: " << driver.getReceivedSignalStrengthIndicator() << std::endl;
        std::cout << "Relative Velocity: " << driver.getRelativeVelocity() << std::endl;
        std::cout << "Get Remote Local Bitrate: " << driver.getRemoteLocalBitrate() << std::endl;
        std::cout << "Signal Integrity Level: " << driver.getSignalIntegrityLevel() << std::endl;
        if (ethernet){
            usbl_evologics::SendInstantMessage im;
            im.destination = 2;
            im.deliveryReport = true;
            im.deliveryStatus = PENDING; 
            im.buffer[0] = 'h';
            im.buffer[1] = 'a';
            im.len = 2;*/
            //try {
                /*driver.sendInstantMessage(&im);
                sendIms++;
                std::cout << sendIms << " Instant Messages gesendet" << std::endl;*/
            /*} catch (InstantMessagingError){
            }*/
            /*driver.sendBurstData(reinterpret_cast<const uint8_t*>("Hello Wordld!"), 7);
        } else {
            if (driver.hasPacket()){
                std::cout << "DRIVER HAS A PACKET" << std::endl;
                uint8_t buffer[100];
                if (driver.read(buffer, 100)){
                    std::cout << "RECEIVED BURST DATA: " << buffer << std::endl;
                }
            }

            std::cout << "In the inbox are " << driver.getInboxSize() << " Instant Messages" << std::endl;
            }*/
        sleep(1);

    }
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

/*    while(true){
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
    }*/
    return 0;
}
