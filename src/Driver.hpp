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
            int getIntValue(std::string value_name);
            size_t readInternal(uint8_t  *buffer, size_t buffer_size);
            void incommingDeliveryReport(std::string s);
            void incommingInstantMessage(std::string s);
            void sendWithLineEnding(std::string line);
            void setValue(std::string value_name, int value);
            void validateValue(int value, int min, int max);

            std::vector<uint8_t> buffer;
            std::vector<SendInstantMessage*> sendInstantMessages;
            std::vector<ReceiveInstantMessage> receivedInstantMessages;
            enum InterfaceType interfaceType;
            UsblParser mParser;
        public: 
            Driver();
            /*
             * Function returns the current connection status to the remote device
             */
            ConnectionStatus getConnectionStatus();
            /*
             * Function returns all settings in a struct
             */
            DeviceSettings getDeviceSettings();
            /*
             * Function returns the Positon of the remote device
             * as driver specific position struct.
             * Attention: The device only refresh the position by communication
             */
            Position getPosition(bool x);
            /* Function returns the internal system time.
             * You can set this time with setSystemTime
             */ 
            int getSystemTime();
            /* Open the driver.
             * Have a look to  documentation to iodriverbase for the meaning of uri
             */
            void open(std::string const& uri);
            /* Process the Driver and throws exception if there is an error.
             * TODO Burst data should be returned here, not via callback
             */
            size_t read(uint8_t *buffer, size_t size);
            /* Function sends out burst data.
             * Burst data are uncontroled data and is send to 
             * configured remote adress
             */
            void sendBurstData(uint8_t const *buffer, size_t buffer_size);
            /*
             * The functions sends a instantmessage and save a reference in a cue.
             * When a deliveryreport for this instantmessage comes in the message is
             * marked as delivered.
             */
            void sendInstantMessage(SendInstantMessage *instantMessage);
            size_t getInboxSize();
            ReceiveInstantMessage dropInstantMessage();
            /*
             * Sets every devicesettings
             */
            void setDeviceSettings(DeviceSettings device_settings);
            /*
             * Function sets the internal system time in the Device.
             */
            void setSystemTime(int time);
            /*
             * Function stores the Devicesettings permanently on the device.
             * If you not sure about the right settings, be careful with this function
             */
            void storeSettings();
            /*
             * Function waits for a Synchronous Int Value
             */
            int waitSynchronousInt();
            /*
             * Function waits for a Synchronous Int Value as response
             * to command. The Function validate the response and throws
             * an error if the response is not a valid response to the command.
             */
            int waitSynchronousInt(std::string command);
            /* Function waits for a OK. Throws an error, if the response is an Error.
             */
            void waitSynchronousOk();
            /*
             * Function waits for any Synchronous Message as response to command.
             * The function validate the response and throws an error, if the response is not valid.
             */
            std::string waitSynchronousString(std::string command);

            //Configuration
            /*
             * Functions to set devicesettings
             */
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
            /*
             * Functions to read the devicesettings
             */
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

            /*
             * Functions to read Connectionstats.
             * Connectionstats are signal quality indicators
             */
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
