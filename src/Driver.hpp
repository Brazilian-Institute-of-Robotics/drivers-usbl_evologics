#ifndef _DUMMYPROJECT_DRIVER_HPP_
#define _DUMMYPROJECT_DRIVER_HPP_

#include <iodrivers_base/Driver.hpp>
#include <vector>
#include <string>
#include <map>
#include <queue>
#include "UsblParser.hpp"
#include "DriverTypes.hpp"
#include "Exceptions.hpp"
#include "base/samples/RigidBodyState.hpp"
#include <base/Eigen.hpp>
#include "base/Pose.hpp"
#include "base-logging/Logging.hpp"

namespace usbl_evologics
{

class Driver : public iodrivers_base::Driver
{

public:

    Driver();
    Driver(const OperationMode &init_mode);
    ~Driver();


    /** Define the interface with device. ETHERNET or SERIAL.
     *
     * @param deviceInterface, ETHERNET or SERIAL
     */
    void setInterface(InterfaceType	deviceInterface);

    /** Send a command to device.
     *
     * Fill buffer with necessary data.
     * Manage the mode of operation according to command.
     * @param command to be sent.
     */
    void sendCommand(std::string const &command);

    /** Send a command to device and wait for the corresponding OK
     *
     * @param command prefix
     * @param the command parameters, if any
     */
    void sendCommandAndACK(std::string const &command, std::string const &parameters = "");

    /** Send raw data to remote device.
     *
     * Use in DATA mode.
     * Doesn't require response.
     * @param raw_data string to be sent to remote device.
     *
     */
    void sendRawData(std::vector<uint8_t> const &raw_data);

    /** Read response from device.
     *
     * Read from device. Push Notification and Raw data in respective queue.
     * @result ReponseInfo. If incoming buffer is not a response, ResponseInfo.response = NO_RESPONSE.
     * ResponseInfo.response is the kind of response and ResponseInfo.buffer is the response.
     */
    ResponseInfo readResponse(void);

    /** Check if a Notification string is present in buffer.
     *
     * Auxiliary function of extractPacket()
     * To be used in COMMAND mode.
     * @param buffer to be analyzed
     * @return size of buffer till end of message, or -1 in case of no Notification.
     */
    int checkNotificationCommandMode(std::string const& buffer) const;

    /** Check if am Instant Message Notification string is present in buffer.
     *
     * Auxiliary function of checkNotificationCommandMode() and extractPacket()
     * To be used in COMMAND mode.
     * @param buffer to be analyzed
     * @return size of buffer till end of message, or -1 in case of no Notification.
     */
    int checkIMNotification(std::string const& buffer) const;

    /** Check the size of a particular response.
     *
     * Auxiliary function of extractPacket().
     * Response to command AT&V (getCurrentSetting) uses multiples '\r\n' and a final '\r\n\r\n'. Damn EvoLogics.
     * Maybe there are other command's responses that use the same pattern.
     * @param buffer to be analyzed
     * @return size of buffer till end of message.
     */
    int checkParticularResponse(std::string const& buffer) const;

    /** Check the size of regular response.
     *
     * Auxiliary function used by extractPacket().
     * Response and Notification end by a end-of-line '\r\n'.
     * @param buffer to be analyzed
     * @return size of buffer till end of message.
     */
    int checkRegularResponse(std::string const& buffer) const;

    /** Check kind of response.
     *
     * In DATA mode: +++<AT command>:<length>:<command response><end-of-line>
     * IN COMMAND mode: <response><end-of-line>
     * Throw ValidationError or ModeError in case of failure.
     * @param buffer to be analyzed
     * @return CommandResponse kind of response. If is not a response, returns NO_RESPONSE.
     */
    CommandResponse isResponse(std::string const &buffer);

    /** Check kind of notification.
     *
     * In DATA mode: +++AT:<length>:<notification><end-of-line>
     * IN COMMAND mode: <notification><end-of-line>
     * Throw ValidationError or ModeError in case of failure.
     * @param buffer to be analyzed.
     * @return Notification kind. If is not a notification, returns NO_NOTIFICATION.
     */
    Notification isNotification(std::string const &buffer);

