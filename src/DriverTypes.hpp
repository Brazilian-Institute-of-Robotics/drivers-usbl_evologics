#ifndef _DUMMYPROJECT_DRIVER_TYPES_HPP_
#define _DUMMYPROJECT_DRIVER_TYPES_HPP_
#include <string>
#include <stdint.h>
#include <vector>
#include <base/Time.hpp>
namespace usbl_evologics
{

	///The type of the interface to the local device
	enum InterfaceType
	{
        // Interface with modem/vehicle.
    	SERIAL,
    	// Interface with USBL/Dock/Boat
        ETHERNET
	};

    ///The type of operation.
    enum OperationMode
    {
        // For raw sensors data. Can send command/instant_message with Time Independent Escape Sequence (TIES)
    	DATA,
    	// For command/instant_message exclusively
        COMMAND
    };

    ///The connection status of the acoustic connection to the remote device
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
        // Local device will stay in Backoff state for a random Backokk Timeout interval.
        BACKOFF,
        // The device is in Noise State. Acoustic connection is impossible
        NOISE,
        // The device is in Deaf State, receiving incoming transmissions is impossible.
        DEAF
    };

    // Response to a command by local device. Every command generates a response
	enum CommandResponse
	{
        // Command accepted and will be applied as soon as possible. OK
		COMMAND_RECEIVED,
		// Response to a request. Current setting value
        VALUE_REQUESTED,
        // Error message
        ERROR,
        // Busy message
        BUSY,
        //No response. Only command ATO (switches from COMMAND to DATA mode) doesn't get response
  //      NO
    };

    // Notification that does'nt require a command to be received.
	// The number represents the number of fields splitted by comma(,) presents in a Notification
	// <Notification>,<data1>,<data2>,<data3>,...
	enum Notification
	{
        // Instant Message received
		RECVIM,
        // Synchronous Instant Message received
		RECVIMS,
        // PiggyBack Message received
		RECVPBM,
		// Report of sending a Instant Message. Delivered or Failed
        DELIVERY_REPORT,
        // Transmission canceled due the wrong delivery time. Try again
        CANCELED_IM,
        // Pose of remote device
        USBLLONG,
        // Orientation of remote device, in case the pose was computed
        USBLANGLE,
        // Extra notifications. See about Extended notification. Not implemented.
        EXTRA_NOTIFICATION,
        // No notification
   //     NO
    };

    ///The delivery status of an instant message. Only if required by a specific command.
    enum DeliveryStatus
    {
        // No messages are been delivered
    	EMPTY,
    	// Message is been delivered
        PENDING,
        // Delivered of an Instant Message was not acknowledged
        FAILED,
        // A synchronous Instant Message has expired
        EXPIRED
    };

    enum ResetType
    {
    	// Reset device to stored settings and restart it.
    	// TCP connection will be closed. Restart in DATA mode.
    	DEVICE=0,
    	// Terminate acoustic connection
        ACOUSTIC_CONNECTION=1,
        // Drop Instant Messages
        INSTANT_MESSAGES=3,
        // Clear the transmission buffer
        SEND_BUFFER=4
    };

    //?????
