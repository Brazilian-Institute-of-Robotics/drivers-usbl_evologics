#ifndef _DUMMYPROJECT_DRIVER_TYPES_HPP_
#define _DUMMYPROJECT_DRIVER_TYPES_HPP_
#include <string>
#include <stdint.h>
#include <vector>
namespace usbl_evologics
{
    enum ResetType {
        DEVICE=0,
        ACOUSTIC_CONNECTION=1,
        INSTANT_MESSAGES=3,
        SEND_BUFFER=4
    };
    ///The delivery status of an instant message
    enum DeliveryStatus {
        WAIT_FOR_SEND,
        PENDING,
        DELIVERED,
        FAILED,
        CANCELED
    };
    ///The connection status of the acustic connection to the remote device
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
    struct VersionNumbers {
        std::string firmwareVersion;
        std::string accousticVersion;
        std::string apiVersion;
    };
   ///All device settings in a struct to set or get them all with one function
    struct DeviceSettings {
        int sourceLevel;
        bool sourceLevelControl;
        bool lowGain;
        /*Devices can just etablish a connection with specific carrier Waveform ID combinations
        * The combinations are 0-1 and 2-2.
        * It's recommended to use 0-1 for a two devices connection and 2-2 for networking.
        * For details have a look at p. 32 of the manual.
        */
        int carrierWaveformId;
        int localAddress;
        /*The Address of the remote Address to transmit BurstData or Instant Messages.
        * If the Remote Address is 0 the Device accepts evert connection etablishment.
        * The Remote Address is Changing in this case immedently to the Remote Address
        */
        int remoteAddress;
        /* The number of packets in one train. It's recommended to use a cluster size less then
        * 10 for moving objects. For stationary you can use a cluster size up to 32.
        */
        int clusterSize;
        /*The device calculates automatically the packet size. To calculate the packet size
        * the device uses the patemeters of the accustic channel and the packet time.
        * The packet time have to be between 50 and 1000 ms)
        */
        int packetTime; 
        ///Retry count 0-255
        int retryCount;
        ///Retry timeout 500-12000 ms
        int retryTimeout;
        /*The timeout before closing an idle acustiv connection
        * 0-3600 s
        */
        int idleTimeout;
        ///Speed of sound 1300-1700 m/s
        int speedSound;
        ///Instant Message retrys 0-255
        int imRetry;
        //int streamNumber; //TODO add support
        //int poolSize; //TODO add support
        //TODO add Wake-Up-Module specific entries
    };
    ///Statistics and indicators of the acustiv connection to the remote device
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
    ///Instant message to send it
    struct SendInstantMessage {
        int destination;
        bool deliveryReport;
        enum DeliveryStatus deliveryStatus;
        std::vector<uint8_t> buffer;
    };
    ///Received instant message
    struct ReceiveInstantMessage {
        int time;
        int destination;
        int source;
        std::vector<uint8_t> buffer;
    };
    ///The type of the interface to the local device
    enum InterfaceType {
        SERIAL,
        ETHERNET
    };
    ///Type of an asynchronous message from the local device
    enum AsynchronousMessages {
        NO_ASYNCHRONOUS,
        INSTANT_MESSAGE,
        DELIVERY_REPORT,
        CANCELEDIM,
        USBLLONG,
        USBLANGLE
    };
    ///Device specific position struct. To be independent
    struct Position {
        double time;
        double measure_time;
        double x;
        double y;
        double z;
        double propagation_time;
        int rssi;
        int integrity;
        double accouracy;
    };
}
#endif
