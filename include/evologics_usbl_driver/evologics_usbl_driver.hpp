// Copyright (c) 2026, SENAI Cimatec
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//              http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <cstdint>
#include <map>
#include <queue>
#include <string>
#include <vector>

#include <ros_driver_base/driver.hpp>

#include "evologics_usbl_driver/evologics_usbl_parser.hpp"
#include "evologics_usbl_driver/evologics_usbl_types.hpp"
#include "evologics_usbl_driver/evologics_usbl_exceptions.hpp"

namespace evologics_usbl_driver
{
/**
 * @brief Driver class for Evologics USBL devices, provides methods to communicate with the device, send commands,
 * receive responses, and handle notifications and raw data.
 *
 */
class EvologicsUsblDriver : public ros_driver_base::Driver
{
public:
  /**
   * @brief Construct a new Driver object with default constructor
   *
   */
  EvologicsUsblDriver();

  /**
   * @brief Construct a new Driver object passing the Operation Mode
   *
   * @param init_mode Initial operation mode
   */
  explicit EvologicsUsblDriver(const OperationMode & init_mode);

  /**
   * @brief Destroy the Driver object
   *
   */
  ~EvologicsUsblDriver();

  /**
   * @brief Define the interface with device. ETHERNET or SERIAL.
   *
   * @param device_interface ETHERNET or SERIAL
   */
  void setInterface(InterfaceType device_interface);

  /**
   * @brief Send a command to device.
   *
   * Fill buffer with necessary data.
   * Manage the mode of operation according to command.
   * @param command to be sent.
   */
  void sendCommand(const std::string & command);

  /**
   * @brief Send a command to device and wait for the corresponding OK
   *
   * @param command prefix
   * @param parameters command parameters, if any
   */
  void sendCommandAndACK(const std::string & command, const std::string & parameters = "");

  /**
   * @brief Send raw data to remote device.
   *
   * Use in DATA mode.
   * Doesn't require response.
   * @param raw_data string to be sent to remote device.
   *
   */
  void sendRawData(const std::vector<uint8_t> & raw_data);

  /**
   * @brief Read response from device.
   *
   * Read from device. Push Notification and Raw data in respective queue.
   * @result ReponseInfo. If incoming buffer is not a response, ResponseInfo.response = NO_RESPONSE.
   * ResponseInfo.response is the kind of response and ResponseInfo.buffer is the response.
   */
  ResponseInfo readResponse();

  /**
   * @brief Check if a Notification string is present in buffer.
   *
   * Auxiliary function of extractPacket()
   * To be used in COMMAND mode.
   * @param buffer to be analyzed
   * @return size of buffer till end of message, or -1 in case of no Notification.
   */
  int checkNotificationCommandMode(const std::string & buffer) const;

  /**
   * @brief Check if am Instant Message Notification string is present in buffer.
   *
   * Auxiliary function of checkNotificationCommandMode() and extractPacket()
   * To be used in COMMAND mode.
   * @param buffer to be analyzed
   * @return size of buffer till end of message, or -1 in case of no Notification.
   */
  int checkIMNotification(const std::string & buffer) const;

  /**
   * @brief Check the size of a particular response.
   *
   * Auxiliary function of extractPacket().
   * Response to command AT&V (getCurrentSetting) uses multiples '\r\n' and a final '\r\n\r\n'. Damn EvoLogics.
   * Maybe there are other command's responses that use the same pattern.
   * @param buffer to be analyzed
   * @return size of buffer till end of message.
   */
  int checkParticularResponse(const std::string & buffer) const;

  /**
   * @brief Check the size of regular response.
   *
   * Auxiliary function used by extractPacket().
   * Response and Notification end by a end-of-line '\r\n'.
   * @param buffer to be analyzed
   * @return size of buffer till end of message.
   */
  int checkRegularResponse(const std::string & buffer) const;

