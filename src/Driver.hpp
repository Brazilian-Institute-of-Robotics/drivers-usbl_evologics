#ifndef _DUMMYPROJECT_DRIVER_HPP_
#define _DUMMYPROJECT_DRIVER_HPP_

#include <iodrivers_base/Driver.hpp>
#include <vector>
#include <string>
#include <map>
#include "UsblParser.hpp"
#include "DriverTypes.hpp"
namespace usbl_evologics
{
    
    class Driver : public iodrivers_base::Driver
    {
        private:
            int extractPacket (uint8_t const *buffer, size_t buffer_size) const;
            size_t readInternal(uint8_t  *buffer, size_t buffer_size);
            void sendWithLineEnding(std::string line);
            void setValue(std::string value_name, int value);
            void validateValue(int value, int min, int max);

            std::vector<uint8_t> buffer;
            UsblDriverCallbacks *mCallbacks;
            InterfaceStatus mInterfaceStatus;
            UsblParser* mParser;
        public: 
            Driver();
            ConnectionStatus getConnectionStatus();
            DeviceSettings getDeviceSettings();
            int getIntValue(std::string value_name);
            Position getPosition(bool x);
            int getSystemTime();
            void open(std::string const& uri);
            void read();
            void sendBurstData(uint8_t const *buffer, size_t buffer_size);
            void sendInstantMessage(usbl_evologics::SendInstantMessage *instantMessage);
            void setDeviceSettings(DeviceSettings device_settings);
            void setDriverCallbacks(UsblDriverCallbacks *cb);
            void setSystemTime(int time);
            void storeSettings();
            int waitSynchronousInt();
            int waitSynchronousInt(std::string command);
            void waitSynchronousOk();
            std::string waitSynchronousString(std::string command);

            //Configuration
            //Set
            void setCarrierWaveformId(int id);
            void setClusterSize(int size);
            void setIdleTimeout(int timeout);
            void setImRetry(int retries);
            void setLocalAddress(int address);
            void setLowGain(bool low_gain);
            void setPacketTime(int time);
            void setRemoteAddress(int address);
            void setRetryCount(int count);
            void setRetryTimeout(int timeout);
            void setSourceLevel(int source_level);
            void setSourceLevelControl(bool source_level_control);
            void setSpeedSound(int speed);
            //Get
            int getCarrierWaveformId();
            int getClusterSize();
            int getIdleTimeout();
            int getImRetry();
            int getLocalAddress();
            int getLowGain();
            int getPacketTime();
            int getRemoteAddress();
            int getRetryCount();
            int getRetryTimeout();
            int getSourceLevel();
            bool getSourceLevelControl();
            int getSpeedSound();

            //Stats
            int getDropCounter();
            int getLocalRemoteBitrate();
            int getOverflowCounter();
            int getPropagationTime();
            int getReceivedSignalStrengthIndicator();
            int getRelativeVelocity();
            int getRemoteLocalBitrate();
            int getSignalIntegrityLevel();
            
            //TODO see some device stats i.e. Drop counter, Overflow counter
            //reset some device stats i.e. drop counter, over flow counter
    };
 
}
#endif 
