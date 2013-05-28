#include <iostream>
#include <usbl_evologics/Driver.hpp>
#include "Driver.hpp"
using namespace usbl_evologics;
class CB : public usbl_evologics::UsblDriverCallbacks{
    public:
        CB();
        void gotInstantMessage(struct usbl_evologics::ReceiveInstantMessage* im);
        void gotBurstData(uint8_t const *data, size_t data_size);
};
CB::CB():usbl_evologics::UsblDriverCallbacks(){
}
void CB::gotInstantMessage(struct usbl_evologics::ReceiveInstantMessage* im){
    std::cout << "Instant Message Callback" << std::endl;
};
void CB::gotBurstData(uint8_t const *data, size_t data_size){
    std::cout << "BURST CALLBACK" << std::endl;
};

int main(int argc, char** argv)
{
        CB *cb = new CB();
        usbl_evologics::ReceiveInstantMessage im2;
        cb->gotInstantMessage(&im2);
	usbl_evologics::Driver driver;
        driver.open("file:///home/nikpec/usblinput");
        std::cout << "Driver opened" <<std::endl;
        struct usbl_evologics::SendInstantMessage im;
        im.destination = 10;
        im.deliveryReport = true;
        im.deliveryStatus = usbl_evologics::PENDING;
        //driver.sendInstantMessage(&im);
        //driver.setInterfaceToBurstMode();
        //driver.read();
        driver.setDriverCallbacks(cb);
        int time = driver.getSystemTime();
        //Position pos = driver.requestPosition(true);
        std::cout << "Time: " << time <<  std::endl;

        //driver.read();
        std::cout << "END PROGRAM" << std::endl;
        /*while(true){
            driver.read();
        }*/
	return 0;
}