  /**
   * @brief Check kind of response.
   *
   * In DATA mode: +++<AT command>:<length>:<command response><end-of-line>
   * IN COMMAND mode: <response><end-of-line>
   * Throw ValidationError or ModeError in case of failure.
   * @param buffer to be analyzed
   * @return CommandResponse kind of response. If is not a response, returns NO_RESPONSE.
   */
  CommandResponse isResponse(const std::string & buffer);

  /**
   * @brief Check kind of notification.
   *
   * In DATA mode: +++AT:<length>:<notification><end-of-line>
   * IN COMMAND mode: <notification><end-of-line>
   * Throw ValidationError or ModeError in case of failure.
   * @param buffer to be analyzed.
   * @return Notification kind. If is not a notification, returns NO_NOTIFICATION.
   */
  Notification isNotification(const std::string & buffer);

  /**
   * @brief Wait for a response from device.
   *
   * @param expected_prefix command prefix
   * @param command sent to device
   * @param expected kind of response expected
   * @param ignore_unexpected_responses if true, unexpected responses will be ignored and the function will
   * keep waiting for the expected response. If false, an exception will be thrown on unexpected responses.
   * @return string containing the response buffer.
   */
  std::string waitResponse(
    const std::string & expected_prefix, const std::string & command, CommandResponse expected,
    bool ignore_unexpected_responses = false);

  /**
   * @brief Wait for a OK response.
   *
   * @param expected_prefix command prefix
   * @param command sent to device.
   */
  void waitResponseOK(const std::string & expected_prefix, const std::string & command);

  /**
   * @brief Wait for a integer response.
   *
   * @param expected_prefix command prefix
   * @param command sent to device.
   * @return integer requested.
   */
  int waitResponseInt(const std::string & expected_prefix, const std::string & command);

  /**
   * @brief Wait for a floating point response.
   *
   * @param expected_prefix command prefix
   * @param command sent to device.
   * @return double requested.
   */
  double waitResponseDouble(const std::string & expected_prefix, const std::string & command);

  /**
   * @brief Wait for a integer (that may be very long) response.
   *
   * @param expected_prefix command prefix
   * @param command sent to device.
   * @return uint64_teger requested.
   */
  uint64_t waitResponseULLongInt(const std::string & expected_prefix, const std::string & command);

  /**
   * @brief Wait for string response.
   *
   * @param expected_prefix command prefix
   * @param command sent to device.
   * @return string requested.
   */
  std::string waitResponseString(const std::string & expected_prefix, const std::string & command);

  /**
   * @brief Get Underwater Connection Status.
   *
   * @return connection status
   */
  AcousticConnection getConnectionStatus();

  /**
   * @brief Get Current Setting parameters.
   *
   * @return current DeviceSettings
   */
  DeviceSettings getCurrentSetting();

  /**
   * @brief get Instant Message Delivery status.
   *
   * @return delivery status
   */
  DeliveryStatus getIMDeliveryStatus();

  /**
   * @brief Get the Instant Message Delivery Report from a notification buffer.
   * @param buffer the notification buffer
   * @return the delivery status
   */
  DeliveryStatus getIMDeliveryReport(const std::string & buffer);

  /**
   * @brief Switch to COMMAND mode using Guard Time Escape Sequence (GTES).
   *
   */
  void GTES();

  /**
   * @brief Switch to COMMAND mode, no need to wait for guard time.
   *
   */
  void switchToCommandMode();

  /**
   * @brief Switch to DATA mode, no need to wait for guard time.
   *
   */
  void switchToDataMode();

  /**
   * @brief Reset device, drop data and/or instant message
   *
   * @param type define what will be reset in device
   * @param ignore_unexpected_responses if true, unexpected responses will be ignored and the function will
   * keep waiting for the expected response. If false, an exception will be thrown on unexpected
   */
  void resetDevice(const ResetType & type, bool ignore_unexpected_responses = false);

  /**
   * @brief Get interface type.
   *
   * @return interface type. SERIAL or ETHERNET.
   */
  InterfaceType getInterface();

