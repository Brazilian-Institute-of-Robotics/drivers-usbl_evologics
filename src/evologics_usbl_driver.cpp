#include <chrono>
#include <iostream>
#include <sstream>
#include <unistd.h>

#include "evologics_usbl_driver/evologics_usbl_driver.hpp"

namespace evologics_usbl_driver
{

EvologicsUsblDriver::EvologicsUsblDriver(const OperationMode & init_mode)
: ros_driver_base::Driver(max_packet_size), mode(init_mode)
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
void EvologicsUsblDriver::sendCommand(std::string const & command)
{
    // Fill buffer (header and end-line) according operational mode and interface type.
  std::string buffer = fillCommand(command);
  writePacket(reinterpret_cast<const uint8_t *>(buffer.c_str()), buffer.length());

    // Manage the operational mode, DATA or COMMAND, according to command.
  modeManager(command);
}

// Send raw data to remote device.
void EvologicsUsblDriver::sendRawData(std::vector<uint8_t> const & raw_data)
{
  writePacket(raw_data.data(), raw_data.size());
}

// Filled command string to be sent to device.
std::string EvologicsUsblDriver::fillCommand(std::string const & command)
{
  std::stringstream ss;
    // Guard Time Escape Sequence (GTES) doesn't require <end-line>
    // GTES: (1s)<+++>(1s)
  if(command == "+++") {
    ss << command << std::flush;
    if(mode == DATA) {
      sleep(1);
    }
  } else {
    if (mode == DATA) {
            // If in DATA, buffer = +++<AT command>
      ss << "+++" << std::flush;
    }
    ss << addEndLine(command) << std::flush;
  }
  return ss.str();
}

// Add a end line, according interface type.
std::string EvologicsUsblDriver::addEndLine(std::string const & command)
{
  std::stringstream ss;
  if (interface == SERIAL) {
    ss << command << "\r" << std::flush;
  } else { // (interface == ETHERNET)
    ss << command << "\n" << std::flush;
  }
  return ss.str();
}

int EvologicsUsblDriver::extractRawDataPacket(std::string const & buffer) const
{
  return buffer.size();
}


// Check the size of a particular response.
int EvologicsUsblDriver::checkParticularResponse(std::string const & buffer) const
{
  std::string::size_type eol = buffer.find("\r\n\r\n");
  if(eol != std::string::npos) {
        // Add \r\n\r\n to buffer.
    return eol + 4;
  }
    // Max observed Particular Response: AT&V (get parameters) with 345 in length
  else if(buffer.size() > 400) {
    throw std::runtime_error("CheckParticularResponse: Received a too big buffer of size \"" +
        std::to_string(buffer.size()) +
        "\". Check buffer \"" + usblParser.printBuffer(buffer) + "\"");
  }
  return 0;
}

// Check the size of regular response.
int EvologicsUsblDriver::checkRegularResponse(std::string const & buffer) const
{
    // Find <end-of-line>
  std::string::size_type eol = buffer.find("\r\n");
  if(eol != std::string::npos) {
        // Add \r\n to buffer.
    return eol + 2;
  }
    // Max observed Response: USBLLONG (pose) with 118 in length
  else if(buffer.size() > 150) {
    throw std::runtime_error("CheckRegularResponse: Received a too big buffer of size \"" +
        std::to_string(buffer.size()) +
        "\". Check buffer \"" + usblParser.printBuffer(buffer) + "\"");
  }
  return 0;
}

int EvologicsUsblDriver::extractATPacket(std::string const & buffer) const
{
    // Smallest packet possible is +++AT:0:\r\n
  if (buffer.size() < 10) {
    return 0;
  }
  if (buffer.substr(0, 5) != "+++AT") {
    throw std::runtime_error("extractATPacket: Buffer does not start with \"+++AT\". Check buffer, \"" +
        usblParser.printBuffer(buffer) + "\"");
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
            "\" of the end of buffer  \"" + usblParser.printBuffer(buffer) + "\"");
      }

      return length;
    } else if (buffer.size() > 16) {
      throw std::runtime_error(
          "extractATPacket: Assuming max lenght of 999, could not find second \":\" before 16 bytes in buffer, \"" +
          usblParser.printBuffer(buffer) + "\"");
    }
    return 0;
  }
    // Check max command size. +++AT?CLOCK:
  else if (buffer.size() > 12) {
    throw std::runtime_error("extractATPacket: Could not find any \":\" before 12 bytes in buffer \"" +
        usblParser.printBuffer(buffer) + "\"");
  }
  return 0;
}

