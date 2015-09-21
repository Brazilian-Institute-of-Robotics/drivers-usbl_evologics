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
#include "base/Logging.hpp"

using namespace std;
namespace usbl_evologics
{

class Driver : public iodrivers_base::Driver
{

public:

    Driver();
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
    void sendCommand(string const &command);

    /** Send raw data to remote device.
     *
     * Use in DATA mode.
     * Doesn't require response.
     * @param raw_data string to be sent to remote device.
     *
     */
    void sendRawData(string const &raw_data);

    /** Read response from device.
     *
     * Read from device. Push Notification and Raw data in respective queue.
     * @result ReponseInfo. If incoming buffer is not a response, ResponseInfo.response = NO_RESPONSE.
     * ResponseInfo.response is the kind of response and ResponseInfo.buffer is the response.
     */
    ResponseInfo readResponse(void);

    /** Read input data till get a response.
     *
     * @param command that was sent to device.
     * @param expected response from device.
     * @return string with response content, in COMMAND mode
     */
    string waitResponse(string const &command, CommandResponse expected);

    /** Wait for a OK response
     *
     * @param command sent to device.
     */
    void waitResponseOK(string const &command);

    /** Wait for a integer response.
     *
     * @param command sent to device.
     * @return integer requested.
     */
    int waitResponseInt(string const &command);

    /** Wait for string response.
     *
     * @param command sent to device.
     * @return string requested.
     */
    string waitResponseString(string const &command);

    /** Get Underwater Connection Status.
     *
     * @return connection status
     */
    Connection getConnectionStatus(void);

    /** Get Current Setting parameters.
     *
     * TODO Parse values.
     */
    void getCurrentSetting(void);

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
    bool getIMDeliveryReport(string const &buffer);

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
    void switch2CMDmode(void);

    /** Switch to DATA mode.
     *
     * Doesn't require response.
     */
    void switch2DATAmode(void);

    /* Reset device, drop data and/or instant message
     *
     * @param type define what will be reset in device
     */
    void resetDevice(ResetType const &type);

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

    /** Parse a received Instant Message.
     *
     * @param buffer that contains the IM
     * @return Received Instant Message
     */
    ReceiveIM receiveInstantMessage(string const &buffer);

    /** Get the RigidBodyState pose of remote device.
     *
     * Only used by devices with ETHERNET interface.
     * Convert the data from internal struct to RigidBodyState.
     * @return RigidBodyState pose.
     * TODO implement.
     */
    base::samples::RigidBodyState getPose(Position const &pose);

    /** Get the Position pose of remote device.
     *
     * Only used by devices with ETHERNET interface.
     * Convert the received buffer from USBLLONG notification to Position.
     * It may have some data of interest.
     * @return Position pose.
     */
    Position getPose(string const &buffer);

    /** Helper method to separate AT and raw packets in a data stream
     */
    int extractRawFromATPackets(string const& buffer) const;

    /** Helper method to extract packets from raw data
     *
     * This is to be reimplemented in subclasses if the raw data
     * has a packet-based protocol. The default implementation will
     * just interpret any amount of raw data as a packet
     */
    virtual int extractRawDataPacket(string const& buffer) const;

    /** Given a buffer that starts with a TIE header (+++), return whether it
     * could be a AT command
     */
    int extractATPacket(string const& buffer) const;

    /** Pop out RawData from queueRawData.
     *
     *  @return string of raw data
     */
    std::string getRawData(void);

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

private:
    UsblParser	usblParser;

    OperationMode	mode;
    InterfaceType	interface;

    Position    usbl_pose;
    SendIM      sendedIM;
    ReceiveIM   receiveIM;

    VersionNumbers device;
    StatusRequest device_status;
    DeviceSettings device_settings;
    DataChannel channel;
    AcousticChannel acoustic_channel;

    /**
     * Queue of received Raw Data
     */
    queue<string> queueRawData;

    /**
     * Queue of received Notification
     */
    queue<NotificationInfo> queueNotification;


    static const int max_packet_size = 20000;

    /** Read packets
     *
     * @return string with data (response, notification or raw data).
     */
    string readInternal(void);