  /**
   * @brief Send Instant Message to remote device.
   *
   * @param im Instant Message to be sent.
   */
  void sendInstantMessage(const SendIM & im);

  /**
   * @brief Get Instant Message parsed as string
   *
   * @param im Instant Message to be sent.
   * @return string parsed of im.
   */
  std::string getStringOfIM(const SendIM & im);

  /**
   * @brief Parse a received Instant Message.
   *
   * @param buffer that contains the IM
   * @return Received Instant Message
   */
  ReceiveIM receiveInstantMessage(const std::string & buffer);

  /**
   * @brief Get the RigidBodyState pose of remote device.
   *
   * Only used by devices with ETHERNET interface.
   * Convert the data from internal struct to RigidBodyState.
   * @return RigidBodyState pose.
   */
  RigidBodyState getPose(const Position & pose);

  /**
   * @brief Get the Position pose of remote device.
   *
   * Only used by devices with ETHERNET interface.
   * Convert the received buffer from USBLLONG notification to Position.
   * It may have some data of interest.
   * @return Position pose.
   */
  Position getPose(const std::string & buffer);

  /**
   * @brief Get the Direction of remote device.
   *
   * Only used by devices with ETHERNET interface, in case the device is not able to compute the pose.
   * Convert the received buffer from USBLANGLE notification to Direction.
   * It may have some data of interest.
   * @return Direction direction.
   */
  Direction getDirection(const std::string & buffer);

  /**
   * @brief Converts from euler angles to quaternions.
   *
   * euler = [roll, pitch, yaw]
   * @param euler_angles - Euler angles vector
   * @return quaternion - Quaternion variable
   */
  Eigen::Quaterniond eulerToQuaternion(const Eigen::Vector3d & euler_angles);

  /**
   * @brief Helper method to separate AT and raw packets in a data stream
   *
   * @param buffer - The data stream to be processed
   * @return The index of the first raw packet in the buffer
   */
  int extractRawFromATPackets(const std::string & buffer) const;

  /**
   * @brief Helper method to extract packets from raw data
   *
   * This is to be reimplemented in subclasses if the raw data
   * has a packet-based protocol. The default implementation will
   * just interpret any amount of raw data as a packet
   */
  virtual int extractRawDataPacket(const std::string & buffer) const;

  /**
   * @brief Given a buffer that starts with a TIE header (+++), return whether it could be a AT command
   *
   * @param buffer to be analyzed
   * @return size of buffer till end of message, or -1 in case of no AT command.
   */
  int extractATPacket(const std::string & buffer) const;

  /**
   * @brief Pop out RawData from queueRawData.
   *
   * @return string of raw data
   */
  std::vector<uint8_t> getRawData();

  /**
   * @brief Verify if queueRawData has raw data.
   *
   * @return TRUE if queue has raw data, FALSE otherwise.
   */
  bool hasRawData();

  /**
   * @brief Pop out Notification from queueNotification.
   *
   * @return NotificationInfo
   */
  NotificationInfo getNotification();

  /**
   * @brief Verify if queueNotification has any notification.
   *
   * @return TRUE if queue has notification, FALSE otherwise.
   */
  bool hasNotification();

  /**
   * @brief Get mode of operation.
   *
   * DATA or COMMAND
   * @return Operation mode.
   */
  OperationMode getMode();

  /**
   * @brief Set the specific carrier Waveform ID
   *
   * Devices can just establish a connection with specific carrier Waveform ID combinations
   * The combinations are 0-1 and 2-2.
   * It's recommended to use 0-1 for a two devices connection and 2-2 for networking.
   * @param value of carrier waveform
   */
  void setCarrierWaveformID(int value);

  /**
   * @brief Set number of packets in one train.
   *
   * It's recommended to use a cluster size less then 10 for moving objects.
   * For stationary you can use a cluster size up to 32.
   * @param value
   */
  void setClusterSize(int value);