int EvologicsUsblDriver::extractRawFromATPackets(std::string const & buffer) const
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
        }
                // extract raw data present before the start of +++AT packet
        else {
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

int EvologicsUsblDriver::extractPacket(uint8_t const * buffer, size_t buffer_size) const
{
  std::string buffer_as_string = std::string(reinterpret_cast<char const *>(buffer), buffer_size);

    // Both COMMAND and DATA mode, answer finish by \r\n.
  if(mode == DATA) {
    return extractRawFromATPackets(buffer_as_string);
  }
    // Check in COMMAND mode
  else { // Check for a single number as response.
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
ResponseInfo EvologicsUsblDriver::readResponse(void)
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
    queueNotification.push(notification_info);
    return response_info;
  }
    // Check for response and output it.
  else if((response = isResponse(buffer_as_string)) != NO_RESPONSE) {
    ResponseInfo response_info;
    response_info.response = response;
    response_info.buffer = buffer_as_string;
    return response_info;
  }
    // If data is neither notification or response, it's raw data.
  queueRawData.push(buffer_as_string);
  return response_info;
}


// Read packets.
std::string EvologicsUsblDriver::readInternal(void)
{
  std::vector<uint8_t> read_buffer;
  read_buffer.resize(max_packet_size);
  int readpacket = readPacket(&read_buffer[0], max_packet_size);

  return std::string(reinterpret_cast<char const *>(&read_buffer[0]), readpacket);
}

// Read input data till get a response.
std::string EvologicsUsblDriver::waitResponse(
  std::string const & expected_prefix, std::string const & command, CommandResponse expected,
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
    } else if (mode == DATA && !expected_prefix.empty() && (response_info.buffer.substr(3,
        expected_prefix.size()) != expected_prefix))
    {
      if (ignore_unexpected_responses) {
        continue;
      } else {
        throw DeviceError("USBL Driver.cpp waitResponse: Expected response " +
            usblParser.printBuffer(response_info.buffer) + " to start with " + expected_prefix);
      }
    } else if(response_info.response == ERROR) {
      throw DeviceError("USBL Driver.cpp waitResponse: For the command: \"" + usblParser.printBuffer(command) +
          "\", device return the follow ERROR msg: \"" + usblParser.printBuffer(response_info.buffer) + "\"");
    } else if(response_info.response == BUSY) {
      throw BusyError("USBL Driver.cpp waitResponse: For the command: \"" + usblParser.printBuffer(command) +
          "\", device return the follow BUSY msg: \"" + usblParser.printBuffer(response_info.buffer) +
          "\". Try it latter.");
    }
  }
    // Check for time_out
  if(response_info.response != expected) {
    throw std::runtime_error("USBL Driver.cpp waitResponse: For the command: \"" + usblParser.printBuffer(command) +
        "\", device didn't send a response in " +
        std::to_string(std::chrono::duration_cast<std::chrono::seconds>(time_out).count()) + " seconds time-out");
  }

    // In DATA mode, validate response and return content without header.
  if(mode == DATA) { // Buffer validation of indicated length was displaced for extract packet.
                     // No need to do it here again.
                     // Check if response and command match.
    return usblParser.getAnswerContent(response_info.buffer, command);
  }
  return response_info.buffer;
}

// Wait for a OK response.
void EvologicsUsblDriver::waitResponseOK(std::string const & expected_prefix, std::string const & command)
{
  waitResponse(expected_prefix, command, COMMAND_RECEIVED);
}

// Wait for a integer response.
int EvologicsUsblDriver::waitResponseInt(std::string const & expected_prefix, std::string const & command)
{
  return usblParser.getNumber(waitResponseString(expected_prefix, command));
}

// Wait for a floating point response.
double EvologicsUsblDriver::waitResponseDouble(std::string const & expected_prefix, std::string const & command)
{
  return usblParser.getDouble(waitResponseString(expected_prefix, command));
}

// Wait for a integer (that may be very long) response.
long long unsigned int EvologicsUsblDriver::waitResponseULLongInt(
  std::string const & expected_prefix,
  std::string const & command)
{
  return usblParser.getULLongInt(waitResponseString(expected_prefix, command));
}

// Wait for string response.
std::string EvologicsUsblDriver::waitResponseString(std::string const & expected_prefix, std::string const & command)
{
  return waitResponse(expected_prefix, command, VALUE_REQUESTED);
}