    /** Read input data till get a response.
     *
     * @param command that was sent to device.
     * @param expected response from device.
     * @return string with response content, in COMMAND mode
     */
    std::string waitResponse(std::string const &expected_prefix, std::string const &command, CommandResponse expected, bool ignore_unexpected_responses = false);

    /** Wait for a OK response
     *
     * @param command sent to device.
     */
    void waitResponseOK(std::string const &expected_prefix, std::string const &command);

    /** Wait for a integer response.
     *
     * @param command sent to device.
     * @return integer requested.
     */
    int waitResponseInt(std::string const &expected_prefix, std::string const &command);

    /** Wait for a floating point response.
     *
     * @param command sent to device.
     * @return double requested.
     */
    double waitResponseDouble(std::string const &expected_prefix, std::string const &command);

    /** Wait for a integer (that may be very long) response.
     *
     * @param command sent to device.
     * @return long long unsigned integer requested.
     */
    long long unsigned int waitResponseULLongInt(std::string const &expected_prefix, std::string const &command);

    /** Wait for string response.
     *
     * @param command sent to device.
     * @return string requested.
     */
    std::string waitResponseString(std::string const &expected_prefix, std::string const &command);

    /** Get Underwater Connection Status.
     *
     * @return connection status
     */
    AcousticConnection getConnectionStatus(void);

    /** Get Current Setting parameters.
     *
     * @return current DeviceSettings
     */
    DeviceSettings getCurrentSetting(void);

    /** get Instant Message Delivery status.
     *
     * @return delivery status
     */
    DeliveryStatus getIMDeliveryStatus(void);

    /** Delivery report notification for Instant Message.
     *
     * If a send Instant Message requires to be acknowledge by remote device,
     * a notification is sent to device.
     * @returns TRUE if IM delivery was successful. FALSE if IM not acknowledged
     *  (a false means the local device did not receive a delivered acknowledgment. The IM may actually be delivered).
     *  throw a exception if
     */
    bool getIMDeliveryReport(std::string const &buffer);

    /** Switch to COMMAND mode.
     *
     * Guard Time Escape Sequence.
     * Wait 1 second before and after send command.
     */
    void GTES(void);

    /** Switch to Command mode.
     *
     * No need for waiting time.
     */
    void switchToCommandMode(void);

    /** Switch to DATA mode.
     *
     * Doesn't require response.
     */
    void switchToDataMode(void);

    /* Reset device, drop data and/or instant message
     *
     * @param type define what will be reset in device
     */
    void resetDevice(ResetType const &type, bool ignore_unexpected_responses = false);

    /** Get interface type.
     *
     * @return interface type. SERIAL or ETHERNET.
     */
    InterfaceType getInterface(void);

    /** Send Instant Message to remote device.
     *
     * @param im Instant Message to be sent.
     */
    void sendInstantMessage(SendIM const &im);

    /** Get Instant Message parsed as string
     *
     * @param im Instant Message to be sent.
     * @return string parsed of im.
     */
    std::string getStringOfIM(SendIM const &im);

    /** Parse a received Instant Message.
     *
     * @param buffer that contains the IM
     * @return Received Instant Message
     */
    ReceiveIM receiveInstantMessage(std::string const &buffer);

    /** Get the RigidBodyState pose of remote device.
     *
     * Only used by devices with ETHERNET interface.
     * Convert the data from internal struct to RigidBodyState.
     * @return RigidBodyState pose.
     */
    base::samples::RigidBodyState getPose(Position const &pose);

    /** Get the Position pose of remote device.
     *
     * Only used by devices with ETHERNET interface.
     * Convert the received buffer from USBLLONG notification to Position.
     * It may have some data of interest.
     * @return Position pose.
     */
    Position getPose(std::string const &buffer);

    /** Get the Direction of remote device.
     *
     * Only used by devices with ETHERNET interface, in case the device is not able to compute the pose.
     * Convert the received buffer from USBLANGLE notification to Direction.
     * It may have some data of interest.
     * @return Direction direc.
     */
    Direction getDirection(std::string const &buffer);

