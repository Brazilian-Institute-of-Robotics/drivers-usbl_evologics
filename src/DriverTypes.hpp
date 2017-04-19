#ifndef _DUMMYPROJECT_DRIVER_TYPES_HPP_
#define _DUMMYPROJECT_DRIVER_TYPES_HPP_
#include <string>
#include <stdint.h>
#include <vector>
#include <base/Time.hpp>
namespace usbl_evologics
{

/** Type of interface with local device.
 *
 */
enum InterfaceType
{
    // Interface with modem/vehicle.
    SERIAL,
    // Interface with USBL/Dock/Boat
    ETHERNET
};

///The type of operation.
/** Type of operation.
 *
 * In DATA mode, device can exchange raw data with remote device and receive commands/notifications in AT format.
 * In COMMAND mode, device can NOT exchange raw data, only directs commands/notifications.
 */
enum OperationMode
{
    // For raw sensors data. Can send command/notification with Time Independent Escape Sequence (TIES)
    DATA,
    // For command/notification exclusively
    COMMAND
};

/** Connection status of the acoustic connection to the remote device.
 *
 */
enum ConnectionStatus
{
    // Initial state after switching on/reset
    OFFLINE_READY,
    // Acoustic connection failed
    OFFLINE_CONNECTION_FAILED,
    // Acoustic connection failed or has been terminated
    OFFLINE_TERMINATED,
    // Internal error has occurred, reset the device
    OFFLINE_ALARM,
    // The device is ready for immediate connection initiated by the remote side
    INITIATION_LISTEN,
    // The device attempts to establish an acoustic connection with the remote side
    INITIATION_ESTABLISH,
    // Acoustic connection is being closed
    INITIATION_DISCONNECT,
    // Acoustic connection established and active
    ONLINE,
    // An acoustic connection between other network nodes is detected.
    // Local device will stay in Backoff state for a random Backoff Timeout interval.
    BACKOFF,
    // The device is in Noise State. Acoustic connection is impossible
    NOISE,
    // The device is in Deaf State, receiving incoming transmissions is impossible.
    DEAF
};

// Response to a command by local device. Every command generates a response
/** Kind of response to a command by local device.
 *
 * Command generates a response. Command AT0 (switch to data mode) does'nt generate response.
 */
enum CommandResponse
{
    // Command accepted and will be applied as soon as possible. OK
    COMMAND_RECEIVED,
    // Response to a request. Current setting value.
    VALUE_REQUESTED,
    // Error message
    ERROR,
    // Busy message
    BUSY,
    //No response.
    NO_RESPONSE
};

/** Notification that can be received.
 *
 * Notification are asynchronous. They doesn't require a command to be received.
 * They can arrive at any time, and each one has a defined structure.
 */
enum Notification
{
    // Instant Message received.
    RECVIM,
    // Synchronous Instant Message received.
    RECVIMS,
    // PiggyBack Message received.
    RECVPBM,
    // Report of sending a Instant Message. Delivered or Failed.
    DELIVERY_REPORT,
    // Pose of remote device.
    USBLLONG,
    // Orientation of remote device, in case the pose wasn't computed.
    USBLANGLE,
    // Extra notifications. See about Extended notification. Not implemented.
    EXTRA_NOTIFICATION,
    // No notification
    NO_NOTIFICATION
};

/** Delivery status of an instant message.
 *
 * Only if required by a specific command.
 */
enum DeliveryStatus
{
    // Instant Message has been delivered.
    DELIVERED,
    // No messages are been delivered.
    EMPTY,
    // Message is been delivered.
    PENDING,
    // Delivered of an Instant Message was not acknowledged.
    FAILED,
    // A synchronous Instant Message has expired.
    EXPIRED,
    // An Instant Message was canceled.
    CANCELED
};

/** Reset device or clear buffer.
 *
 * Parameter of command ATZn
 * Reset device, drop data and/or instant messages.
 */
enum ResetType
{
    // Reset device to stored settings and restart it.
    // TCP connection will be closed. Restart in DATA mode.
    // No command response.
    DEVICE = 0,
    // Drop raw data and terminate acoustic connection.
    ACOUSTIC_CONNECTION = 1,
    // Drop Instant Messages
    INSTANT_MESSAGES = 3,
    // Clear the transmission buffer - drop raw data and instant messages.
    SEND_BUFFER = 4
};

/** Firmware information
 *
 *  Parameter for command ATIn
 *  View firmware information
 */
enum FirmwareInformation
{
    // Firmware version number.
    VERSION_NUMBER = 0,
    // Physical layer protocol and data-link layer protocol
    PHY_MAC = 1,
    // Device Manufacturer.
    MANUFACTURER = 7
};

/** Sound Pressure Level (SPL) in transmission mode
 *
 *  Default value is 3 (MINIMAL).
 *  For test in air, use ONLY value MINIMAL.
 */
enum SourceLevel
{
    // Maximum Sound Pressure Level (SPL).
    // See The Factory Certificate value for further information.
    // For S2CR 48/78, Max SPL = 184 dB re 1uPa
    // SPL = 184
    MAXIMUM = 0,
    // Maximum-6dB.
    // SPL = 178
    HIGH = 1,
    // Maximum-12bB.
    // SPL = 172
    LOW = 2,
    // MINIMAL. In air test.
    // Maximum-20dB.
    // SPL = 164
    MINIMAL = 3,
    // Alias of above. For in air test.
    IN_AIR = 3
};

/** Pre-defined Address of devices
 *
 */
enum DeviceAddress
{
    // For sending a message in broadcast mode, to all devices.
    BROADCAST = 255,
    // Default value of auv's address
    AUV = 1,
    // Default value of dock's address
    DOCK = 2,
    // Case Remote Address is set to 0, local device would accept connection/data from any device, but could not initiate a connection.
    // After accept connection/data, it will adopt remote address of remote device.
    UNSET = 0
};

/** Notification information
 *
 * Kind of notification and its content.
 */
struct NotificationInfo
{
    Notification notification;
    std::string buffer;
};

/** Response information
 *
 * Kind of response and its content.
 */
struct ResponseInfo
{
    CommandResponse response;
    std::string buffer;
};

/** Firmware information
 *
 */
struct VersionNumbers
{
    // Firmware version number
    std::string firmwareVersion;
    // Physical and data-layer protocol versions
    std::string accousticVersion;
    // Device manufacturer
    std::string manufacturer;
};

/**  Configuration for acoustic connection
 *
 */
struct StatusRequest
{
    base::Time time;

