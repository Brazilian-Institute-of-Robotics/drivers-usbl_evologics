#include <iostream>
#include <usbl_evologics/Driver.hpp>
#include "Driver.hpp"
#include "Exceptions.hpp"
//#include <boost/regex.hpp>
//#include <boost/algorithm/string.hpp>
using namespace usbl_evologics;



//void atv_get_current_settings() {
//    char message[] = {
//    0x2b, 0x2b, 0x2b, 0x41, 0x54, 0x26, 0x56, 0x3a,
//    0x33, 0x34, 0x35, 0x3a, 0x53, 0x6f, 0x75, 0x72,
//    0x63, 0x65, 0x20, 0x4c, 0x65, 0x76, 0x65, 0x6c,
//    0x3a, 0x20, 0x33, 0x0d, 0x0a, 0x53, 0x6f, 0x75,
//    0x72, 0x63, 0x65, 0x20, 0x4c, 0x65, 0x76, 0x65,
//    0x6c, 0x20, 0x43, 0x6f, 0x6e, 0x74, 0x72, 0x6f,
//    0x6c, 0x3a, 0x20, 0x30, 0x0d, 0x0a, 0x47, 0x61,
//    0x69, 0x6e, 0x3a, 0x20, 0x30, 0x0d, 0x0a, 0x43,
//    0x61, 0x72, 0x72, 0x69, 0x65, 0x72, 0x20, 0x57,
//    0x61, 0x76, 0x65, 0x66, 0x6f, 0x72, 0x6d, 0x20,
//    0x49, 0x44, 0x3a, 0x20, 0x31, 0x0d, 0x0a, 0x4c,
//    0x6f, 0x63, 0x61, 0x6c, 0x20, 0x41, 0x64, 0x64,
//    0x72, 0x65, 0x73, 0x73, 0x3a, 0x20, 0x32, 0x0d,
//    0x0a, 0x48, 0x69, 0x67, 0x68, 0x65, 0x73, 0x74,
//    0x20, 0x41, 0x64, 0x64, 0x72, 0x65, 0x73, 0x73,
//    0x3a, 0x20, 0x31, 0x34, 0x0d, 0x0a, 0x43, 0x6c,
//    0x75, 0x73, 0x74, 0x65, 0x72, 0x20, 0x53, 0x69,
//    0x7a, 0x65, 0x3a, 0x20, 0x31, 0x30, 0x0d, 0x0a,
//    0x50, 0x61, 0x63, 0x6b, 0x65, 0x74, 0x20, 0x54,
//    0x69, 0x6d, 0x65, 0x3a, 0x20, 0x37, 0x35, 0x30,
//    0x0d, 0x0a, 0x52, 0x65, 0x74, 0x72, 0x79, 0x20,
//    0x43, 0x6f, 0x75, 0x6e, 0x74, 0x3a, 0x20, 0x33,
//    0x0d, 0x0a, 0x52, 0x65, 0x74, 0x72, 0x79, 0x20,
//    0x54, 0x69, 0x6d, 0x65, 0x6f, 0x75, 0x74, 0x3a,
//    0x20, 0x31, 0x35, 0x30, 0x30, 0x0d, 0x0a, 0x57,
//    0x61, 0x6b, 0x65, 0x20, 0x55, 0x70, 0x20, 0x41,
//    0x63, 0x74, 0x69, 0x76, 0x65, 0x20, 0x54, 0x69,
//    0x6d, 0x65, 0x3a, 0x20, 0x31, 0x32, 0x0d, 0x0a,
//    0x57, 0x61, 0x6b, 0x65, 0x20, 0x55, 0x70, 0x20,
//    0x50, 0x65, 0x72, 0x69, 0x6f, 0x64, 0x3a, 0x20,
//    0x31, 0x32, 0x0d, 0x0a, 0x50, 0x72, 0x6f, 0x6d,
//    0x69, 0x73, 0x63, 0x75, 0x6f, 0x75, 0x73, 0x20,
//    0x4d, 0x6f, 0x64, 0x65, 0x3a, 0x20, 0x31,
//    0x0d, 0x0a, 0x53, 0x6f, 0x75, 0x6e, 0x64, 0x20,
//    0x53, 0x70, 0x65, 0x65, 0x64, 0x3a, 0x20, 0x31,
//    0x35, 0x30, 0x30, 0x0d, 0x0a, 0x49, 0x4d, 0x20,
//    0x52, 0x65, 0x72, 0x74, 0x79, 0x20, 0x43, 0x6f,
//    0x75, 0x6e, 0x74, 0x3a, 0x20, 0x31, 0x0d, 0x0a,
//    0x50, 0x6f, 0x6f, 0x6c, 0x20, 0x53, 0x69, 0x7a,
//    0x65, 0x3a, 0x20, 0x31, 0x36, 0x33, 0x38, 0x34,
//    0x0d, 0x0a, 0x48, 0x6f, 0x6c, 0x64, 0x20, 0x54,
//    0x69, 0x6d, 0x65, 0x6f, 0x75, 0x74, 0x3a, 0x20,
//    0x30, 0x0d, 0x0a, 0x49, 0x64, 0x6c, 0x65, 0x20,
//    0x54, 0x69, 0x6d, 0x65, 0x6f, 0x75, 0x74, 0x3a,
//    0x20, 0x31, 0x32, 0x30, 0x0d, 0x0a, 0x0d, 0x0a };
//
//    std::string msg = message;
//
//    int npos = std::string::npos;
//    if ((npos = msg.find(":")) != std::string::npos){
//        std::cout << msg.substr(0, npos) << std::endl;
//        std::cout << "npos: " << npos << std::endl;
//        msg = msg.substr(npos+1, msg.size()-npos);
//    }
//
//    if ((npos = msg.find(":")) != std::string::npos){
//        std::cout << msg.substr(0, npos) << std::endl;
//        std::cout << "npos: " << npos << std::endl;
//        msg = msg.substr(npos+1, msg.size()-npos);
//
//    }
//
//
//    std::cout << msg << std::endl;
//    std::cout << "data: " << msg.size()-2 << std::endl;
//
//
//
////    std::cout << msg << std::endl;
////    std::cout << "message size: " << msg.size() << std::endl;
////
////    std::vector<std::string> result;
////
////    char symbol[] = {0x0d, 0x0a};
////    boost::split( result, msg, boost::algorithm::is_any_of(symbol) );
////
////    std::cout << "total lines: " << result.size() << std::endl;
////
////    for (int i = 0; i < result.size(); i++){
////        if (!result[i].empty()){
////            std::cout << result[i] << std::endl;
////            std::vector<std::string> props;
////            boost::split( props, result[i] , boost::algorithm::is_any_of(":") );
////            std::cout << "props[0]: " << props[0] << std::endl;
////            std::cout << "props[1]: " << props[1] << std::endl;
////        }
////    }
//
//
//
//}