  /**
   * @brief Set limits of devices in the network.
   *
   * @param value: 2, 6, 14, 30, 62, 126, 254
   */
  void setHighestAddress(int value);

  /**
   * @brief Set timeout before closing an idle acoustic connection
   *
   * @param value in seconds (0-3600 s)
   */
  void setIdleTimeout(int value);

  /**
   * @brief Set Instant Message retry count
   *
   * Range 0-255. 255 = retry indefinitely
   * @param value
   */
  void setIMRetry(int value);

  /**
   * @brief Set address of local device
   *
   * 1-highest_address
   * @param value
   */
  void setLocalAddress(int value);

  /**
   * @brief Set address of remote device
   *
   * 0-highest_address
   * @param value
   */
  void setRemoteAddress(int value);

  /**
   * @brief Get address of remote device
   *
   * 0-highest_address
   * @return address
   */
  int getRemoteAddress();

  /**
   * @brief Get highest address
   *
   * @return highest address
   */
  int getHighestAddress();

  /**
   * @brief Automatic positioning output
   *
   * @return 0 fro disable, 1 for enable
   */
  int getPositioningDataOutput();

  /**
   * @brief Enable or disable automatic positioning output
   *
   * TRUE: Enable automatic position output
   * FALSE: Disable
   * @param pose_on
   */
  void setPositioningDataOutput(bool pose_on);

  /**
   * @brief Set input amplifier gain
   *
   * Low gain recommended for short-distance communication or test.
   * TRUE low gain applied.
   * FALSE normal gain applied.
   * @param low_gain
   */
  void setLowGain(bool low_gain);

  /**
   * @brief Set maximum duration of a data packet.
   *
   * Packet Time MUST be equal for all devices.
   * Range 50..1000 (in ms).
   * Short values are recommend for challenging hydroacoustic channels.
   * @param value maximum duration of a data packet.
   */
  void setPacketTime(int value);

  /**
   * @brief Set if device will receive instant message addressed to others devices.
   *
   *  FALSE: Local device will only accept message addressed to it. 0
   *  TRUE: Receive message addressed to any device on network. 1
   *  @param promiscuos_mode
   */
  void setPromiscuosMode(bool promiscuos_mode);

  /**
   * @brief Set number of connection establishment retries.
   *
   * @param value number of connection retries
   */
  void setRetryCount(int value);

  /**
   * @brief Set time of wait for establish an acoustic connection
   *
   * retry timeout should exceed the round-trip time that corresponds to the device's
   * maximum operation range.
   * Range 500..12000 (in ms)
   * @param value in ms
   */
  void setRetryTimeout(int value);

  /**
   * @brief Set Source Level
   *
   * Defines Sound Pressure Level (SPL)
   * @param source_level
   */
  void setSourceLevel(SourceLevel source_level);

  /**
   * @brief Set if source level of local device can be changed remotely over a acoustic connection.
   *
   * TRUE: local sourceLevel can be changed by remote device.
   * Automatically change for source level of remote device during connection. 1
   * FALSE: local sourceLevel cannot be changed by remote device. 0
   * @param source_level_control
   */
  void setSourceLevelControl(bool source_level_control);

  /**
   * @brief Get source level of device
   *
   * @return source level
   */
  SourceLevel getSourceLevel();

  /**
   * @brief Get source level control of device
   *
   * True: local sourceLevel can be changed by remote device.
   *   Matching of sourceLevel during connection. 1
   * False: local sourceLevel cannot be changed by remote device. 0
   * @return source level control
   */
  bool getSourceLevelControl();

  /**
   * @brief Set speed of sound on water
   *
   * Range 1300..1700 m/s
   * @param value in m/s
   */
  void setSpeedSound(int value);

  /**
   * @brief Set active interval of acoustic channel monitoring.
   *
   * Command effect only on devices with the Wake Up Module installed.
   * Range: 0..3600 (in s)
   * NOTE: MUST be less than the total duration of the Wake Up cycle.
   * @param value in s
   */
  void setWakeUpActiveTime(int value);