    // Interpreter type. True: AT interpreter. False: NET interpreter
    // At: Standard, for DATA mode and COMMAND mode
    // NET: Not tested, for Networking command and COMMAND mode.
    bool atInterpreter;

    // Parameter of command interpreter
    // protocolID = 0, global device settings can be edited.
    // protocolID = 1...7, global device settings cannot be edit. Identifier for Instant Message communication
    int protocolID;

    // Physical layer of local device
    // True: On. False: off
    bool physical;

    // Built-in battery voltage (in Volts). Can be external battery. Need validation
    double batteryVoltage;

};

/** Connection status of acoustic link
 *
 */
struct AcousticConnection
{
    base::Time time;

    // Connection status
    ConnectionStatus status;

    // Free transmission buffer space (in bytes)
    std::vector<int> freeBuffer;
};

/** Major device settings
 *
 */
struct DeviceSettings
{
    // True: low gain, reduced sensitivity. For short distance and test. 1
    // False: Normal gain, high sensitivity. 0
    bool lowGain;

    // Devices can just establish a connection with specific carrier Waveform ID combinations
    // The combinations are 0-1 and 2-2.
    // It's recommended to use 0-1 for a two devices connection and 2-2 for networking.
    int carrierWaveformId;

    // Address of local device
    int localAddress;

    // The Address of the remote device to transmit BurstData or Instant Messages.
    // If the Remote Address is 0 the device accepts every connection request, but can not initiate a connection.
    // remoteAddress of local device must match localAddress of remote device.
    int remoteAddress;

    // Define limits of devices in the network
    // Values: 2, 6, 14, 30, 62, 126, 254
    int highestAddress;

    // The number of packets in one train. It's recommended to use a cluster size less then
    // 10 for moving objects. For stationary you can use a cluster size up to 32.
    int clusterSize;

    // Maximum duration of a data packet. From 50..1000 (in ms).
    // packetTime must be equal for all devices.
    // Short values are recommend for challenging hydroacoustic channels.
    int packetTime;

    // How many times the device will retry to connect
    // Retry count 0-255
    int retryCount;

    // Retry timeout 500-12000 ms
    int retryTimeout;

    // The timeout before closing an idle acoustic connection
    // 0-3600 s
    int idleTimeout;

    // Speed of sound 1300-1700 m/s
    int speedSound;

    // Instant Message retry 0-255. 255 = retry indefinitely
    int imRetry;

    // False: Local device will only accept message addressed to it. 0
    // True: Receive message addressed to any device on network. 1
    bool promiscuosMode;