    /** Check if a Notification string is present in buffer.
     *
     * Auxiliary function of extractPacket()
     * To be used in COMMAND mode.
     * @param buffer to be analyzed
     * @return size of buffer till end of message, or -1 in case of no Notification.
     */
    int checkNotificationCommandMode(string const& buffer) const;

    /** Check the size of a particular response.
     *
     * Auxiliary function of extractPacket().
     * Response to command AT&V (getCurrentSetting) uses multiples '\r\n' and a final '\r\n\r\n'. Damn EvoLogics.
     * Maybe there are other command's responses that use the same pattern.
     * @param buffer to be analyzed
     * @return size of buffer till end of message.
     */
    int checkParticularResponse(string const& buffer) const;

    /** Check the size of regular response.
     *
     * Auxiliary function used by extractPacket().
     * Response and Notification end by a end-of-line '\r\n'.
     * @param buffer to be analyzed
     * @return size of buffer till end of message.
     */
    int checkRegularResponse(string const& buffer) const;

    /** Check kind of response.
     *
     * In DATA mode: +++<AT command>:<length>:<command response><end-of-line>
     * IN COMMAND mode: <response><end-of-line>
     * Throw ValidationError or ModeError in case of failure.
     * @param buffer to be analyzed
     * @return CommandResponse kind of response. If is not a response, returns NO_RESPONSE.
     */
    CommandResponse isResponse(string const &buffer);

    /** Check for a valid response in DATA mode.
     *
     *  Used by waitResponse().
     *  Throw ValidationError or ModeError in case of failure.
     *  @param buffer to be analyzed.
     *  @param command sent to device.
     */
    void validResponse(string const &buffer, string const &command);

    /** Check kind of notification.
     *
     * In DATA mode: +++AT:<length>:<notification><end-of-line>
     * IN COMMAND mode: <notification><end-of-line>
     * Throw ValidationError or ModeError in case of failure.
     * @param buffer to be analyzed.
     * @return Notification kind. If is not a notification, returns NO_NOTIFICATION.
     */
    Notification isNotification(string const &buffer);

    /** Check a valid notification in DATA mode.
     *
     * Used by isNotification().
     * Throw ValidationError or ModeError in case of failure.
     * @param buffer to be analyzed.
     */
    void validNotification(string const &buffer);

    /** Check a valid notification.
     *
     * Used by isNotification().
     * Can be used in DATA or COMMAND mode.
     * Throw ValidationError in case of failure.
     * @param buffer to be analyzed.
     * @param notification kind present in buffer.
     */
    void fullValidation(string const &buffer, Notification const &notification);

    // TODO. Check the best way to interpreted every kind of notification and what it should return.
    /** Interpret notification.
     *
     * Interpret the pose, instant messages and delivery reports notification in respective variable.
     * @param buffer to be interpreted.
     * @param notification kind of buffer.
     */
    void interpretNotification(string const &buffer, Notification const &notification);

    /** Filled command string to be sent to device.
     *
     *  Used by sendCommand().
     *  @param command to be sent.
     *  @return string filled.
     */
    string fillCommand(string const &command);

    /** Add a end line, according interface type.
     *
     * Used by fillCommand().
     * SERIAL, <end-line> = '\r'.
     * ETHERNET, <end-line> = '\n'.
     * @param command to be sent.
     * @return string command with end line.
     */
    string addEndLine (string const &command);

    /** Manage mode operation according command sent.
     *
     * Act before get a response.
     * @param command sent to device.
     */
    void modeManager(string const &command);

    /** Manage mode operation according command sent and response obtained.
     *
     * Act after get a response.
     * @param command sent to device.
     */
    void modeMsgManager(string const &command);

    /** Converts from euler angles to quaternions.
     *
     * euler = [roll, pitch, yaw]
     * @param eulerAngles - Euler angles vector
     * @return quaternion - Quaternion variable
     */
    base::Quaterniond eulerToQuaternion(const base::Vector3d &eulerAngles);

protected:

    //Pure virtual function from Driver
    virtual int extractPacket(uint8_t const* buffer, size_t buffer_size) const;
};

}
#endif 