// Check if a Notification string is present in buffer.
int EvologicsUsblDriver::checkNotificationCommandMode(std::string const & buffer) const
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
    Notification notification = usblParser.findNotification(buffer);
    if(notification == RECVIM ||
      notification == RECVIMS ||
      notification == RECVPBM)
    {
      return checkIMNotification(buffer);
    } else {
            // It is a extra notification that starts with RECV (RECVSTART, RECVEND, ...)
      return checkRegularResponse(buffer);
    }
  }
    // All other notifications.
  else if(buffer.find("DELI") != std::string::npos ||
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
int EvologicsUsblDriver::checkIMNotification(std::string const & buffer) const
{
  if(buffer.size() < 7) {
    return 0;
  }
  Notification notification = usblParser.findNotification(buffer);
  if(notification != RECVIM && notification != RECVIMS && notification != RECVPBM) {
    throw std::runtime_error("usbl Driver.cpp checkIMNotification: Notification should be a message, instead got \"" +
        usblParser.printBuffer(buffer) + "\"");
  }

    // Get the amount of comma expected for a notification
  int ncomma = usblParser.getNumberFields(notification) - 1;
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
        "\"of the end of buffer  \"" + usblParser.printBuffer(buffer) + "\"");
  }
  return length;
}

// Check kind of notification.
Notification EvologicsUsblDriver::isNotification(std::string const & buffer)
{
  if(buffer.size() < 4) {
    return NO_NOTIFICATION;
  }
  std::string content;
  if(mode == DATA) {
        // in DATA mode +++AT:<length>:notification\r\n
    if(buffer.substr(0, 6) != "+++AT:") {
      return NO_NOTIFICATION;
    }

        // Get content of Notification.
    content = usblParser.splitMinimalValidate(buffer, ":", 3)[2];
  } else {
    content = buffer;
  }
    // If buffer is Message Notification , the message part of buffer may contains malicious data.
    // If notification is a received Message, First 4 letters of Notification string are 'RECV'
  if(content.substr(0, 4) == "RECV") {
        // Notification is a Received Message, so there is a comma after notification string
    content = usblParser.splitMinimalValidate(content, ",", 2)[0];
  }

  Notification notification = usblParser.findNotification(content);

  if(notification != NO_NOTIFICATION) {
        // Buffer validation of indicated length was displaced for extract packet.
        // No need to do it here again.
        // Check if notification has the predicted quantity of commas.
    notificationValidation(buffer, notification);
  }
  return notification;
}

// Check kind of response.
CommandResponse EvologicsUsblDriver::isResponse(std::string const & buffer)
{
    // In DATA mode, all response starts by "+++AT". If there is no initial string, it isn't a response.
  if(mode == DATA && buffer.find("+++AT") == std::string::npos) {
    return NO_RESPONSE;
  }
  return usblParser.findResponse(buffer);
}

// Check a valid notification.
void EvologicsUsblDriver::notificationValidation(std::string const & buffer, Notification const & notification)
{
  usblParser.splitValidateNotification(buffer, notification);
}

// Manage mode operation according command sent.
void EvologicsUsblDriver::modeManager(std::string const & command)
{
    // Command ATO. Switch to DATA mode doesn't require answer
  if(command.find("ATO") != std::string::npos && mode == COMMAND) {
        // Proceed just after send the command
    mode = DATA;
  }
    // Switch to COMMAND mode. (1s)<+++>(1s). Require OK response.
  else if(command.find("+++") != std::string::npos && mode == DATA) {
    sleep(1);
        // Receive answer in COMMAND mode.
    mode = COMMAND;
  }
    // Switch to COMMAND mode. Require OK response.
  else if(command.find("ATC") != std::string::npos && mode == DATA) {
        // Receive answer in COMMAND mode.
    mode = COMMAND;
  }
}

// Manage mode operation according command sent and response obtained.
void EvologicsUsblDriver::modeMsgManager(std::string const & command)
{
    // Command ATO. Switch from DATA to COMMAND mode doesn't require answer
  if(command.find("ATO") != std::string::npos && mode == COMMAND) {
        // Proceed just after send the command
        //queueCommand.pop();
        //mode = DATA;
  }
    // Switch to COMMAND mode. (1s)<+++>(1s). Require OK response.
  else if(command.find("+++") != std::string::npos && mode != COMMAND) {
        // If there was a error, keep in DATA mode.
    mode = DATA;
  }
    // Switch to COMMAND mode. Require OK response.
  else if(command.find("ATC") != std::string::npos && mode != COMMAND) {
        // If there was a error, keep in DATA mode.
    mode = DATA;
  }
}