    /** Converts from euler angles to quaternions.
     *
     * euler = [roll, pitch, yaw]
     * @param eulerAngles - Euler angles vector
     * @return quaternion - Quaternion variable
     */
    base::Quaterniond eulerToQuaternion(const base::Vector3d &eulerAngles);

    /** Helper method to separate AT and raw packets in a data stream
     */
    int extractRawFromATPackets(std::string const& buffer) const;

    /** Helper method to extract packets from raw data
     *
     * This is to be reimplemented in subclasses if the raw data
     * has a packet-based protocol. The default implementation will
     * just interpret any amount of raw data as a packet
     */
    virtual int extractRawDataPacket(std::string const& buffer) const;

    /** Given a buffer that starts with a TIE header (+++), return whether it
     * could be a AT command
     */
    int extractATPacket(std::string const& buffer) const;

    /** Pop out RawData from queueRawData.
     *
     *  @return string of raw data
     */
    std::vector<uint8_t> getRawData(void);

    /** verify if queueRawData has raw data.
     *
     * @return TRUE if queue has raw data, FALSE otherwise.
     */
    bool hasRawData(void);

    /** Pop out Notification from queueNotification.
     *
     *  @return NotificationInfo
     */
   NotificationInfo getNotification(void);

   /** verify if queueNotification has any notification.
    *
    * @return TRUE if queue has notification, FALSE otherwise.
    */
  bool hasNotification(void);

   /** Get mode of operation.
    *
    * DATA or COMMAND
    * @return Operation mode.
    */
   OperationMode getMode(void);

   /** Set the specific carrier Waveform ID
    *
    * Devices can just establish a connection with specific carrier Waveform ID combinations
    * The combinations are 0-1 and 2-2.
    * It's recommended to use 0-1 for a two devices connection and 2-2 for networking.
    *  @param value of carrier waveform
    */
   void setCarrierWaveformID(int value);

   /** Set number of packets in one train.
    *
    * It's recommended to use a cluster size less then 10 for moving objects.
    * For stationary you can use a cluster size up to 32.
    * @param value
    */
   void setClusterSize(int value);

   /** Set limits of devices in the network.
    *
    * @param value: 2, 6, 14, 30, 62, 126, 254
    */
   void setHighestAddress(int value);

   /** Set timeout before closing an idle acoustic connection
    *
    * @param value in seconds (0-3600 s)
    */
   void setIdleTimeout(int value);

   /** Set Instant Message retry count
    *
    * Range 0-255. 255 = retry indefinitely
    * @param value
    */
   void setIMRetry(int value);

   /** Set address of local device
    *
    * 1-highest_address
    * @param value
    */
   void setLocalAddress(int value);

   /** Set address of remote device
    *
    * 0-highest_address
    * @param value
    */
   void setRemoteAddress(int value);

   /** Get address of remote device
    *
    * 0-highest_address
    * @return address
    */
   int getRemoteAddress(void);

   /** Get highest address
    *
    * @return highest address
    */
   int getHighestAddress(void);

   /** Automatic positioning output
    *
    * @return 0 fro disable, 1 for enable
    */
   int getPositioningDataOutput(void);

   /** Enable or disable automatic positioning output
    *
    * TRUE: Enable automatic position output
    * FALSE: Disable
    * @param pose_on
    */
   void setPositioningDataOutput(bool pose_on);

   /** Set input amplifier gain
    *
    * Low gain recommended for short-distance communication or test.
    * TRUE low gain applied.
    * FALSE normal gain applied.
    * @param low_gain
    */
   void setLowGain(bool low_gain);

   /** Set maximum duration of a data packet.
    *
    * Packet Time MUST be equal for all devices.
    * Range 50..1000 (in ms).
    * Short values are recommend for challenging hydroacoustic channels.
    * @param value maximum duration of a data packet.
    */
   void setPacketTime(int value);

   /** Set if device will receive instant message addressed to others devices.
    *
    *  FALSE: Local device will only accept message addressed to it. 0
    *  TRUE: Receive message addressed to any device on network. 1
    *  @param promiscuos_mode
    */
   void setPromiscuosMode(bool promiscuos_mode);

