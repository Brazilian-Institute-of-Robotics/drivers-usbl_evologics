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

    	public:

    		Driver(InterfaceType deviceInterface);
    		~Driver();

            /*
             * Function waits for any Synchronous Message as response to command.
             * The function validate the response and throws an error, if the response is not valid.
             *
             * @param command A command to fit a response with this command
             * @return A int value extracted from a synchronous message
             */
			std::string waitSynchronousString(std::string command);


            /*
             * Function returns the current connection status to the remote device
             *
             * @return The Status of the connection in a enum
             */
            ConnectionStatus getConnectionStatus();

        private:

    		OperationMode mode;
    		InterfaceType interface;

            size_t readInternal(uint8_t  *buffer, size_t buffer_size);
            void sendWithLineEnding(std::string line);
            bool handleAsynchronousCommand(std::string buffer_as_string);
            void incomingDeliveryReport(std::string s);
            void incomingInstantMessage(std::string s);
            void incomingPosition(std::string s);
            SendInstantMessage currentInstantMessage;

//    	void cancelIm(std::string string_as_buffer);
//            int getIntValue(std::string value_name);
//            size_t readInternal(uint8_t  *buffer, size_t buffer_size);
//            void incomingDeliveryReport(std::string s);
//            void incomingInstantMessage(std::string s);
//            void incomingPosition(std::string s);
//            void sendInstantMessageInternal(SendInstantMessage& im);
//            void sendWithLineEnding(std::string line);
//            void setValue(std::string value_name, int value);
//            void validateValue(int value, int min, int max);
//
//            bool handleAsynchronousCommand(std::string buffer_as_string);
//            std::vector<uint8_t> buffer;
//            SendInstantMessage currentInstantMessage;
//            std::vector<ReceiveInstantMessage> receivedInstantMessages;
//            enum InterfaceType interfaceType;
//            VersionNumbers versionNumbers;
//            //Time of last soft reset
//            base::Time last_reset;
//            void requestVersionNumbers();
//            Position current_position;
//            ReverseMode reverse_mode;
//            base::Time last_position_sending;
//	    bool new_position_available;



//            /*
//             * Function returns the current connection status to the remote device
//             *
//             * @return The Status of the connection in a enum
//             */
//            ConnectionStatus getConnectionStatus();
//            /*
//             * Function returns all settings in a struct
//             *
//             * @return A struct with all device settings
//             */
//            DeviceSettings getDeviceSettings();
//            DeviceStats getDeviceStatus();
//            DeliveryStatus getInstantMessageDeliveryStatus();
//            /*
//             * Function returns the Positon of the remote device
//             * as driver specific position struct.
//             * Attention: The device only refresh the position by communication
//             *
//             * @param[in] x Positioning with Pitch, Roll and Heading compensation
//             * @return The position of the remote device in a driver specific struct
//             */
//            Position getPosition(bool x);

