#ifndef _DUMMYPROJECT_DRIVER_HPP_
#define _DUMMYPROJECT_DRIVER_HPP_

#include <iodrivers_base/Driver.hpp>
#include <vector>
#include <usbl_evologics/UsblParser.hpp>
#include <string>
namespace usbl_evologics
{
    class UsblParser;
    enum DeliveryStatus {
        PENDING,
        DELIVERED,
        FAILED
    };
    struct DeviceSettings {
        int sourceLevel;
        bool sourceLevelControl;
        bool lowGain;
        //Devices can just etablish a connection with specific carrier Waveform ID combinations
        //The combinations are 0-1 and 2-2.
        //It's recommended to use 0-1 for a two devices connection and 2-2 for networking.
        //For details have a look at p. 32 of the manual.
        int carrierWaveformId;
        int localAddress;
        //The Address of the remote Address to transmit BurstData or Instant Messages.
        //If the Remote Address is 0 the Device accepts evert connection etablishment.
        //The Remote Address is Changing in this case immedently to the Remote Address
        int remoteAddress;
        //The number of packets in one train. It's recommended to use a cluster size less then
        //10 for moving objects. For stationary you can use a cluster size up to 32.
        int clusterSize;
        //The device calculates automatically the packet size. To calculate the packet size
        //the device uses the patemeters of the accustic channel and the packet time.
        //The packet time have to be between 50 and 1000 ms)
        int packetTime; 
        //Retry count 0-255
        int retryCount;
        //Retry timeout 500-12000 ms
        int retryTimeout;
        //The timeout before closing an idle acustiv connection
        //0-3600 s
        int idleTimeout;
        //Speed of sound 1300-1700 m/s
        int speedSound;
        //Instant Message retrys 0-255
        int imRetry;
        //int streamNumber; //TODO add support
        //int poolSize; //TODO add support
        //TODO add Wake-Up-Module specific entries
    };
    struct SendInstantMessage {
        int destination;
        bool deliveryReport;
        enum DeliveryStatus deliveryStatus;
        size_t len;
        uint8_t *buffer;
    };
    struct ReceiveInstantMessage {
        int destination;
        int source;
        size_t len;
        uint8_t *buffer;
    };
    enum InterfaceMode {
        BURST_MODE,
        CONFIG_MODE
    };
    enum Pending {
        PENDING_OK,
        PENDING_POSITION,
        PENDING_TIME,
        PENDING_SETTINGS,
        ERROR,
        NO_PENDING
    };
    
    struct Position {
        int time;
        float x;
        float y;
        float z;
    };
    struct InterfaceStatus{
        enum InterfaceMode interfaceMode;
        enum Pending pending;
        int time;
        struct Position position;
        std::vector<struct SendInstantMessage*> instantMessages;
        struct DeviceSettings deviceSettings;
    };
    
    class UsblDriverCallbacks
    {
        public:
            virtual void gotInstantMessage(struct ReceiveInstantMessage *im) =0;
            virtual void gotBurstData(uint8_t const *data, size_t data_size) =0;
    };
    class Driver : public iodrivers_base::Driver
    {
            int extractPacket (uint8_t const *buffer, size_t buffer_size) const;
            struct InterfaceStatus mInterfaceStatus;
            std::vector<uint8_t> buffer;
            UsblParser* mParser;
            UsblDriverCallbacks *mCallbacks;
            void waitSynchronousMessage();
            void setInterfaceToConfigMode();
            void setValue(std::string value_name, int value);
            void validateValue(int value, int min, int max);
        public: 
            Driver();
            void read();
            void sendInstantMessage(struct usbl_evologics::SendInstantMessage *instantMessage);
            void sendBurstData(uint8_t const *buffer, size_t buffer_size);
            void open(std::string const& uri);
            struct Position requestPosition(bool x);
            struct Position getPosition();
            void setInterfaceToBurstMode();
            void setDriverCallbacks(UsblDriverCallbacks *cb);
            int getSystemTime();
            void setSystemTime(int time);
            struct DeviceSettings getDeviceSettings();
            void storeSettings();
            void setSourceLevel(int source_level);
            void setSourceLevelControl(bool source_level_control);
            void setLowGain(bool low_gain);
            void setCarrierWaveformId(int id);
            void setLocalAddress(int address);
            void setRemoteAddress(int address);
            void setClusterSize(int size);
            void setPacketTime(int time);
            void setRetryCount(int count);
            void setRetryTimeout(int timeout);
            void setIdleTimeout(int timeout);
            void setSpeedSound(int speed);
            void setImRetry(int retries);
            void setSettings(struct DeviceSettings device_settings);
            //TODO see some device stats i.e. Drop counter, Overflow counter
            //reset some device stats i.e. drop counter, over flow counter
    };
 
}
#endif 