  /**
   * @brief Set hold timeout after completed data transmission.
   *
   * Command effect only on devices with the Wake Up Module installed.
   * Range: 0..3600 (in s)
   * @param value in s
   */
  void setWakeUpHoldTimeout(int value);

  /**
   * @brief Set period of the acoustic channel monitoring cycle.
   *
   * Comprises an active interval and an idle interval.
   * Command effect only on devices with the Wake Up Module installed.
   * Range: 0..3600 (in s)
   * @param value in s
   */
  void setWakeUpPeriod(int value);

  /**
   * @brief Set transmission buffer size of actual data channel.
   *
   *  Before changing buffer size, the buffer will empty.
   *  Range: 8096..2097152
   *  @param value bufer size in bytes
   */
  void setPoolSize(int value);

  /**
   * @brief Reset Drop Counter.
   *
   */
  void resetDropCounter();

  /**
   * @brief Reset Overflow Counter.
   *
   */
  void resetOverflowCounter();

  /**
   * @brief Get firmware information of device.
   *
   * @return VersionNumbers
   */
  VersionNumbers getFirmwareInformation();

  /**
   * @brief Get last transmission's raw bitrate value of local-to-remote direction.
   *
   * Include both useful data (raw data) and the protocol overhead.
   * @return bitrate in bits per second.
   */
  int getLocalToRemoteBitrate();

  /**
   * @brief Get last transmission's raw bitrate value of remote-to-local direction.
   *
   * Include both useful data (raw data) and the protocol overhead.
   * @return bitrate in bits per second.
   */
  int getRemoteToLocalBitrate();

  /**
   * @brief Get Received Signal Strength Indicator.
   *
   * Indicates the received signal level in dB re 1 V and represents the relative received signal strength.
   * Higher RSSI values correspond to stronger signals.
   * Signal strength is acceptable when measured RSSI values lie between -20dB and -85dB.
   * In NOISE state, return RMS of the noise. RSSI of communication should exceeds the noise by 6dB.
   * @return rssi in dB.
   */
  double getRSSI();

  /**
   * @brief Get Signal Integrity.
   *
   * Illustrate distortion of last acoustic signal.
   * High Signal Integrity Level values correspond to less distortion signals.
   * An acoustic link is weak if value is less than 100.
   * @return signal integrity level.
   */
  int getSignalIntegrity();

  /**
   * @brief Get acoustic signal's propagation time between communicating devices.
   *
   * @return propagation time in ms
   */
  int getPropagationTime();

  /**
   * @brief Get relative velocity between communicating devices.
   *
   * @return relative velocity in m/s
   */
  double getRelativeVelocity();

  /**
   * @brief Get Multipath propagation structure.
   *
   * @return Multipath components
   */
  std::vector<MultiPath> getMultipath();

  /**
   * @brief Get dropCounter of actual channel
   *
   * @return value in bytes
   */
  int getDropCounter();

  /**
   * @brief Get overflowCounter of actual channel
   *
   * @return value in bytes
   */
  int getOverflowCounter();

  /**
   * @brief Get channel number of current interface.
   *
   * @return value in bytes
   */
  int getChannelNumber();

  /**
   * @brief Get overall delivered raw data
   *
   * Usbl documentation doesn't say the max size neither a way to reset it, so a ullong_int was chosen.
   * @return counter of raw data bytes delivered to remote device.
   */
  uint64_t getRawDataDeliveryCounter();

  /**
   * @brief Set System Time for current time
   *
   * Default System Time value is the number of seconds elapsed since the device has been powered on.
   * Is possible to syncronize the System Time with a Network Time Protocol (NTP) server. Not implemented.
   */
  void setSystemTimeNow();

  /**
   * @brief Set operation mode of device
   *
   * Operation mode impact in
   * DATA, all data is interpreted as raw_data. Command are send through TIES string.
   * COMMAND, all data is interpreted as command. Raw_data is NOT transmitted.
   * @param mode, DATA or COMMAND mode
   */
  void setOperationMode(OperationMode const & new_mode);

