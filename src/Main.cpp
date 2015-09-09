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





    std::string out_msg;
    std::string raw_data;
    Answer answer;
    answer.type = RAW_DATA;

    int i=0;
    while(driver->getSizeQueueRawData() != 0)
    {
        driver->sendRawData();
    }

    while(driver->getSizeQueueCommand() != 0)
    {
        WaitResponse wait_response = driver->sendCommand();
        while(wait_response == RESPONSE_REQUIRED && answer.type != RESPONSE)
        {
            try{
                answer = driver->readAnswer();
                if( answer.type == NOTIFICATION)
                {
                    switch (answer.notification){
                    case USBLLONG:
                        std::cout << "output new_pose" << std::endl;
                        break;
                    case RECVIM:
                        std::cout << "output received IM" << std::endl;
                        break;
                    }
                }
                else if( answer.type == RESPONSE)
                {
                    switch (answer.response){
                    case VALUE_REQUESTED:
                        std::cout << "output updated status" << std::endl;
                        break;
                    }
                }
                else if( answer.type == RAW_DATA)
                    std::cout<< "output raw data" <<std::endl;

            }
            catch (ValidationError &error) {
                std::cout << "Validation error: "<< &error << std::endl;
                // Temp. Ignore command
//                driver->queueCommand.pop();
            }
            catch (ParseError &error) {
                std::cout << "Parse error: "<< &error << std::endl;
                // Temp. Ignore command
//                driver->queueCommand.pop();
            }

        }
    }

    try{
        answer = driver->readAnswer();
        if( answer.type == NOTIFICATION)
        {
            switch (answer.notification){
            case USBLLONG:
                std::cout << "output new_pose" << std::endl;
                break;
            case RECVIM:
                std::cout << "output received IM" << std::endl;
                break;
            }
        }

    }
    catch (ValidationError &error) {
        std::cout << "Validation error: "<< &error << std::endl;
        // Temp. Ignore command
//        driver->queueCommand.pop();
    }
    catch (ParseError &error) {
        std::cout << "Parse error: "<< &error << std::endl;
        // Temp. Ignore command
//        driver->queueCommand.pop();
    }



/*
    while (possuir comandos){
        envia comando
        pega tempo atual
        while (tempo menor que limite e não recebeu resposta do comando) {
            try {
                ler dispositivo
                se for resposta para o loop com sucesso
                se for notificação interpreta notificação
            }
            catch {
                trata execeção
            }
        }


        if (timeout){
            trata timeout
        }

    }
*/



    return 0;
}
;
