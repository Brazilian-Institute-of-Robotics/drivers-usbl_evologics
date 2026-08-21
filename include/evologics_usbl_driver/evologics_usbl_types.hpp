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

#include <Eigen/Geometry>

#include <chrono>
#include <string>
#include <cstdint>
#include <vector>

namespace evologics_usbl_driver
{
const int BROADCAST = 255;

/**
 * @brief Type of interface with local device.
 *
 */
enum InterfaceType
{
    // Interface with modem/vehicle.
  SERIAL,
    // Interface with USBL/Dock/Boat
  ETHERNET
};

/**
 * @brief Type of operation.
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

/**
 * @brief Connection status of the acoustic connection to the remote device.
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

/**
 * @brief Kind of response to a command by local device.
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
    // No response.
  NO_RESPONSE
};

/**
 * @brief Notification that can be received.
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
    // Drop count notification.
  DROPCNT,
    // Extra notifications. See about Extended notification. Not implemented.
  EXTRA_NOTIFICATION,
    // No notification
  NO_NOTIFICATION
};

/**
 * @brief Delivery status of an instant message.
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

/**
 * @brief Reset device or clear buffer.
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

/**
 * @brief Firmware information
 *
 * Parameter for command ATIn
 * View firmware information
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

/**
 * @brief Sound Pressure Level (SPL) in transmission mode
 *
 * Default value is 3 (MINIMAL).
 * For test in air, use ONLY value MINIMAL.
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

/**
 * @brief Notification information
 *
 * Kind of notification and its content.
 */
struct NotificationInfo
{
  Notification notification;
  std::string buffer;
};

/**
 * @brief Response information
 *
 * Kind of response and its content.
 */
struct ResponseInfo
{
  CommandResponse response;
  std::string buffer;
};

/**
 * @brief Firmware information
 *
 */
struct VersionNumbers
{
    // Firmware version number
  std::string firmware_version;
    // Physical and data-layer protocol versions
  std::string accoustic_version;
    // Device manufacturer
  std::string manufacturer;
};

/**
 * @brief Configuration for acoustic connection
 *
 */
struct StatusRequest
{
  std::chrono::system_clock::time_point time;

    // Interpreter type. True: AT interpreter. False: NET interpreter
    // At: Standard, for DATA mode and COMMAND mode
    // NET: Not tested, for Networking command and COMMAND mode.
  bool at_interpreter;

    // Parameter of command interpreter
    // protocolID = 0, global device settings can be edited.
    // protocolID = 1...7, global device settings cannot be edit. Identifier for Instant Message communication
  int protocol_id;

    // Physical layer of local device
    // True: On. False: off
  bool physical;

    // Built-in battery voltage (in Volts). Can be external battery. Need validation
  double battery_voltage;
};

/**
 * @brief Connection status of acoustic link
 *
 */
struct AcousticConnection
{
  std::chrono::system_clock::time_point time;

    // Connection status
  ConnectionStatus status;

    // Free transmission buffer space (in bytes)
  std::vector<int> free_buffer;
};

/**
 * @brief Major device settings
 *
 */
struct DeviceSettings
{
    // True: low gain, reduced sensitivity. For short distance and test. 1
    // False: Normal gain, high sensitivity. 0
  bool low_gain;

    // Devices can just establish a connection with specific carrier Waveform ID combinations
    // The combinations are 0-1 and 2-2.
    // It's recommended to use 0-1 for a two devices connection and 2-2 for networking.
  int carrier_waveform_id;

    // Address of local device
  int local_address;

    // The Address of the remote device to transmit BurstData or Instant Messages.
    // If the Remote Address is 0 the device accepts every connection request, but can not initiate a connection.
    // remoteAddress of local device must match localAddress of remote device.
  int remote_address;

    // Define limits of devices in the network
    // Values: 2, 6, 14, 30, 62, 126, 254
  int highest_address;

    // The number of packets in one train. It's recommended to use a cluster size less then
    // 10 for moving objects. For stationary you can use a cluster size up to 32.
  int cluster_size;

    // Maximum duration of a data packet. From 50..1000 (in ms).
    // packetTime must be equal for all devices.
    // Short values are recommend for challenging hydroacoustic channels.
  int packet_time;

    // How many times the device will retry to connect
    // Retry count 0-255
  int retry_count;

    // Retry timeout 500-12000 ms
  int retry_timeout;

    // The timeout before closing an idle acoustic connection
    // 0-3600 s
  int idle_timeout;

    // Speed of sound 1300-1700 m/s
  int sound_speed;

    // Instant Message retry 0-255. 255 = retry indefinitely
  int im_retry;

