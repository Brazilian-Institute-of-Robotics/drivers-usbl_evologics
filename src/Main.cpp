#include <iostream>
#include <usbl_evologics/Driver.hpp>
#include "Driver.hpp"

int main(int argc, char** argv)
{
	usbl_evologics::Driver driver;
        driver.open("file://tmp");
        std::cout << "Driver opened" <<std::endl;
        struct usbl_evologics::InstantMessage im;
        im.destination = 10;
        im.delivery_report = true;
        im.deliveryStatus = usbl_evologics::PENDING;
        driver.sendInstantMessage(&im);
        std::cout << im.deliveryStatus << std::endl;
        driver.read();
        std::cout << im.deliveryStatus << std::endl;
        std::cout << "END PROGRAM" << std::endl;
        /*while(true){
            driver.read();
        }*/
	return 0;
}
