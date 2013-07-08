#ifndef _DUMMYPROJECT_DRIVER_TYPES_HPP_
#define _DUMMYPROJECT_DRIVER_TYPES_HPP_
namespace usbl_evologics
{
    enum DeliveryStatus {
        PENDING,
        DELIVERED,
        FAILED
    };
    enum ConnectionStatus {
        OFFLINE,
        OFFLINE_CONNECTION_FAILED,
        OFFLINE_TERMINATED,
        OFFLINE_ALARM,
        INITIATION_LISTEN,
        INITIATION_ESTABLISH,
        INITIATION_DISCONNECT,
        ONLINE,
        BACKOFF
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
    struct DeviceStats {
        int dropCount;
        int overflowCount;
        int localRemoteBitrate;
        int remoteLocalBitrate;
        int receivedSignalStrengthIndicator;
        int signalIntegrityLevel;
        int propagationTime;
        int relativeVelocity;
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
    enum InterfaceType {
        SERIAL,
        ETHERNET
    };
    struct Position {
        int time;
        float x;
        float y;
        float z;
    };
    struct InterfaceStatus{
        int time;
        struct Position position;
        std::vector<struct SendInstantMessage*> instantMessages;
        struct DeviceSettings deviceSettings;
        struct DeviceStats deviceStats;
        enum InterfaceType interfaceType;
    };
    class UsblDriverCallbacks
    {
        public:
            virtual void gotInstantMessage(struct ReceiveInstantMessage *im) =0;
            virtual void gotBurstData(uint8_t const *data, size_t data_size) =0;
    };
}
#endif
