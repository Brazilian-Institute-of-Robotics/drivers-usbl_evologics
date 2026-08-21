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

#include <chrono>
#include <iostream>
#include <sstream>

#include "evologics_usbl_driver/evologics_usbl_driver.hpp"

namespace evologics_usbl_driver
{

EvologicsUsblDriver::EvologicsUsblDriver(const OperationMode & init_mode)
: ros_driver_base::Driver(kMaxPacketSize), mode_(init_mode)
{
}

EvologicsUsblDriver::EvologicsUsblDriver()
: EvologicsUsblDriver(DATA)
{
}

EvologicsUsblDriver::~EvologicsUsblDriver()
{
}

// Send a command to device.
void EvologicsUsblDriver::sendCommand(const std::string & command)
{
    // Fill buffer (header and end-line) according operational mode and interface type.
  std::string buffer = fillCommand(command);
  writePacket(reinterpret_cast<const uint8_t *>(buffer.c_str()), buffer.length());

    // Manage the operational mode, DATA or COMMAND, according to command.
  modeManager(command);
}

// Send raw data to remote device.
void EvologicsUsblDriver::sendRawData(const std::vector<uint8_t> & raw_data)
{
  writePacket(raw_data.data(), raw_data.size());
}

// Filled command string to be sent to device.
std::string EvologicsUsblDriver::fillCommand(const std::string & command)
{
  std::stringstream ss;
    // Guard Time Escape Sequence (GTES) doesn't require <end-line>
    // GTES: (1s)<+++>(1s)
  if(command == "+++") {
    ss << command << std::flush;
    if(mode_ == DATA) {
      sleep(1);
    }
  } else {
    if (mode_ == DATA) {
            // If in DATA, buffer = +++<AT command>
      ss << "+++" << std::flush;
    }
    ss << addEndLine(command) << std::flush;
  }
  return ss.str();
}

// Add a end line, according interface type.
std::string EvologicsUsblDriver::addEndLine(const std::string & command)
{
  std::stringstream ss;
  if (interface_ == SERIAL) {
    ss << command << "\r" << std::flush;
  } else {
    ss << command << "\n" << std::flush;
  }
  return ss.str();
}

// Extract a raw data packet.
int EvologicsUsblDriver::extractRawDataPacket(const std::string & buffer) const
{
  return buffer.size();
}


// Check the size of a particular response.
int EvologicsUsblDriver::checkParticularResponse(const std::string & buffer) const
{
  std::string::size_type eol = buffer.find("\r\n\r\n");
  if(eol != std::string::npos) {
        // Add \r\n\r\n to buffer.
    return eol + 4;
  } else if(buffer.size() > 400) {
    throw std::runtime_error("CheckParticularResponse: Received a too big buffer of size \"" +
        std::to_string(buffer.size()) +
        "\". Check buffer \"" + usbl_parser_.printBuffer(buffer) + "\"");
  }
  return 0;
}

// Check the size of regular response.
int EvologicsUsblDriver::checkRegularResponse(const std::string & buffer) const
{
    // Find <end-of-line>
  std::string::size_type eol = buffer.find("\r\n");
  if(eol != std::string::npos) {
        // Add \r\n to buffer.
    return eol + 2;
  } else if(buffer.size() > 150) {
    throw std::runtime_error("CheckRegularResponse: Received a too big buffer of size \"" +
        std::to_string(buffer.size()) +
        "\". Check buffer \"" + usbl_parser_.printBuffer(buffer) + "\"");
  }
  return 0;
}

// Extract an AT packet.
int EvologicsUsblDriver::extractATPacket(const std::string & buffer) const
{
    // Smallest packet possible is +++AT:0:\r\n
  if (buffer.size() < 10) {
    return 0;
  }
  if (buffer.substr(0, 5) != "+++AT") {
    throw std::runtime_error("extractATPacket: Buffer does not start with \"+++AT\". Check buffer, \"" +
        usbl_parser_.printBuffer(buffer) + "\"");
  }
    // Get length.
    // +++<AT command>:<length>:<command response><end-of-line>
    // +++AT:<length>:<notification><end-of-line>
  std::string::size_type npos = std::string::npos;
  std::string::size_type length = 0;
  if ((npos = buffer.find(":")) != std::string::npos) {
    length += npos;
    std::string msg = buffer;
    msg = msg.substr(npos + 1, msg.size() - npos);
    if ((npos = msg.find(":")) != std::string::npos) {
      length += npos;
            // add 2 colon ":".
      length += 2;
            // Convert <length> to integer
      length += stoi(msg.substr(0, npos), &npos);
            // add <end-of-line>
      length += 2;
      if(length > buffer.size()) {
        return 0;
      }
            // Check the presence of end-of-line (\r\n).
      if (buffer.substr(length - 2, 2) != "\r\n") {
        throw std::runtime_error("extractATPacket: Could not find <end-of-line> at position \"" +
            std::to_string((length - 2)) +
            "\" of the end of buffer  \"" + usbl_parser_.printBuffer(buffer) + "\"");
      }

      return length;
    } else if (buffer.size() > 16) {
      throw std::runtime_error(
          "extractATPacket: Assuming max lenght of 999, could not find second \":\" before 16 bytes in buffer, \"" +
          usbl_parser_.printBuffer(buffer) + "\"");
    }
    return 0;
  } else if (buffer.size() > 12) {
    throw std::runtime_error("extractATPacket: Could not find any \":\" before 12 bytes in buffer \"" +
        usbl_parser_.printBuffer(buffer) + "\"");
  }
  return 0;
}

int EvologicsUsblDriver::extractRawFromATPackets(const std::string & buffer) const
{
    // TIES: Time Independent Escape Sequence
  const char * TIES_HEADER = "+++AT";
  const int   TIES_HEADER_SIZE = 5;

  std::string::size_type buffer_size = buffer.size();
  std::string::size_type ties_start = 0;
  while (ties_start < buffer_size) {
        // Look for the start of the first character of a TIES header
    while (ties_start < buffer_size && buffer[ties_start] != TIES_HEADER[0]) {
      ties_start++;
    }

        // Check whether we actually have a header. If there's enough space left in the
        // buffer, it has to be exactly like a full header
    if (ties_start + TIES_HEADER_SIZE < buffer_size) {
      if (buffer.substr(ties_start, TIES_HEADER_SIZE) == TIES_HEADER) {
        if (ties_start == 0) {
          return extractATPacket(buffer);
        } else {
          return extractRawDataPacket(buffer.substr(0, ties_start));
        }
      }
    } else if (std::string(TIES_HEADER, buffer_size - ties_start) == buffer.substr(ties_start,
        buffer_size - ties_start))
    {
            // Here, we have something that might be the start of a TIES header at the end of the buffer
      return extractRawDataPacket(buffer.substr(0, ties_start));
    }
    ++ties_start;
  }
  return extractRawDataPacket(buffer.substr(0, ties_start));
}

// Extract a packet from the buffer.
int EvologicsUsblDriver::extractPacket(const uint8_t * buffer, size_t buffer_size) const
{
  std::string buffer_as_string = std::string(reinterpret_cast<char const *>(buffer), buffer_size);

    // Both COMMAND and DATA mode, answer finish by \r\n.
  if(mode_ == DATA) {
    return extractRawFromATPackets(buffer_as_string);
  } else {  // Check for a single number as response.
    if(buffer_as_string.size() < 4) {
      return checkRegularResponse(buffer_as_string);
    }
        // Check for Notification
    int ret = checkNotificationCommandMode(buffer_as_string);
    if(ret >= 0) {
      return ret;
    }
        // Second, check for particular Response.
    if(buffer_as_string.find("Sour") != std::string::npos) {
      return checkParticularResponse(buffer_as_string);
    }
        // Last, check for regular Response.
    return checkRegularResponse(buffer_as_string);
  }
}

// Read response from device.
ResponseInfo EvologicsUsblDriver::readResponse()
{
  Notification notification;
  CommandResponse response;
  ResponseInfo response_info;
  response_info.response = NO_RESPONSE;

    // Get string from device.
  std::string buffer_as_string = readInternal();
    // Check for Notification and enqueue data.
  if((notification = isNotification(buffer_as_string)) != NO_NOTIFICATION) {
        // interpretNotification(buffer_as_string, notification);
    NotificationInfo notification_info;
    notification_info.notification = notification;
    notification_info.buffer = buffer_as_string;
    queue_notification_.push(notification_info);
    return response_info;
  } else if((response = isResponse(buffer_as_string)) != NO_RESPONSE) {
    ResponseInfo response_info;
    response_info.response = response;
    response_info.buffer = buffer_as_string;
    return response_info;
  }
    // If data is neither notification or response, it's raw data.
  queue_raw_data_.push(buffer_as_string);
  return response_info;
}


// Read packets.
std::string EvologicsUsblDriver::readInternal()
{
  std::vector<uint8_t> read_buffer;
  read_buffer.resize(kMaxPacketSize);
  int readpacket = readPacket(&read_buffer[0], kMaxPacketSize);

  return std::string(reinterpret_cast<char const *>(&read_buffer[0]), readpacket);
}

// Read input data till get a response.
std::string EvologicsUsblDriver::waitResponse(
  const std::string & expected_prefix, const std::string & command, CommandResponse expected,
  bool ignore_unexpected_responses)
{
  ResponseInfo response_info;
  response_info.response = NO_RESPONSE;
  auto init_time = std::chrono::system_clock::now();
    // 1 second time-out. Arbitrary value.
  auto time_out = std::chrono::seconds(1);
  auto time_now = std::chrono::system_clock::now();

  while(response_info.response != expected && (time_now - init_time) <= time_out) {
        // Read till get expected response.
    response_info = readResponse();
    time_now = std::chrono::system_clock::now();

    if (response_info.response == NO_RESPONSE) {
      continue;
    } else if (mode_ == DATA && !expected_prefix.empty() && (response_info.buffer.substr(3,
        expected_prefix.size()) != expected_prefix))
    {
      if (ignore_unexpected_responses) {
        continue;
      } else {
        throw DeviceError("USBL Driver.cpp waitResponse: Expected response " +
            usbl_parser_.printBuffer(response_info.buffer) + " to start with " + expected_prefix);
      }
    } else if(response_info.response == ERROR) {
      throw DeviceError("USBL Driver.cpp waitResponse: For the command: \"" + usbl_parser_.printBuffer(command) +
          "\", device return the follow ERROR msg: \"" + usbl_parser_.printBuffer(response_info.buffer) + "\"");
    } else if(response_info.response == BUSY) {
      throw BusyError("USBL Driver.cpp waitResponse: For the command: \"" + usbl_parser_.printBuffer(command) +
          "\", device return the follow BUSY msg: \"" + usbl_parser_.printBuffer(response_info.buffer) +
          "\". Try it latter.");
    }
  }
    // Check for time_out
  if(response_info.response != expected) {
    throw std::runtime_error("USBL Driver.cpp waitResponse: For the command: \"" + usbl_parser_.printBuffer(command) +
        "\", device didn't send a response in " +
        std::to_string(std::chrono::duration_cast<std::chrono::seconds>(time_out).count()) + " seconds time-out");
  }

    // In DATA mode, validate response and return content without header.
  if(mode_ == DATA) { // Buffer validation of indicated length was displaced for extract packet.
                      // No need to do it here again.
                      // Check if response and command match.
    return usbl_parser_.getAnswerContent(response_info.buffer, command);
  }
  return response_info.buffer;
}

// Wait for a OK response.
void EvologicsUsblDriver::waitResponseOK(const std::string & expected_prefix, const std::string & command)
{
  waitResponse(expected_prefix, command, COMMAND_RECEIVED);
}

// Wait for a integer response.
int EvologicsUsblDriver::waitResponseInt(const std::string & expected_prefix, const std::string & command)
{
  return usbl_parser_.getNumber(waitResponseString(expected_prefix, command));
}

// Wait for a floating point response.
double EvologicsUsblDriver::waitResponseDouble(const std::string & expected_prefix, const std::string & command)
{
  return usbl_parser_.getDouble(waitResponseString(expected_prefix, command));
}

// Wait for a integer (that may be very long) response.
uint64_t EvologicsUsblDriver::waitResponseULLongInt(
  const std::string & expected_prefix,
  const std::string & command)
{
  return usbl_parser_.getULLongInt(waitResponseString(expected_prefix, command));
}

// Wait for string response.
std::string EvologicsUsblDriver::waitResponseString(const std::string & expected_prefix, const std::string & command)
{
  return waitResponse(expected_prefix, command, VALUE_REQUESTED);
}

// Check if a Notification string is present in buffer.
int EvologicsUsblDriver::checkNotificationCommandMode(const std::string & buffer) const
{
  if (buffer.size() < 4) {
    return 0;
  }

    // All 4 letters notification
    // If Notification is a Received Message, the message part of string may contain malicious characters.
    // RECVxxx,<length>,<source address>,<destination address>,...<data><end-line>
    // <length> is size of <data>
  if (buffer.substr(0, 4).find("RECV") != std::string::npos) {
    if(buffer.size() < 7) {
      return 0;
    }
    Notification notification = usbl_parser_.findNotification(buffer);
    if(notification == RECVIM ||
      notification == RECVIMS ||
      notification == RECVPBM)
    {
      return checkIMNotification(buffer);
    } else {
            // It is a extra notification that starts with RECV (RECVSTART, RECVEND, ...)
      return checkRegularResponse(buffer);
    }
  } else if(buffer.find("DELI") != std::string::npos ||
    buffer.find("FAIL") != std::string::npos ||
    buffer.find("CANC") != std::string::npos ||
    buffer.find("EXPI") != std::string::npos ||
    buffer.find("SEND") != std::string::npos ||
    buffer.find("USBL") != std::string::npos ||
    buffer.find("BITR") != std::string::npos ||
    buffer.find("SRCL") != std::string::npos ||
    buffer.find("PHYO") != std::string::npos ||
    buffer.find("DROP") != std::string::npos ||
    buffer.find("RADD") != std::string::npos)
  {
    return checkRegularResponse(buffer);
  } else {
    return -1;
  }
}

// Check if am Instant Message Notification string is present in buffer.
int EvologicsUsblDriver::checkIMNotification(const std::string & buffer) const
{
  if(buffer.size() < 7) {
    return 0;
  }
  Notification notification = usbl_parser_.findNotification(buffer);
  if(notification != RECVIM && notification != RECVIMS && notification != RECVPBM) {
    throw std::runtime_error("usbl Driver.cpp checkIMNotification: Notification should be a message, instead got \"" +
        usbl_parser_.printBuffer(buffer) + "\"");
  }

    // Get the amount of comma expected for a notification
  int ncomma = usbl_parser_.getNumberFields(notification) - 1;
  std::string::size_type npos = std::string::npos;
  std::string::size_type comma_1;
  size_t length = 0;
  size_t size_buffer = 0;
  for(int i = 0; i < ncomma; i++) {
        // A comma in the last character of buffer. Wait for more data.
    if(size_buffer == buffer.size()) {
      return 0;
    }
        // No more comma from here in buffer. Wait for more.
    if((npos = buffer.substr(size_buffer, buffer.size() - size_buffer).find(",")) == std::string::npos) {
            // The biggest fields in notification is a 32bits timestamp, which max value of 2^32 has 10 digits
      if((buffer.size() - size_buffer) > 10) {
        return -1;
      }
      return 0;
    }
        // Increase the correct part of buffer size
    size_buffer += (npos + 1);
        // Get the first comma that encapsulate <length>
    if(i == 0) {
      comma_1 = npos;
    }
        // Get length with the second comma that encapsulate <length>
    if(i == 1) {
      length += stoi(buffer.substr(comma_1 + 1, npos - comma_1), &npos);
    }
  }
    // Buffer should have the size of the last comma plus the length of <data> and <end-line>
  length += (size_buffer + 2);
  if(length > buffer.size()) {
    std::cerr << "Size Error. Found length " << length << " doesn't match with buffer size of " << buffer.c_str() <<
      ". Waiting more data in buffer " << std::endl;
    return 0;
  }
    // Check for the <end-line>
  if(buffer.substr(length - 2, 2) != "\r\n") {
    throw std::runtime_error("Could not find <end-of-line> at position \"" + std::to_string(buffer.size() - 3) +
        "\"of the end of buffer  \"" + usbl_parser_.printBuffer(buffer) + "\"");
  }
  return length;
}

// Check kind of notification.
Notification EvologicsUsblDriver::isNotification(const std::string & buffer)
{
  if(buffer.size() < 4) {
    return NO_NOTIFICATION;
  }
  std::string content;
  if(mode_ == DATA) {
        // in DATA mode +++AT:<length>:notification\r\n
    if(buffer.substr(0, 6) != "+++AT:") {
      return NO_NOTIFICATION;
    }

        // Get content of Notification.
    content = usbl_parser_.splitMinimalValidate(buffer, ":", 3)[2];
  } else {
    content = buffer;
  }
    // If buffer is Message Notification , the message part of buffer may contains malicious data.
    // If notification is a received Message, First 4 letters of Notification string are 'RECV'
  if(content.substr(0, 4) == "RECV") {
        // Notification is a Received Message, so there is a comma after notification string
    content = usbl_parser_.splitMinimalValidate(content, ",", 2)[0];
  }

  Notification notification = usbl_parser_.findNotification(content);

  if(notification != NO_NOTIFICATION) {
        // Buffer validation of indicated length was displaced for extract packet.
        // No need to do it here again.
        // Check if notification has the predicted quantity of commas.
    notificationValidation(buffer, notification);
  }
  return notification;
}

// Check kind of response.
CommandResponse EvologicsUsblDriver::isResponse(const std::string & buffer)
{
    // In DATA mode, all response starts by "+++AT". If there is no initial string, it isn't a response.
  if(mode_ == DATA && buffer.find("+++AT") == std::string::npos) {
    return NO_RESPONSE;
  }
  return usbl_parser_.findResponse(buffer);
}

// Check a valid notification.
void EvologicsUsblDriver::notificationValidation(const std::string & buffer, const Notification & notification)
{
  usbl_parser_.splitValidateNotification(buffer, notification);
}

// Manage mode operation according command sent.
void EvologicsUsblDriver::modeManager(const std::string & command)
{
    // Command ATO. Switch to DATA mode doesn't require answer
  if(command.find("ATO") != std::string::npos && mode_ == COMMAND) {
        // Proceed just after send the command
    mode_ = DATA;
  } else if(command.find("+++") != std::string::npos && mode_ == DATA) {
    sleep(1);
        // Receive answer in COMMAND mode.
    mode_ = COMMAND;
  } else if(command.find("ATC") != std::string::npos && mode_ == DATA) {
        // Receive answer in COMMAND mode.
    mode_ = COMMAND;
  }
}

// Manage mode operation according command sent and response obtained.
void EvologicsUsblDriver::modeMsgManager(const std::string & command)
{
    // Command ATO. Switch from DATA to COMMAND mode doesn't require answer
  if(command.find("ATO") != std::string::npos && mode_ == COMMAND) {
        // Proceed just after send the command
        // queueCommand.pop();
        // mode = DATA;
  } else if(command.find("+++") != std::string::npos && mode_ != COMMAND) {
        // If there was a error, keep in DATA mode.
    mode_ = DATA;
  } else if(command.find("ATC") != std::string::npos && mode_ != COMMAND) {
        // If there was a error, keep in DATA mode.
    mode_ = DATA;
  }
}

// Send Instant Message to remote device.
void EvologicsUsblDriver::sendInstantMessage(const SendIM & im)
{
  std::string command = usbl_parser_.parseSendIM(im);
  sendCommand(command);
  waitResponseOK("AT*SENDIM", command);
}

// Get Instant Message parsed as string
std::string EvologicsUsblDriver::getStringOfIM(const SendIM & im)
{
  return fillCommand(usbl_parser_.parseSendIM(im));
}

// Parse a received Instant Message.
ReceiveIM EvologicsUsblDriver::receiveInstantMessage(const std::string & buffer)
{
  return usbl_parser_.parseReceivedIM(buffer);
}

// Get the RigidBodyState pose of remote device.
RigidBodyState EvologicsUsblDriver::getPose(const Position & pose)
{
  RigidBodyState new_pose;
  new_pose.time = pose.measurement_time;
  new_pose.position[0] = pose.x;
  new_pose.position[1] = pose.y;
  new_pose.position[2] = pose.z;
  Eigen::Vector3d euler;
  euler[0] = pose.roll;
  euler[1] = pose.pitch;
  euler[2] = pose.yaw;
  new_pose.orientation = eulerToQuaternion(euler);

  return new_pose;
}

// Get the Position
// pose of remote device.
Position EvologicsUsblDriver::getPose(const std::string & buffer)
{
  return usbl_parser_.parsePosition(buffer);
}

// Get the Direction of remote device.
Direction EvologicsUsblDriver::getDirection(const std::string & buffer)
{
  return usbl_parser_.parseDirection(buffer);
}

// Get interface type.
InterfaceType EvologicsUsblDriver::getInterface()
{
  return interface_;
}

// Define the interface with device. ETHERNET or SERIAL.
void EvologicsUsblDriver::setInterface(InterfaceType device_interface)
{
  interface_ = device_interface;
}

// Get Underwater Connection Status.
AcousticConnection EvologicsUsblDriver::getConnectionStatus()
{
  std::string command = "AT?S";
  sendCommand(command);
  return usbl_parser_.parseConnectionStatus(waitResponseString(command, command));
}

// Get Current Setting parameters.
DeviceSettings EvologicsUsblDriver::getCurrentSetting()
{
  std::string command = "AT&V";
  sendCommand(command);
  return usbl_parser_.parseCurrentSettings(waitResponseString(command, command));
}

// get Instant Message Delivery status.
DeliveryStatus EvologicsUsblDriver::getIMDeliveryStatus()
{
  std::string command = "AT?DI";
  sendCommand(command);
  return usbl_parser_.parseDeliveryStatus(waitResponseString(command, command));
}

// Delivery report notification for Instant Message.
DeliveryStatus EvologicsUsblDriver::getIMDeliveryReport(const std::string & buffer)
{
  return usbl_parser_.parseIMReport(buffer);
}

// Switch to COMMAND mode.
void EvologicsUsblDriver::GTES()
{
  std::string command = "+++";
  sendCommand(command);
  waitResponseOK("", command);
  modeMsgManager(command);
}

// Switch to COMMAND mode.
void EvologicsUsblDriver::switchToCommandMode()
{
  std::string command = "ATC";
  sendCommand(command);
  waitResponseOK("", command);
  modeMsgManager(command);
}

// Switch to DATA mode.
void EvologicsUsblDriver::switchToDataMode()
{
  std::string command = "ATO";
  sendCommand(command);
  usleep(1.5e6);
  modeMsgManager(command);
}

void EvologicsUsblDriver::resetDevice(ResetType const & type, bool ignore_unexpected_responses)
{
  std::string command = "ATZ" + std::to_string(type);
  sendCommand(command);

    // No command response
  if(type != DEVICE) {
    waitResponse("ATZ", command, COMMAND_RECEIVED, ignore_unexpected_responses);
  }

  if (type != INSTANT_MESSAGES) {
    queue_raw_data_ = std::queue<std::string>();
  }
  if (type != ACOUSTIC_CONNECTION) {
    queue_notification_ = std::queue<NotificationInfo>();
  }
}

// Pop out RawData from queueRawData.
std::vector<uint8_t> EvologicsUsblDriver::getRawData()
{
  if(!queue_raw_data_.empty()) {
    std::vector<uint8_t> ret(queue_raw_data_.front().begin(), queue_raw_data_.front().end());
    queue_raw_data_.pop();
    return ret;
  }
  throw std::runtime_error("EvologicsUsblDriver::getRawData: queueRawData is empty.");
}

// verify if queueRawData has raw data.
bool EvologicsUsblDriver::hasRawData()
{
  return !queue_raw_data_.empty();
}

// Pop out Notification from queueNotification.
NotificationInfo EvologicsUsblDriver::getNotification()
{
  if(!queue_notification_.empty()) {
    NotificationInfo ret = queue_notification_.front();
    queue_notification_.pop();
    return ret;
  }
  throw std::runtime_error("EvologicsUsblDriver::getNotification: queueNotification is empty.");
}

// verify if queueNotification has any notification.
bool EvologicsUsblDriver::hasNotification()
{
  return !queue_notification_.empty();
}

// Get mode of operation.
OperationMode EvologicsUsblDriver::getMode()
{
  return mode_;
}

// Converts from euler angles to quaternions.
Eigen::Quaterniond EvologicsUsblDriver::eulerToQuaternion(const Eigen::Vector3d & eulerAngles)
{
  Eigen::Quaterniond quaternion =
    Eigen::AngleAxisd(eulerAngles(2), Eigen::Vector3d::UnitZ()) *
    Eigen::AngleAxisd(eulerAngles(1), Eigen::Vector3d::UnitY()) *
    Eigen::AngleAxisd(eulerAngles(0), Eigen::Vector3d::UnitX());

  return quaternion;
}

void EvologicsUsblDriver::sendCommandAndACK(const std::string & command, const std::string & parameters)
{
  sendCommand(command + parameters);
  waitResponseOK(command, command + parameters);
}

// Set the specific carrier Waveform ID.
void EvologicsUsblDriver::setCarrierWaveformID(int value)
{
  sendCommandAndACK("AT!C", std::to_string(value));
}

// Set number of packets in one train.
void EvologicsUsblDriver::setClusterSize(int value)
{
  sendCommandAndACK("AT!ZC", std::to_string(value));
}

// Define limits of devices in the network.
void EvologicsUsblDriver::setHighestAddress(int value)
{
  sendCommandAndACK("AT!AM", std::to_string(value));
}

// The timeout before closing an idle acoustic connection.
void EvologicsUsblDriver::setIdleTimeout(int value)
{
  sendCommandAndACK("AT!ZI", std::to_string(value));
}

// Instant Message retry
void EvologicsUsblDriver::setIMRetry(int value)
{
  sendCommandAndACK("AT!RI", std::to_string(value));
}

// Address of local device
void EvologicsUsblDriver::setLocalAddress(int value)
{
  sendCommandAndACK("AT!AL", std::to_string(value));
}

// Address of remote device
void EvologicsUsblDriver::setRemoteAddress(int value)
{
  sendCommandAndACK("AT!AR", std::to_string(value));
}

// Get address of remote device
int EvologicsUsblDriver::getRemoteAddress()
{
  std::string command = "AT?AR";
  sendCommand(command);
  return waitResponseInt(command, command);
}

// Get highest address
int EvologicsUsblDriver::getHighestAddress()
{
  std::string command = "AT?AM";
  sendCommand(command);
  return waitResponseInt(command, command);
}

// Automatic positioning output
int EvologicsUsblDriver::getPositioningDataOutput()
{
  std::string command = "AT?ZU";
  sendCommand(command);
  return waitResponseInt(command, command);
}

// Enable or disable automatic positioning output
void EvologicsUsblDriver::setPositioningDataOutput(bool pose_on)
{
  sendCommandAndACK("AT!ZU", (pose_on ? "1" : "0"));
}

// Set input amplifier gain
void EvologicsUsblDriver::setLowGain(bool low_gain)
{
  sendCommandAndACK("AT!G", (low_gain ? "1" : "0"));
}

// Set maximum duration of a data packet.
void EvologicsUsblDriver::setPacketTime(int value)
{
  sendCommandAndACK("AT!ZP", std::to_string(value));
}

// Set if device will receive instant message addressed to others devices
void EvologicsUsblDriver::setPromiscuosMode(bool promiscuos_mode)
{
  sendCommandAndACK("AT!RP", (promiscuos_mode ? "1" : "0"));
}

// Set number of connection establishment retries.
void EvologicsUsblDriver::setRetryCount(int value)
{
  sendCommandAndACK("AT!RC", std::to_string(value));
}

// Set time of wait for establish an acoustic connection
void EvologicsUsblDriver::setRetryTimeout(int value)
{
  sendCommandAndACK("AT!RT", std::to_string(value));
}

// Set Source Level
void EvologicsUsblDriver::setSourceLevel(SourceLevel source_level)
{
  sendCommandAndACK("AT!L", std::to_string(source_level));
}

// Set if source level of local device can be changed remotely over a acoustic connection.
void EvologicsUsblDriver::setSourceLevelControl(bool source_level_control)
{
  sendCommandAndACK("AT!LC", (source_level_control ? "1" : "0"));
}

// Get source level of device
SourceLevel EvologicsUsblDriver::getSourceLevel()
{
  std::string command = "AT?L";
  sendCommand(command);
  return (SourceLevel)waitResponseInt(command, command);
}

// Get source level control of device
bool EvologicsUsblDriver::getSourceLevelControl()
{
  std::string command = "AT?LC";
  sendCommand(command);
  return  waitResponseInt(command, command) == 1;
}

// Set speed of sound on water
void EvologicsUsblDriver::setSpeedSound(int value)
{
  sendCommandAndACK("AT!CA", std::to_string(value));
}

// Set active interval of acoustic channel monitoring.
void EvologicsUsblDriver::setWakeUpActiveTime(int value)
{
  sendCommandAndACK("AT!DA", std::to_string(value));
}

// Set hold timeout after completed data transmission.
void EvologicsUsblDriver::setWakeUpHoldTimeout(int value)
{
  sendCommandAndACK("AT!ZH", std::to_string(value));
}

// Set period of the acoustic channel monitoring cycle.
void EvologicsUsblDriver::setWakeUpPeriod(int value)
{
  sendCommandAndACK("AT!DT", std::to_string(value));
}

// Set transmission buffer size of actual data channel.
void EvologicsUsblDriver::setPoolSize(int value)
{
  resetDevice(SEND_BUFFER);
  sendCommandAndACK("AT@ZL", std::to_string(value));
}

// Reset Drop Counter.
void EvologicsUsblDriver::resetDropCounter()
{
  sendCommandAndACK("AT@ZD");
}

// Reset Overflow Counter.
void EvologicsUsblDriver::resetOverflowCounter()
{
  sendCommandAndACK("AT@ZO");
}

// Get firmware information of device.
VersionNumbers EvologicsUsblDriver::getFirmwareInformation()
{
  VersionNumbers info;

  std::string command = "ATI" + std::to_string(VERSION_NUMBER);
  sendCommand(command);
  info.firmware_version = waitResponseString("ATI", command);

  command = "ATI" + std::to_string(PHY_MAC);
  sendCommand(command);
  info.accoustic_version = waitResponseString("ATI", command);

  command = "ATI" + std::to_string(MANUFACTURER);
  sendCommand(command);
  info.manufacturer = waitResponseString("ATI", command);

  return info;
}

// Get last transmission's raw bitrate value of local-to-remote direction.
int EvologicsUsblDriver::getLocalToRemoteBitrate()
{
  std::string command = "AT?BL";
  sendCommand(command);
  return waitResponseInt(command, command);
}

// Get last transmission's raw bitrate value of remote-to-local direction.
int EvologicsUsblDriver::getRemoteToLocalBitrate()
{
  std::string command = "AT?BR";
  sendCommand(command);
  return waitResponseInt(command, command);
}

// Get Received Signal Strength Indicator.
double EvologicsUsblDriver::getRSSI()
{
  std::string command = "AT?E";
  sendCommand(command);
  return waitResponseDouble(command, command);
}

// Get Signal Integrity.
int EvologicsUsblDriver::getSignalIntegrity()
{
  std::string command = "AT?I";
  sendCommand(command);
  return waitResponseInt(command, command);
}

// Get acoustic signal's propagation time between communicating devices.
int EvologicsUsblDriver::getPropagationTime()
{
  std::string command = "AT?T";
  sendCommand(command);
  return waitResponseInt(command, command);
}

// Get relative velocity between communicating devices.
double EvologicsUsblDriver::getRelativeVelocity()
{
  std::string command = "AT?V";
  sendCommand(command);
  return waitResponseDouble(command, command);
}

// Get Multipath propagation structure.
std::vector<MultiPath> EvologicsUsblDriver::getMultipath()
{
  std::string command = "AT?P";
  sendCommand(command);
  return usbl_parser_.parseMultipath(waitResponseString(command, command));
}

// Get dropCounter of actual channel
int EvologicsUsblDriver::getDropCounter()
{
  std::string command = "AT?ZD";
  sendCommand(command);
  return waitResponseInt(command, command);
}

// Get overflowCounter of actual channel
int EvologicsUsblDriver::getOverflowCounter()
{
  std::string command = "AT?ZO";
  sendCommand(command);
  return waitResponseInt(command, command);
}

// Get channel number of current interface.
int EvologicsUsblDriver::getChannelNumber()
{
  std::string command = "AT?ZS";
  sendCommand(command);
  return waitResponseInt(command, command);
}

// Get overall delivered raw data
uint64_t EvologicsUsblDriver::getRawDataDeliveryCounter()
{
  std::string command = "AT?ZE";
  sendCommand(command);
  return waitResponseULLongInt(command, command);
}

// Set System Time for current time
void EvologicsUsblDriver::setSystemTimeNow()
{
  double time_now = std::chrono::duration<double>(
    std::chrono::system_clock::now().time_since_epoch()).count();
  sendCommandAndACK("AT!UT", std::to_string(time_now));
}

// Set operation mode of device
void EvologicsUsblDriver::setOperationMode(const OperationMode & new_mode)
{
  if(mode_ != new_mode) {
    if(new_mode == DATA) {
      switchToDataMode();
    } else if(new_mode == COMMAND) {
      switchToCommandMode();
    } else {
      std::stringstream ss;
      ss << "in Driver.cpp setOperationMode, can not identify mode \"" << new_mode << "\"" << std::flush;
      throw WrongInputValue(ss.str());
    }
  }
}

// Store current setting profile
void EvologicsUsblDriver::storeCurrentSettings()
{
  sendCommandAndACK("AT&W");
}

// Restore factory settings and reset device.
void EvologicsUsblDriver::RestoreFactorySettings()
{
  std::string command = "AT&F";
  sendCommand(command);
  if(mode_ == COMMAND) {
    switchToDataMode();
  }
  return;
}

// Get communication parameters
AcousticChannel EvologicsUsblDriver::getAcousticChannelparameters()
{
  AcousticChannel channel;
  channel.time = std::chrono::system_clock::now();
  channel.rssi = getRSSI();
  channel.local_bitrate = getLocalToRemoteBitrate();
  channel.remote_bitrate = getRemoteToLocalBitrate();
  channel.propagation_time = getPropagationTime();
  channel.relative_velocity = getRelativeVelocity();
  channel.signal_integrity = getSignalIntegrity();
  channel.multi_path = getMultipath();
  channel.channel_number = getChannelNumber();
  channel.drop_count = getDropCounter();
  channel.overflow_counter = getOverflowCounter();
  channel.delivered_raw_data = getRawDataDeliveryCounter();
  return channel;
}

// Update parameters on device.
void EvologicsUsblDriver::updateDeviceParameters(
  const DeviceSettings & desired_setting,
  const DeviceSettings & actual_setting)
{
  if(desired_setting.carrier_waveform_id != actual_setting.carrier_waveform_id) {
    setCarrierWaveformID(desired_setting.carrier_waveform_id);
  }
  if(desired_setting.cluster_size != actual_setting.cluster_size) {
    setClusterSize(desired_setting.cluster_size);
  }
  if(desired_setting.highest_address != actual_setting.highest_address) {
    setHighestAddress(desired_setting.highest_address);
  }
  if(desired_setting.idle_timeout != actual_setting.idle_timeout) {
    setIdleTimeout(desired_setting.idle_timeout);
  }
  if(desired_setting.im_retry != actual_setting.im_retry) {
    setIMRetry(desired_setting.im_retry);
  }
  if(desired_setting.local_address != actual_setting.local_address) {
    setLocalAddress(desired_setting.local_address);
  }
  if(desired_setting.low_gain != actual_setting.low_gain) {
    setLowGain(desired_setting.low_gain);
  }
  if(desired_setting.packet_time != actual_setting.packet_time) {
    setPacketTime(desired_setting.packet_time);
  }
  if(desired_setting.promiscuos_mode != actual_setting.promiscuos_mode) {
    setPromiscuosMode(desired_setting.promiscuos_mode);
  }
  if(desired_setting.remote_address != actual_setting.remote_address) {
    setRemoteAddress(desired_setting.remote_address);
  }
  if(desired_setting.retry_count != actual_setting.retry_count) {
    setRetryCount(desired_setting.retry_count);
  }
  if(desired_setting.retry_timeout != actual_setting.retry_timeout) {
    setRetryTimeout(desired_setting.retry_timeout);
  }
  if(desired_setting.sound_speed != actual_setting.sound_speed) {
    setSpeedSound(desired_setting.sound_speed);
  }
  if(desired_setting.wu_active_time != actual_setting.wu_active_time) {
    setWakeUpActiveTime(desired_setting.wu_active_time);
  }
  if(desired_setting.wu_hold_timeout != actual_setting.wu_hold_timeout) {
    setWakeUpHoldTimeout(desired_setting.wu_hold_timeout);
  }
  if(desired_setting.wu_period != actual_setting.wu_period) {
    setWakeUpPeriod(desired_setting.wu_period);
  }
  if(!actual_setting.pool_size.empty() && !desired_setting.pool_size.empty()) {
         // Only takes in account the first and actual channel
    if(desired_setting.pool_size.at(0) != actual_setting.pool_size.at(0)) {
      setPoolSize(desired_setting.pool_size.at(0));
    }
  }
}
}  // namespace evologics_usbl_driver