    // False: Local device will only accept message addressed to it. 0
    // True: Receive message addressed to any device on network. 1
  bool promiscuos_mode;

    // Wake Up active time.
    // 0..3600 (s)
  int wu_active_time;

    // Wake Up period
    // 0..3600 (s)
  int wu_period;

    // Wake Up hold timeout
    // 0..3600 (s)
  int wu_hold_timeout;

    // Transmission buffer size (bytes) per channel.
    // 8096.. 2097152
  std::vector<int> pool_size;
};

/**
 * @brief Multipath propagation of acoustic signal, from transmitter to receiver.
 *
 * Geometry and reflection properties of underwater channel determine the number of
 * significant propagation path, the strengths and delays.
 */
struct MultiPath
{
    // Delay of path propagation (in us)
  int timeline;

    // Strengths of signal
  int signal_integrity;
};

/**
 * @brief Acoustic channel performance
 *
 */
struct AcousticChannel
{
  std::chrono::system_clock::time_point time;

    // Channel of current input-output interface
    // Data transferring among different channel is impossible.
    // 0..7
  int channel_number;

    // Dropped data from transmission buffer (bytes) of channelNumber, in bytes.
    // Cases: 1)ResetType; 2)idleTimeout; 3)transmission to remoteAddress 0
  int drop_count;

    // Current Overflow count of channelNumber, in bytes.
  int overflow_counter;

    // Local-2-Remote bitrate (bit/s)
  int local_bitrate;

    // Remote-2-Local bitrate (bit/s)
  int remote_bitrate;

    // RSSI (Received Signal Strength Indicator). Signal level in dB. Higher values correspond stronger signals.
    // Signal acceptable for rssi between -20dB and -85dB.
    // In NOISE state, rssi return RMS of noise. Communication performs best when rssi exceeds noise by 6dB
  double rssi;

    // Signal integrity represent distortions of signal.
    // Weak connection of integrityLevel < 100.
  int signal_integrity;

    // Propagation time between devices. Delay (im ms)
  int propagation_time;

    // Relative velocity between devices (in m/s)
  double relative_velocity;

  std::vector<MultiPath> multi_path;

    // Data sent to remote device.
  uint64_t sent_raw_data;
    // Data sent with receipt acknowledgment from remote side
    // Got from usbl. How to reset it is unknown.
  uint64_t delivered_raw_data;
    // Data received from remote device.
  uint64_t received_raw_data;
};

/**
 * @brief IN NOISE state
 *
 */
struct NoiseSample
{
  double noise;
  int size;
  double sample_rate;
  bool low_gain;
  double rssi;
};

/**
 * @brief Device specific position structure.
 *
 * To be independent.
 */
struct Position
{
  std::chrono::system_clock::time_point time;
  std::chrono::system_clock::time_point measurement_time;
  int remote_address;
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
  std::chrono::microseconds propagation_time;
  int rssi;
  int integrity;
    // Accuracy of the position fix, (in rad)
  double accuracy;
};

/**
 * @brief Pose of remote device, in local device's reference frame.
 *
 */
struct RigidBodyState
{
  std::chrono::system_clock::time_point time;
  Eigen::Vector3d position;
  Eigen::Quaterniond orientation;
};

/**
 * @brief Device specific direction structure.
 *
 * In case device is not able to provide position, it provides Direction
 */
struct Direction
{
  std::chrono::system_clock::time_point time;
  std::chrono::system_clock::time_point measurement_time;
  int remote_address;
    // Coordinates in local device's reference frame. (in rad)
  double local_bearing;
  double local_elevation;
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

/**
 * @brief Instant message to be sent
 *
 */
struct SendIM
{
  std::chrono::system_clock::time_point time;
  int destination;
  bool delivery_report;
  std::vector<uint8_t> buffer;
};

/**
 * @brief Received instant message
 *
 */
struct ReceiveIM
{
  std::chrono::system_clock::time_point time;
  int destination;
  int source;
  bool delivery_report;
  std::chrono::microseconds duration;
  int rssi;
  int integrity;
  double velocity;
  std::vector<uint8_t> buffer;
};

/**
 * @brief Message Delivery status
 *
 */
struct MessageStatus
{
  std::chrono::system_clock::time_point time;
  SendIM send_im;
  DeliveryStatus status;
    // The statement below should happen
    // messageSent = messageDelivered + messageFailed
    // Total of messages sent to remote device
  uint64_t message_sent;
    // Total of message successfully delivered
  uint64_t message_delivered;
    // Total of messages failed.
  uint64_t message_failed;
    // Total of messages received.
  uint64_t message_received;
    // Total of messages canceled.
  uint64_t message_canceled;
};
}  // namespace evologics_usbl_driver
