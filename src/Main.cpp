#include <iostream>
#include <usbl_evologics/Driver.hpp>

int main(int argc, char** argv)
{
	usbl_evologics::Driver driver;
        driver.open("file://tmp");
        driver.requestPosition(true);
        std::cout << "END PROGRAM" << std::endl;
        /*while(true){
            driver.read();
        }*/
	return 0;
}
