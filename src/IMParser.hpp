#ifndef _IMDRIVER_IMPARSER_HPP_
#define _IMDRIVER_IMPARSER_HPP_

#include <iodrivers_base/Driver.hpp>
#include <boost/crc.hpp>
#include <bitset>
#include "DriverTypes.hpp"
#include "IMDataTypes.hpp"
namespace usbl_evologics
{
    class IMParser
    {
        // Parser DATA of Instant Messages.
        // AT*SENDIM,<length>,<destination address>,<flag>,<data>
        //  <data> = <Header><Information><CheckSum>
        //      <Header> = <Init> <size of Information> <kind of Information>
        //          <Init> = uint8_t, 0xAA
        //          <size of information> = uint16_t, number of bytes in <Information>
        //          <kind of information> = uint8_t. IMData: Pose, AUVCommand. See IMDataTypes
        //      <Information>
        //          Pose: <Timestamp><X><Y><Z><roll><pitch><yaw><accuracy><Frame>
        //              <Timestamp> = uint64_t
        //              <X>, <Y>, <Z> = int64_t (double), (in m)
        //              <roll>, <pitch>, <yaw> = int64_t (double), (in rad)
        //              <accuracy> = int64_t (double), (in rad)
        //              <Frame> = uint8_t: Frame. See IMDataTypes
        //          AUV Command: <Timestamp><Command>
        //              <Timestamp> = uint64_t
        //              <Command> = uint8_t: AUVCommand. See IMDataTypes
        //      <CheckSum> = uint8_t, 8-bit CRC, with initial value (0x00). Based on < http://www.datastat.com/sysadminjournal/maximcrc.cgi >

        private:


            bool checkCRC( uint8_t const* buffer, int size_buffer);
            uint8_t genCRC( uint8_t const* buffer, int size_buffer);
            std::string putInit(void);
            std::string fillAUVCommand( uint8_t information);


        public:
            IMParser();
            ~IMParser();

            std::string parsePosition(Position const& pose);
            std::string goSurface(void);
            std::string backDockStation(void);
            // 0 <= n <=9
            std::string doTask(int n);
    };
}
#endif