   /** Set number of connection establishment retries.
    *
    * @param value number of connection retries
    */
   void setRetryCount(int value);

   /** Set time of wait for establish an acoustic connection
    *
    * retry timeout should exceed the round-trip time that corresponds to the device's
    * maximum operation range.
    * Range 500..12000 (in ms)
    * @param value in ms
    */
   void setRetryTimeout(int value);

   /** Set Source Level
    *
    * Defines Sound Pressure Level (SPL)
    * @param source_level
    */
   void setSourceLevel(SourceLevel source_level);

   /** Set if source level of local device can be changed remotely over a acoustic connection.
    *
    * TRUE: local sourceLevel can be changed by remote device.
    * Automatically change for source level of remote device during connection. 1
    * FALSE: local sourceLevel cannot be changed by remote device. 0
    * @param source_level_control
    */
   void setSourceLevelControl(bool source_level_control);

   /** Get source level of device
    *
    * @return source level
    */
   SourceLevel getSourceLevel(void);

   /** Get source level control of device
    *
    * True: local sourceLevel can be changed by remote device.
    *   Matching of sourceLevel during connection. 1
    * False: local sourceLevel cannot be changed by remote device. 0
    * @return source level control
    */
   bool getSourceLevelControl(void);

   /** Set speed of sound on water
    *
    * Range 1300..1700 m/s
    * @param value in m/s
    */
   void setSpeedSound(int value);

   /** Set active interval of acoustic channel monitoring.
    *
    * Command effect only on devices with the Wake Up Module installed.
    * Range: 0..3600 (in s)
    * NOTE: MUST be less than the total duration of the Wake Up cycle.
    * @param value in s
    */
   void setWakeUpActiveTime(int value);

   /** Set hold timeout after completed data transmission.
    *
    * Command effect only on devices with the Wake Up Module installed.
    * Range: 0..3600 (in s)
    * @param value in s
    */
   void setWakeUpHoldTimeout(int value);

   /** Set period of the acoustic channel monitoring cycle.
    *
    * Comprises an active interval and an idle interval.
    * Command effect only on devices with the Wake Up Module installed.
    * Range: 0..3600 (in s)
    * @param value in s
    */
   void setWakeUpPeriod(int value);

   /** Set transmission buffer size of actual data channel.
    *
    *  Before changing buffer size, the buffer will empty.
    *  Range: 8096..2097152
    *  @param value bufer size in bytes
    */
   void setPoolSize(int value);

   /** Reset Drop Counter.
    *
    */
   void resetDropCounter(void);

   /** Reset Overflow Counter.
    *
    */
   void resetOverflowCounter(void);

   /** Get firmware information of device.
    *
    * @return VersionNumbers
    */
   VersionNumbers getFirmwareInformation(void);

   /** Get last transmission's raw bitrate value of local-to-remote direction.
    *
    * Include both useful data (raw data) and the protocol overhead.
    * @return bitrate in bits per second.
    */
   int getLocalToRemoteBitrate(void);

   /** Get last transmission's raw bitrate value of remote-to-local direction.
    *
    * Include both useful data (raw data) and the protocol overhead.
    * @return bitrate in bits per second.
    */
   int getRemoteToLocalBitrate(void);

   /** Get Received Signal Strength Indicator.
    *
    * Indicates the received signal level in dB re 1 V and represents the relative received signal strength.
    * Higher RSSI values correspond to stronger signals.
    * Signal strength is acceptable when measured RSSI values lie between -20dB and -85dB.
    * In NOISE state, return RMS of the noise. RSSI of communication should exceeds the noise by 6dB.
    * @return rssi in dB.
    */
   double getRSSI(void);

   /** Get Signal Integrity.
    *
    * Illustrate distortion of last acoustic signal.
    * High Signal Integrity Level values correspond to less distortion signals.
    * An acoustic link is weak if value is less than 100.
    * @return signal integrity level.
    */
   int getSignalIntegrity(void);