// Send Instant Message to remote device.
void EvologicsUsblDriver::sendInstantMessage(SendIM const & im)
{
  std::string command = usblParser.parseSendIM(im);
  sendCommand(command);
  waitResponseOK("AT*SENDIM", command);
}

// Get Instant Message parsed as string
std::string EvologicsUsblDriver::getStringOfIM(SendIM const & im)
{
  return fillCommand(usblParser.parseSendIM(im));
}

// Parse a received Instant Message.
ReceiveIM EvologicsUsblDriver::receiveInstantMessage(std::string const & buffer)
{
  return usblParser.parseReceivedIM(buffer);
}

// Get the RigidBodyState pose of remote device.
RigidBodyState EvologicsUsblDriver::getPose(Position const & pose)
{
  RigidBodyState new_pose;
  new_pose.time = pose.measurementTime;
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
Position EvologicsUsblDriver::getPose(std::string const & buffer)
{
  return usblParser.parsePosition(buffer);
}

// Get the Direction of remote device.
Direction EvologicsUsblDriver::getDirection(std::string const & buffer)
{
  return usblParser.parseDirection(buffer);
}

// Get interface type.
InterfaceType EvologicsUsblDriver::getInterface(void)
{
  return interface;
}

// Define the interface with device. ETHERNET or SERIAL.
void EvologicsUsblDriver::setInterface(InterfaceType deviceInterface)
{
  interface = deviceInterface;
}

// Get Underwater Connection Status.
AcousticConnection EvologicsUsblDriver::getConnectionStatus(void)
{
  std::string command = "AT?S";
  sendCommand(command);
  return usblParser.parseConnectionStatus(waitResponseString(command, command));
}

// TODO parse input.
// Get Current Setting parameters.
DeviceSettings EvologicsUsblDriver::getCurrentSetting(void)
{
  std::string command = "AT&V";
  sendCommand(command);
  return usblParser.parseCurrentSettings(waitResponseString(command, command));
}

// get Instant Message Delivery status.
DeliveryStatus EvologicsUsblDriver::getIMDeliveryStatus(void)
{
  std::string command = "AT?DI";
  sendCommand(command);
  return usblParser.parseDeliveryStatus(waitResponseString(command, command));
}

// Delivery report notification for Instant Message.
DeliveryStatus EvologicsUsblDriver::getIMDeliveryReport(std::string const & buffer)
{
  return usblParser.parseIMReport(buffer);
}

// Switch to COMMAND mode.
void EvologicsUsblDriver::GTES(void)
{
  std::string command = "+++";
  sendCommand(command);
  waitResponseOK("", command);
  modeMsgManager(command);
}

// Switch to COMMAND mode.
void EvologicsUsblDriver::switchToCommandMode(void)
{
  std::string command = "ATC";
  sendCommand(command);
  waitResponseOK("", command);
  modeMsgManager(command);
}

// Switch to DATA mode.
void EvologicsUsblDriver::switchToDataMode(void)
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
    queueRawData = std::queue<std::string>();
  }
  if (type != ACOUSTIC_CONNECTION) {
    queueNotification = std::queue<NotificationInfo>();
  }
}

// Pop out RawData from queueRawData.
std::vector<uint8_t> EvologicsUsblDriver::getRawData(void)
{
  if(!queueRawData.empty()) {
    std::vector<uint8_t> ret(queueRawData.front().begin(), queueRawData.front().end());
    queueRawData.pop();
    return ret;
  }
  throw std::runtime_error("EvologicsUsblDriver::getRawData: queueRawData is empty.");
}

// verify if queueRawData has raw data.
bool EvologicsUsblDriver::hasRawData(void)
{
  return !queueRawData.empty();
}

// Pop out Notification from queueNotification.
NotificationInfo EvologicsUsblDriver::getNotification(void)
{
  if(!queueNotification.empty()) {
    NotificationInfo ret = queueNotification.front();
    queueNotification.pop();
    return ret;
  }
  throw std::runtime_error("EvologicsUsblDriver::getNotification: queueNotification is empty.");
}

// verify if queueNotification has any notification.
bool EvologicsUsblDriver::hasNotification(void)
{
  return !queueNotification.empty();
}

// Get mode of operation.
OperationMode EvologicsUsblDriver::getMode(void)
{
  return mode;
}