//
//	    bool newPositionAvailable();
//
//	    void sendPositionToAUV();
//
//
//            /* Function returns the internal system time in seconds.
//             * You can set this time with setSystemTime.
//             *
//             * @return The time in seconds as unix time.
//             */
//            VersionNumbers getVersionNumbers();
//            int getSystemTime();
//            /* Open the driver.
//             *
//             * @param[in] uri Uri to the interface to communicate withe the device. Have a look to  documentation to iodriverbase for the meaning of uri
//             */
//            void open(std::string const& uri, ReverseMode reverse_mode = NO_REVERSE);
//            /* Process the Driver and throws exception if there is an error.
//             *
//             * @param[out] buffer Pointer to a uint_8 array as buffer
//             * @param[in] size Size of the buffer array
//             * @return read Read bytes
//             */
//            size_t read(uint8_t *buffer, size_t const size);
//            /* Function sends out burst data.
//             * Burst data are uncontroled data and is send to
//             * configured remote adress
//             *
//             * @param[in] buffer Pointer to a uint_8 array as buffer
//             * @param[in] size Size of the buffer array
//             */
//            void sendBurstData(uint8_t const *buffer, size_t const buffer_size);
//            /*
//             * The functions sends a instantmessage and save a reference in a cue.
//             * When a deliveryreport for this instantmessage comes in the message is
//             * marked as delivered.
//             *
//             * @param[in, out] A pointer to instant message to send
//             */
//            void sendInstantMessage(SendInstantMessage instantMessage);
//            /*
//             * Returns the count of instant messages in the inbox
//             *
//             * @return count of instant messages in the inbox
//             */
//            size_t getInboxSize();
//            /*
//             * The function drops and returns the first instant message in the inbox.
//             *
//             * @return The first instant message in the inbox
//             */
//            ReceiveInstantMessage dropInstantMessage();
//
//            void resetDevice(ResetType type);
//            /*
//             * Sets every devicesettings
//             *
//             * @param[in] device_settings All device settings in a struct
//             */
//            void setDeviceSettings(DeviceSettings const device_settings);
//            /*
//             * Function sets the internal system time in the Device.
//             *
//             * @param time Time in seconds as unix time.
//             */
//            void setSystemTime(int const time);
//            /*
//             * Function stores the Devicesettings permanently on the device.
//             * If you not sure about the right settings, be careful with this function
//             */
//            void storeSettings();
//            /*
//             * Function waits for a Synchronous Int Value
//             *
//             * @return A int value extracted from a synchronous message
//             */
//            int waitSynchronousInt();
//            /*
//             * Function waits for a Synchronous Int Value as response
//             * to command. The Function validate the response and throws
//             * an error if the response is not a valid response to the command.
//             *
//             * @param command A command to fit a response with this command
//             * @return A int value extracted from a synchronous message
//             */
//            int waitSynchronousInt(std::string command);
//            /*
//             * Function waits for a OK. Throws an error, if the response is an Error.
//             */
//            void waitSynchronousOk();
//            /*
//             * Function waits for any Synchronous Message as response to command.
//             * The function validate the response and throws an error, if the response is not valid.
//             *
//             * @param command A command to fit a response with this command
//             * @return A int value extracted from a synchronous message
//             */
//            std::string waitSynchronousString(std::string command);
//
//            //Configuration
//            /*
//             * Functions to set devicesettings
//             */
//            void setCarrierWaveformId(int id);
//            void setClusterSize(int size);
//            void setIdleTimeout(int timeout);
//            void setImRetry(int retries);
//            void setLocalAddress(int address);
//            void setLowGain(bool low_gain);
//            void setPacketTime(int time);
//            void setRemoteAddress(int address);
//            void setRetryCount(int count);
//
//	    void setKeepOnline(int timeout);
//	    void setPositionEnable(int value);
//
//            void setRetryTimeout(int timeout);
//            void setSourceLevel(int source_level);
//            void setSourceLevelControl(bool source_level_control);
//            void setSpeedSound(int speed);
//            /*
//             * Functions to read the devicesettings
//             */
//            int getCarrierWaveformId();
//            int getClusterSize();
//            int getIdleTimeout();
//            int getImRetry();
//            int getLocalAddress();
//            int getLowGain();
//            int getPacketTime();
//            int getRemoteAddress();
//            int getRetryCount();
//            int getRetryTimeout();
//            int getSourceLevel();
//            bool getSourceLevelControl();
//            int getSpeedSound();
//
//            /*
//             * Functions to read Connectionstats.
//             * Connectionstats are signal quality indicators
//             */
//            int getDropCounter();
//            int getLocalRemoteBitrate();
//            int getOverflowCounter();
//            int getPropagationTime();
//            int getReceivedSignalStrengthIndicator();
//            int getRelativeVelocity();
//            int getRemoteLocalBitrate();
//            int getSignalIntegrityLevel();
//
//            //TODO reset some device stats i.e. drop counter, over flow counter
//
		protected:

			//Pure virtual function from Driver
			virtual int extractPacket(uint8_t const* buffer, size_t buffer_size) const;
    };
 
}
#endif 