   /** Get acoustic signal's propagation time between communicating devices.
    *
    * @return propagation time in ms
    */
   int getPropagationTime(void);

   /** Get relative velocity between communicating devices.
    *
    * @return relative velocity in m/s
    */
   double getRelativeVelocity(void);

   /** Get Multipath propagation structure.
    *
    * @return Multipath components
    */
   std::vector<MultiPath> getMultipath(void);

   /** Get dropCounter of actual channel
    *
    * @return value in bytes
    */
   int getDropCounter(void);

   /** Get overflowCounter of actual channel
    *
    * @return value in bytes
    */
   int getOverflowCounter(void);

   /** Get channel number of current interface.
    *
    * @return value in bytes
    */
   int getChannelNumber(void);

   /** Get overall delivered raw data
    *
    * Usbl documentation doesn't say the max size neither a way to reset it, so a ullong_int was chosen.
    * @return counter of raw data bytes delivered to remote device.
    */
   long long unsigned int getRawDataDeliveryCounter(void);

   /** Set System Time for current time
    *
    * Default System Time value is the number of seconds elapsed since the device has been powered on.
    * Is possible to syncronize the System Time with a Network Time Protocol (NTP) server. Not implemented.
    */
   void setSystemTimeNow(void);

   /** Set operation mode of device
    *
    * Operation mode impact in
    * DATA, all data is interpreted as raw_data. Command are send through TIES string.
    * COMMAND, all data is interpreted as command. Raw_data is NOT transmitted.
    * @param mode, DATA or COMMAND mode
    */
   void setOperationMode(OperationMode const &new_mode);

   /** Store current setting profile
    *
    */
   void storeCurrentSettings(void);

   /** Restore factory settings and reset device.
    *
    */
   void RestoreFactorySettings(void);

   /** Get communication parameters
    *
    *  @return AcousticChannel with performance.
    */
   AcousticChannel getAcousticChannelparameters(void);

   /** Update parameters on device.
    *
    * Compare actual with desired settings before update.
    * Source level and source level control are not set here. They must be set in component according device's placement.
    * @param desired_setting, parameters that should be applied on device.
    * @param actual_setting, parameters present in device that will be used for compare.
    */
   void updateDeviceParameters(DeviceSettings const &desired_setting, DeviceSettings const &actual_setting);


private:
    UsblParser	usblParser;

    /** Mode of operation
     *
     * DATA or COMMAND
     */
    OperationMode	mode;

    /** Interface type
     *
     * ETHERNET or SERIAL
     */
    InterfaceType	interface;

    /**
     * Queue of received Raw Data
     */
    std::queue<std::string> queueRawData;

    /**
     * Queue of received Notification
     */
    std::queue<NotificationInfo> queueNotification;


    static const int max_packet_size = 20000;

    /** Read packets
     *
     * @return string with data (response, notification or raw data).
     */
    std::string readInternal(void);

    /** Check a valid notification.
     *
     * Used by isNotification().
     * Can be used in DATA or COMMAND mode.
     * Throw ValidationError in case of failure.
     * @param buffer to be analyzed.
     * @param notification kind present in buffer.
     */
    void notificationValidation(std::string const &buffer, Notification const &notification);

    /** Filled command string to be sent to device.
     *
     *  Used by sendCommand().
     *  @param command to be sent.
     *  @return string filled.
     */
    std::string fillCommand(std::string const &command);

    /** Add a end line, according interface type.
     *
     * Used by fillCommand().
     * SERIAL, <end-line> = '\r'.
     * ETHERNET, <end-line> = '\n'.
     * @param command to be sent.
     * @return string command with end line.
     */
    std::string addEndLine (std::string const &command);

    /** Manage mode operation according command sent.
     *
     * Act before get a response.
     * @param command sent to device.
     */
    void modeManager(std::string const &command);

    /** Manage mode operation according command sent and response obtained.
     *
     * Act after get a response.
     * @param command sent to device.
     */
    void modeMsgManager(std::string const &command);


protected:

    //Pure virtual function from Driver
    virtual int extractPacket(uint8_t const* buffer, size_t buffer_size) const;
};

}
#endif 