// Converts from euler angles to quaternions.
// TODO need validation and test.
Eigen::Quaterniond EvologicsUsblDriver::eulerToQuaternion(const Eigen::Vector3d & eulerAngles)
{
  Eigen::Quaterniond quaternion =
    Eigen::AngleAxisd(eulerAngles(2), Eigen::Vector3d::UnitZ()) *
    Eigen::AngleAxisd(eulerAngles(1), Eigen::Vector3d::UnitY()) *
    Eigen::AngleAxisd(eulerAngles(0), Eigen::Vector3d::UnitX());

  return quaternion;
}

void EvologicsUsblDriver::sendCommandAndACK(std::string const & command, std::string const & parameters)
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
int EvologicsUsblDriver::getRemoteAddress(void)
{
  std::string command = "AT?AR";
  sendCommand(command);
  return waitResponseInt(command, command);
}

// Get highest address
int EvologicsUsblDriver::getHighestAddress(void)
{
  std::string command = "AT?AM";
  sendCommand(command);
  return waitResponseInt(command, command);
}

// Automatic positioning output
int EvologicsUsblDriver::getPositioningDataOutput(void)
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
SourceLevel EvologicsUsblDriver::getSourceLevel(void)
{
  std::string command = "AT?L";
  sendCommand(command);
  return (SourceLevel)waitResponseInt(command, command);
}

// Get source level control of device
bool EvologicsUsblDriver::getSourceLevelControl(void)
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
void EvologicsUsblDriver::resetDropCounter(void)
{
  sendCommandAndACK("AT@ZD");
}

// Reset Overflow Counter.
void EvologicsUsblDriver::resetOverflowCounter(void)
{
  sendCommandAndACK("AT@ZO");
}

// Get firmware information of device.
VersionNumbers EvologicsUsblDriver::getFirmwareInformation(void)
{
  VersionNumbers info;

  std::string command = "ATI" + std::to_string(VERSION_NUMBER);
  sendCommand(command);
  info.firmwareVersion = waitResponseString("ATI", command);

  command = "ATI" + std::to_string(PHY_MAC);
  sendCommand(command);
  info.accousticVersion = waitResponseString("ATI", command);

  command = "ATI" + std::to_string(MANUFACTURER);
  sendCommand(command);
  info.manufacturer = waitResponseString("ATI", command);

  return info;
}

// Get last transmission's raw bitrate value of local-to-remote direction.
int EvologicsUsblDriver::getLocalToRemoteBitrate(void)
{
  std::string command = "AT?BL";
  sendCommand(command);
  return waitResponseInt(command, command);
}

// Get last transmission's raw bitrate value of remote-to-local direction.
int EvologicsUsblDriver::getRemoteToLocalBitrate(void)
{
  std::string command = "AT?BR";
  sendCommand(command);
  return waitResponseInt(command, command);
}

// Get Received Signal Strength Indicator.
double EvologicsUsblDriver::getRSSI(void)
{
  std::string command = "AT?E";
  sendCommand(command);
  return waitResponseDouble(command, command);
}

// Get Signal Integrity.
int EvologicsUsblDriver::getSignalIntegrity(void)
{
  std::string command = "AT?I";
  sendCommand(command);
  return waitResponseInt(command, command);
}

// Get acoustic signal's propagation time between communicating devices.
int EvologicsUsblDriver::getPropagationTime(void)
{
  std::string command = "AT?T";
  sendCommand(command);
  return waitResponseInt(command, command);
}

// Get relative velocity between communicating devices.
double EvologicsUsblDriver::getRelativeVelocity(void)
{
  std::string command = "AT?V";
  sendCommand(command);
  return waitResponseDouble(command, command);
}

// Get Multipath propagation structure.
std::vector<MultiPath> EvologicsUsblDriver::getMultipath(void)
{
  std::string command = "AT?P";
  sendCommand(command);
  return usblParser.parseMultipath(waitResponseString(command, command));
}

// Get dropCounter of actual channel
int EvologicsUsblDriver::getDropCounter(void)
{
  std::string command = "AT?ZD";
  sendCommand(command);
  return waitResponseInt(command, command);
}

// Get overflowCounter of actual channel
int EvologicsUsblDriver::getOverflowCounter(void)
{
  std::string command = "AT?ZO";
  sendCommand(command);
  return waitResponseInt(command, command);
}

// Get channel number of current interface.
int EvologicsUsblDriver::getChannelNumber(void)
{
  std::string command = "AT?ZS";
  sendCommand(command);
  return waitResponseInt(command, command);
}

// Get overall delivered raw data
long long unsigned int EvologicsUsblDriver::getRawDataDeliveryCounter(void)
{
  std::string command = "AT?ZE";
  sendCommand(command);
  return waitResponseULLongInt(command, command);
}