int main(int argc, char** argv)
{
    Driver *driver;
    driver = new Driver();
    driver->setInterface(ETHERNET);

    if(driver->getInterface() == ETHERNET)
    	driver->openTCP("192.168.0.191", 9200);
//    	driver->openTCP("localhost", 631);
    else
    	driver->openSerial("modem_ff", 19200);

    SendIM im;
    im.destination = 1;
    im.deliveryReport = true;
    std::string s = "test12345";
    std::copy( s.begin(), s.end(), std::back_inserter(im.buffer));
//    im.buffer.resize(5);
//    im.buffer[0] = 0xAA;
//    im.buffer[1] = 0x01;
//    im.buffer[2] = 0x22;
//    im.buffer[3] = 0x12;
//    im.buffer[4] = 0x02;



    driver->getConnetionStatus();
    driver->sendInstantMessage(im);
    driver->getConnetionStatus();
    driver->goSurface();
    driver->getIMDeliveryStatus();
    driver->goSurface();
    driver->getCurrentSetting();
    driver->goSurface();
    driver->getConnetionStatus();



    bool mail_command = true;
    bool doIt = true;
    std::string out_msg;
    std::string raw_data;
    int i=0;
    while(i<20)
	{
        doIt = driver->sendData(mail_command);
    	if(driver->readAnswer(mail_command, out_msg, raw_data))
    	{
    	    if(!out_msg.empty())
    		    std::cout << "Answer Command " << i << " : "<< out_msg << std::endl;
    		if(!raw_data.empty())
    		    std::cout << "raw data " << i << " : "<< raw_data << std::endl;
    	}
    	driver->getIMDeliveryStatus();
    	i++;
    	out_msg.clear();
    	raw_data.clear();
	}


//    atv_get_current_settings();

    return 0;
}
;