//	enum ReverseMode {
//        NO_REVERSE,
//        REVERSE_POSITION_SENDER,
//        REVERSE_POSITION_RECEIVER
//    };


	struct Answer
	{
		CommandResponse response;
		Notification notification;
	};

    struct VersionNumbers
    {
		// Firmware version number
        std::string firmwareVersion;
        // Physical and data-layer protocol versions
        std::string accousticVersion;
        // Device manufacturer
        std::string apiVersion;
    };


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

    struct Connection
    {
    	base::Time time;

    	// Connection status
    	ConnectionStatus status;

    	// Free transmission buffer space (in bytes)
    	std::vector<int> freeBuffer;
    };



   /// Major device settings
    struct DeviceSettings
    {
    	base::Time time;

    	// Defines Sound Pressure Level (SPL)
    	// 0: Maximum SPL. To be set carefully
    	// 1: Maximum - 6dB
    	// 2: Maximum - 12dB
    	// 3: Maximum - 20dB. Test IN AIR
        int sourceLevel;

        // True: local sourceLevel can be changed by remote device.
        //		  Matching of sourceLevel during connection.
        // False: local sourceLevel cannot be changed by remote device.
        bool sourceLevelControl;

        // True: low gain, reduced sensitivity. For short distance and test
        // False: Normal gain, high sensitivity
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

        // Number of message that keep connection. Above that value, the connection is closed.
        // If set to 0, connection will remain online
        int keepOnline;

        // The timeout before closing an idle acoustic connection
        // 0-3600 s
        int idleTimeout;

        // Channel of current input-output interface
        // Data transferring among different channel is impossible.
        // 0..7
        int currentChannel;

        // Time (s) since the device is powered on
        // Can be synchronized by NTP. Need check
        base::Time systemTime;

        // System Clock. Reset whem physical layer hardware of device is turned off
        base::Time clock;

        // Speed of sound 1300-1700 m/s
        int speedSound;

        // Instant Message retry 0-255. 255 = retry indefinitely
        int imRetry;

        // True: Local device will only accept message addressed to it
        // False: Receive message addressed to any device on network
        bool promiscuosMode;
    };

    // Wake-Up settings
    struct WakeUpSettings
    {
        // Wake Up active time.
    	// 0..3600 (s)
    	int wuActiveTime;

    	// Wake Up period
    	// 0..3600 (s)
    	int wuPeriod;

    	// Wake Up hold timeout
    	// 0..3600 (s)
        int wuHoldTimeout;

        // Awake Remote Mode
        // Local device awake a remote device in wake-up module
        // True: Try to awake
        bool awake;

        // Remote Active Time
        // Set in order to cover wuActiveTime of remote device
        // 0..3600 (s)
        int awakeTime;
    };

    struct DataChannel
    {
        // Channel of current input-output interface
        // Data transferring among different channel is impossible.
        // 0..7
        int channelNumber;

        // Transmission buffer size (bytes)
        // 8096.. 2097152
        int poolSize;

        // Dropped data from transmission buffer (bytes)
        // Cases: 1)ResetType; 2)idleTimeout; transmission to remoteAddress 0
        int dropCount;

        // Current Overflow count of the channel.
        int overflowCounter;

        // TRUE: generate pose for every Instant Message
        bool poseEnable;

        // TRUE: enables extended notification. Extra set of informations. Default FALSE
        bool extendedControl;

        base::Time time;
    };

    // Multipath propagation of acoustic signal, from transmitter to receiver.
    // Geometry and reflection properties of underwater channel determine the number of
    // significant propagation path, the strengths and delays.
    struct MultiPath
    {
    	// Delay of path propagation (in us)
    	int timeLine;

    	// Strengths of signal
    	int signalIntegrity;
    };

    struct AcousticChannel
    {
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
    	double signalIntegrity;

    	// Propagation time between devices. Delay (im ms)
    	int propagationTime;

    	// Relative velocity between devices (in m/s)
    	double relativeVelocity;

    	std::vector<MultiPath> multiPath;

    	base::Time time;
    };


    // IN NOISE state
    struct NoiseSample
    {
    	double noise;
    	int size;
    	double sampleRate;
    	bool lowGain;
    	double rssi;
    };


    ///Device specific position structure. To be independent
    struct Position
    {
    	base::Time time;
    	base::Time measurementTime;
        int remoteAddress;
        // Coordinates in local device's reference frame (in m)
        double x;
        double y;
        double z;
        // Coordinates in local world frame, if apply. (in m)
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

    ///Device specific direction structure. In case device is not able to provide position, it provides Direction
    struct Direction
    {
    	base::Time time;
    	base::Time measurementTime;
        int remoteAddress;
        // Coordinates in local device's reference frame (in rad)
        double lBearing;
        double lElevation;
        // Coordinates in world frame (in rad)
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


    ///Statistics and indicators of the acoustic connection to the remote device
    struct DeviceStats
    {
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
    struct SendIM
    {
        int destination;
        bool deliveryReport;
        //enum DeliveryStatus deliveryStatus;
        std::vector<uint8_t> buffer;
    };
    ///Received instant message
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






}
#endif
