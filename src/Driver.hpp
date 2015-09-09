#ifndef _DUMMYPROJECT_DRIVER_HPP_
#define _DUMMYPROJECT_DRIVER_HPP_

#include <iodrivers_base/Driver.hpp>
#include <vector>
#include <string>
#include <map>
#include <queue>
#include "UsblParser.hpp"
#include "IMParser.hpp"
#include "IMDataTypes.hpp"
#include "DriverTypes.hpp"
#include "Exceptions.hpp"
#include "base/samples/RigidBodyState.hpp"

namespace usbl_evologics
{

class Driver : public iodrivers_base::Driver
{

public:

    Driver();
    ~Driver();

    // Define the interface with device. ETHERNET or SERIAL.
    void setInterface(InterfaceType	deviceInterface);

    // Send a command from queueCommand to device.
    // check queueCommand. Pre-management of mode operation.
    // Fill command according mode operation.
    // Return false if queue is empty
    WaitResponse sendCommand(void);

    // send a raw data from queueRawdata to remote device
    // check queueRawData. Pop sent raw data.
    // Return false if queue is empty
    bool sendRawData(void);

    // return a valid answer, as a response or as a notification.
    // resp=true means the unique and necessary response of a command is received.
    // queueCommand.pop() realized in this case.
    Answer readAnswer(void);


    // List of Commands. They will be pushed to queueCommand
    void getConnetionStatus(void);
    void getCurrentSetting(void);
    void getIMDeliveryStatus(void);

    //General commands to AUV. TODO integrate with component and test
    void goSurface(void);


    // Guard Time Escape Sequence
    // Switches to COMMAND mode
    void GTES(void);

    // Get interface type. SERIAL or ETHERNET.
    InterfaceType getInterface(void);

    // Enqueue Instant Message to be sent to remote device.
    void sendInstantMessage(SendIM const &im);
    // TODO implement.
    ReceiveIM receiveInstantMessage(std::string const &buffer);
    // TODO implement.
    base::samples::RigidBodyState getNewPose(void);
    // TODO implement.
    void sendIMPose(base::samples::RigidBodyState const &send_pose);

    // Enqueue raw data to be sent to remote device.
    void enqueueRawData(std::string const& raw_data);

    // Return amount of command to be sent to device.
    int getSizeQueueCommand(void);
    // enqueue command string in queueCommand
    void enqueueCommand(std::string & command);

    // Return amount of raw data to be sent to remote device.
    int getSizeQueueRawData(void);


private:
    UsblParser	usblParser;
    IMParser    imParser;

    OperationMode	mode;
    InterfaceType	interface;

    Position    usbl_pose;
    SendIM      sendedIM;
    ReceiveIM   receiveIM;
    ConnectionStatus	connection_state;

    std::string raw_data;

    // CommandResponse	response;
    //	Notification	notification;

    VersionNumbers device;
    StatusRequest device_status;
    DeviceSettings device_settings;
    DataChannel channel;
    AcousticChannel acoustic_channel;

    std::queue<std::string> queueCommand;
    std::queue<std::string> queueRawData;

//    bool mailCommand;
    static const int max_packet_size = 20000;

    // Auxiliary function used by readAnswer().
    int readInternal(std::string& buffer);

    // Auxiliary function used by extractPacket().
    // Check if a specific Notification string is present in buffer, just in COMMAND mode.
    int checkNotificationCommandMode(std::string const& buffer) const;
    // Auxiliary function used by extractPacket().
    // Response to command AT&V (getCurrentSetting) uses multiples
    //  '\r\n' and a final '\r\n\r\n'. Damn EvoLogics.
    // Maybe there are other command's responses that use the same pattern.
    int checkParticularResponse(std::string const& buffer) const;
    // Auxiliary function used by extractPacket().
    int checkRegularResponse(std::string const& buffer) const;


    // In DATA mode: +++<AT command>:<length>:<command response><end-of-line>
    // IN COMMAND mode: <response><end-of-line>
    // Return a valid kind of Response. If not a response return NO_RESPONSE.
    // Throw ValidationError or ModeError in case of failure.
    CommandResponse isResponse(std::string const &buffer);

    // Check for a valid response in DATA mode only.
    // Used by isResponse().
    // Throw ValidationError or ModeError in case of failure.
    void validResponse(std::string const &buffer);


    // In DATA mode: +++AT:<length>:<notification><end-of-line>
    // IN COMMAND mode: <notification><end-of-line>
    // Return a valid kind of Notification. If not a notification return NO_NOTIFICATION.
    // Throw ValidationError or ModeError in case of failure.
    Notification isNotification(std::string const &buffer);

    // Check for a valid notification in DATA mode only.
    // Used by isNotification().
    // Throw ValidationError or ModeError in case of failure.
    void validNotification(std::string const &buffer);

    // Check for a valid notification both in DATA and COMMAND mode.
    // Used by isNotification().
    // Throw ValidationError in case of failure.
    void fullValidation(std::string const &buffer, Notification const &notification);


    // TODO. Check the best way to interpreted every kind of response and what it should return.
    std::string interpretResponse(std::string const &buffer, std::string const &command, CommandResponse const &response);
    // TODO. Check the best way to interpreted every kind of notification and what it should return.
    std::string interpretNotification(std::string const &buffer, Notification const &notification);

    // Return a filled command string to be sent to device.
    // Used by sendCommand().
    std::string fillCommand(std::string const &command);
    // Return a filled command with end-line, according interface type.
    // Used by fillComand().
    std::string addEndLine (std::string const &command);

    // Manage mode operation according command sent.
    // Act before get a response.
    // Used by sendCommand().
    // Return FALSE if command does not require response (ONLY case, AT0 - go to command mode). TRUE otherwise.
    WaitResponse modeManager(std::string const &command);
    // Manage mode operation according command sent and response obtained.
    // Act after get a response.
    // Used by interprateResponse().
    void modeMsgManager(std::string const &command);

protected:

    //Pure virtual function from Driver
    virtual int extractPacket(uint8_t const* buffer, size_t buffer_size) const;
};

}
#endif 