  /**
   * @brief Store current setting profile
   *
   */
  void storeCurrentSettings();

  /**
   * @brief Restore factory settings and reset device.
   *
   */
  void RestoreFactorySettings();

  /**
   * @brief Get communication parameters
   *
   *  @return AcousticChannel with performance.
   */
  AcousticChannel getAcousticChannelparameters();

  /**
   * @brief Update parameters on device.
   *
   * Compare actual with desired settings before update.
   * Source level and source level control are not set here. They must be set in component according device's placement.
   * @param desired_setting Parameters that should be applied on device.
   * @param actual_setting Parameters present in device that will be used for compare.
   */
  void updateDeviceParameters(const DeviceSettings & desired_setting, const DeviceSettings & actual_setting);

protected:
  /**
   *   @brief Implementation of the virtual function from Driver
   *   That function will split the packet and analyze it
   *     There are four possible cases to return:
   *     - there is no packet in the buffer. In that case, return -buffer_size
   *       to discard all the data that has been gathered until now.
   *     - there is the beginning of a packet but it is not starting at the
   *       first byte of \c buffer. In that case, return -position_packet_start,
   *       where position_packet_start is the position of the packet in \c
   *       buffer.
   *     - a packet begins at the first byte of \c buffer, but the end of the
   *       packet is not in \c buffer yet. Return 0.
   *     - there is a full packet in \c buffer, starting at the first buffer
   *       byte. Return the packet size. That data will be copied back to the
   *       buffer given to readPacket.
   *   @param buffer a pointer to the buffer data
   *   @param buffer_size size_t with the size of the passed buffer
   *   @return an integer value related to the four possible listed before
   */
  [[nodiscard]] int extractPacket(uint8_t const * buffer, size_t buffer_size) const override;

private:
  /**
   * @brief Read packets
   *
   * @return string with data (response, notification or raw data).
   */
  std::string readInternal();

  /**
   * @brief Check a valid notification.
   *
   * Used by isNotification().
   * Can be used in DATA or COMMAND mode.
   * Throw ValidationError in case of failure.
   * @param buffer to be analyzed.
   * @param notification kind present in buffer.
   */
  void notificationValidation(std::string const & buffer, Notification const & notification);

  /**
   * @brief Filled command string to be sent to device.
   *
   * Used by sendCommand().
   * @param command to be sent.
   * @return string filled.
   */
  std::string fillCommand(std::string const & command);

  /**
   * @brief Add a end line, according interface type.
   *
   * Used by fillCommand().
   * SERIAL, <end-line> = '\r'.
   * ETHERNET, <end-line> = '\n'.
   * @param command to be sent.
   * @return string command with end line.
   */
  std::string addEndLine(std::string const & command);

  /**
   * @brief Manage mode operation according command sent.
   *
   * Act before get a response.
   * @param command sent to device.
   */
  void modeManager(std::string const & command);

  /**
   * @brief Manage mode operation according command sent and response obtained.
   *
   * Act after get a response.
   * @param command sent to device.
   */
  void modeMsgManager(std::string const & command);

  /**
   * @brief Parser for USBL messages
   *
   */
  UsblParser usbl_parser_;

  /**
   * @brief Current operation mode of the device (DATA or COMMAND)
   *
   */
  OperationMode mode_;

  /**
   * @brief Interface type of the device (SERIAL or ETHERNET)
   *
   */
  InterfaceType interface_;

  /**
   * @brief Queue of received raw data
   *
   */
  std::queue<std::string> queue_raw_data_;

  /**
   * @brief Queue of received notifications
   *
   */
  std::queue<NotificationInfo> queue_notification_;

  /**
   * @brief Maximum size of a packet for reading from the device
   *
   */
  static constexpr int kMaxPacketSize{20000};
};
}  // namespace evologics_usbl_driver