// Set System Time for current time
void EvologicsUsblDriver::setSystemTimeNow(void)
{
  double time_now = std::chrono::duration<double>(
    std::chrono::system_clock::now().time_since_epoch()).count();
  sendCommandAndACK("AT!UT", std::to_string(time_now));
}

// Set operation mode of device
void EvologicsUsblDriver::setOperationMode(OperationMode const & new_mode)
{
  if(mode != new_mode) {
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
void EvologicsUsblDriver::storeCurrentSettings(void)
{
  sendCommandAndACK("AT&W");
}

// Restore factory settings and reset device.
void EvologicsUsblDriver::RestoreFactorySettings(void)
{
  std::string command = "AT&F";
  sendCommand(command);
  if(mode == COMMAND) {
    switchToDataMode();
  }
  return;
}

// Get communication parameters
AcousticChannel EvologicsUsblDriver::getAcousticChannelparameters(void)
{
  AcousticChannel channel;
  channel.time = std::chrono::system_clock::now();
  channel.rssi = getRSSI();
  channel.localBitrate = getLocalToRemoteBitrate();
  channel.remoteBitrate = getRemoteToLocalBitrate();
  channel.propagationTime = getPropagationTime();
  channel.relativeVelocity = getRelativeVelocity();
  channel.signalIntegrity = getSignalIntegrity();
  channel.multiPath = getMultipath();
  channel.channelNumber = getChannelNumber();
  channel.dropCount = getDropCounter();
  channel.overflowCounter = getOverflowCounter();
  channel.delivered_raw_data = getRawDataDeliveryCounter();
  return channel;
}

// Update parameters on device.
void EvologicsUsblDriver::updateDeviceParameters(
  DeviceSettings const & desired_setting,
  DeviceSettings const & actual_setting)
{
  if(desired_setting.carrierWaveformId != actual_setting.carrierWaveformId) {
    setCarrierWaveformID(desired_setting.carrierWaveformId);
  }
  if(desired_setting.clusterSize != actual_setting.clusterSize) {
    setClusterSize(desired_setting.clusterSize);
  }
  if(desired_setting.highestAddress != actual_setting.highestAddress) {
    setHighestAddress(desired_setting.highestAddress);
  }
  if(desired_setting.idleTimeout != actual_setting.idleTimeout) {
    setIdleTimeout(desired_setting.idleTimeout);
  }
  if(desired_setting.imRetry != actual_setting.imRetry) {
    setIMRetry(desired_setting.imRetry);
  }
  if(desired_setting.localAddress != actual_setting.localAddress) {
    setLocalAddress(desired_setting.localAddress);
  }
  if(desired_setting.lowGain != actual_setting.lowGain) {
    setLowGain(desired_setting.lowGain);
  }
  if(desired_setting.packetTime != actual_setting.packetTime) {
    setPacketTime(desired_setting.packetTime);
  }
  if(desired_setting.promiscuosMode != actual_setting.promiscuosMode) {
    setPromiscuosMode(desired_setting.promiscuosMode);
  }
  if(desired_setting.remoteAddress != actual_setting.remoteAddress) {
    setRemoteAddress(desired_setting.remoteAddress);
  }
  if(desired_setting.retryCount != actual_setting.retryCount) {
    setRetryCount(desired_setting.retryCount);
  }
  if(desired_setting.retryTimeout != actual_setting.retryTimeout) {
    setRetryTimeout(desired_setting.retryTimeout);
  }
  if(desired_setting.speedSound != actual_setting.speedSound) {
    setSpeedSound(desired_setting.speedSound);
  }
  if(desired_setting.wuActiveTime != actual_setting.wuActiveTime) {
    setWakeUpActiveTime(desired_setting.wuActiveTime);
  }
  if(desired_setting.wuHoldTimeout != actual_setting.wuHoldTimeout) {
    setWakeUpHoldTimeout(desired_setting.wuHoldTimeout);
  }
  if(desired_setting.wuPeriod != actual_setting.wuPeriod) {
    setWakeUpPeriod(desired_setting.wuPeriod);
  }
  if(!actual_setting.poolSize.empty() && !desired_setting.poolSize.empty()) {
         // Only takes in account the first and actual channel
    if(desired_setting.poolSize.at(0) != actual_setting.poolSize.at(0)) {
      setPoolSize(desired_setting.poolSize.at(0));
    }
  }
}
}  // namespace evologics_usbl_driver