    // Wake Up active time.
    // 0..3600 (s)
    int wuActiveTime;

    // Wake Up period
    // 0..3600 (s)
    int wuPeriod;

    // Wake Up hold timeout
    // 0..3600 (s)
    int wuHoldTimeout;

    // Transmission buffer size (bytes) per channel.
    // 8096.. 2097152
    std::vector<int> poolSize;
};

/** Multipath propagation of acoustic signal, from transmitter to receiver.
 *
 * Geometry and reflection properties of underwater channel determine the number of
 * significant propagation path, the strengths and delays.
 */
struct MultiPath
{
    // Delay of path propagation (in us)
    int timeline;

    // Strengths of signal
    int signalIntegrity;
};

/** Acoustic channel performance
 *
 */
struct AcousticChannel
{
    base::Time time;

    // Channel of current input-output interface
    // Data transferring among different channel is impossible.
    // 0..7
    int channelNumber;

    // Dropped data from transmission buffer (bytes) of channelNumber, in bytes.
    // Cases: 1)ResetType; 2)idleTimeout; 3)transmission to remoteAddress 0
    int dropCount;

    // Current Overflow count of channelNumber, in bytes.
    int overflowCounter;

    //Local-2-Remote bitrate (bit/s)
    int localBitrate;

    // Remote-2-Local bitrate (bit/s)
    int remoteBitrate;

    // RSSI (Received Signal Strength Indicator). Signal level in dB. Higher values correspond stronger signals.
    // Signal acceptable for rssi between -20dB and -85dB.
    // In NOISE state, rssi return RMS of noise. Communication performs best when rssi exceeds noise by 6dB
    double rssi;

    // Signal integrity represent distortions of signal.
    // Weak connection of integrityLevel < 100.
    int signalIntegrity;

    // Propagation time between devices. Delay (im ms)
    int propagationTime;

    // Relative velocity between devices (in m/s)
    double relativeVelocity;

    std::vector<MultiPath> multiPath;

    // Data sent to remote device.
    long long unsigned int sent_raw_data;
    // Data sent with receipt acknowledgment from remote side
    // Got from usbl. How to reset it is unknown.
    long long unsigned int delivered_raw_data;
    // Data received from remote device.
    long long unsigned int received_raw_data;
};

/** IN NOISE state
 *
 */
struct NoiseSample
{
    double noise;
    int size;
    double sampleRate;
    bool lowGain;
    double rssi;
};

/** Device specific position structure.
 *
 *  To be independent.
 */
struct Position
{
    base::Time time;
    base::Time measurementTime;
    int remoteAddress;
    // Coordinates in local device's reference frame (in m)
    double x;
    double y;
    double z;
    // Coordinates, motion-compensated. (in m)
    double E;
    double N;
    double U;
    // Rotation angles of local device (in rad)
    double roll;
    double pitch;
    double yaw;
    // in us
    base::Time propagationTime;
    int rssi;
    int integrity;
    // Accuracy of the position fix, (in rad)
    double accuracy;
};

/** Device specific direction structure.
 *
 * In case device is not able to provide position, it provides Direction
 */
struct Direction
{
    base::Time time;
    base::Time measurementTime;
    int remoteAddress;
    // Coordinates in local device's reference frame. (in rad)
    double lBearing;
    double lElevation;
    // Coordinates, motion-compensated  (in rad)
    double bearing;
    double elevation;
    // Rotation angles of local device (in rad)
    double roll;
    double pitch;
    double yaw;
    // in us
    int rssi;
    int integrity;
    double accuracy;
};

/** Instant message to be sent
 *
 */
struct SendIM
{
    base::Time time;
    int destination;
    bool deliveryReport;
    std::vector<uint8_t> buffer;
};

/** Received instant message
 *
 */
struct ReceiveIM
{
    base::Time time;
    int destination;
    int source;
    bool deliveryReport;
    base::Time duration;
    int rssi;
    int integrity;
    double velocity;
    std::vector<uint8_t> buffer;
};

/** Message Delivery status
 *
 */
struct MessageStatus
{
    base::Time time;
    SendIM sendIm;
    DeliveryStatus status;
    // The statement below should happen
    // messageSent = messageDelivered + messageFailed
    // Total of messages sent to remote device
    unsigned long long int   messageSent;
    // Total of message successfully delivered
    unsigned long long int   messageDelivered;
    // Total of messages failed.
    unsigned long long int   messageFailed;
    // Total of messages received.
    unsigned long long int   messageReceived;
    // Total of messages canceled.
    unsigned long long int   messageCanceled;
};

}
#endif
