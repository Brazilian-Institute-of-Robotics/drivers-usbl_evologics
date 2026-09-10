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
#include <string>
#include <utility>
#include <vector>
#include <boost/algorithm/string.hpp>

#include "evologics_usbl_driver/evologics_usbl_parser.hpp"
#include "evologics_usbl_driver/evologics_usbl_exceptions.hpp"

namespace evologics_usbl_driver
{

UsblParser::UsblParser()
{
}

UsblParser::~UsblParser()
{
}

std::chrono::system_clock::time_point UsblParser::timePointFromSeconds(double const seconds)
{
  return std::chrono::system_clock::time_point(
    std::chrono::duration_cast<std::chrono::system_clock::duration>(
      std::chrono::duration<double>(seconds)));
}

std::string UsblParser::printBuffer(const std::string & buffer)
{
  std::stringstream ss;
  bool hex_init = true;
  for (size_t i = 0; i < buffer.size(); i++) {
    if(static_cast<int>(buffer[i]) < 32 || static_cast<int>(buffer[i]) > 126 || !hex_init) {
      char byte[4];
      snprintf(byte, sizeof(byte), "%02X", (unsigned char)buffer[i]);
      if(hex_init) {
        ss << " 0X";
        hex_init = false;
      }
      ss << " " << byte;
    } else {
      ss << buffer[i];
    }
  }
  return ss.str();
}

// Print a buffer vector<uint8_t> that may contain hex that is not a character.
std::string UsblParser::printBuffer(const std::vector<uint8_t> & buffer)
{
  std::string string_buffer(buffer.begin(), buffer.end());
  return printBuffer(string_buffer);
}

// Find a Notification in a buffer.
Notification UsblParser::findNotification(std::string const & buffer) const
{
    // Tokens are matched in order, longest / most specific first, so a token that is a prefix of
    // another (e.g. "RECVIM" of "RECVIMS", "CANCELEDIM" of "CANCELEDIMS") never shadows it.
  static const std::vector<std::pair<std::string, Notification>> notification_tokens{
    {"USBLLONG", kUsbllong},
    {"USBLANGLES", kUsblangle},
    {"DELIVEREDIM", kDeliveryReport},
    {"FAILEDIM", kDeliveryReport},
    {"CANCELEDIMS", kDeliveryReport},
    {"CANCELEDIM", kDeliveryReport},
    {"CANCELEDPBM", kDeliveryReport},
    {"RECVIMS", kRecvims},
    {"RECVIM", kRecvim},
    {"RECVPBM", kRecvpbm},
    {"DROPCNT", kDropcnt},
    {"BITRATE", kExtraNotification},
    {"SRCLEVEL", kExtraNotification},
    {"PHYON", kExtraNotification},
    {"PHYOFF", kExtraNotification},
    {"RECVSTART", kExtraNotification},
    {"RECVFAILED", kExtraNotification},
    {"RECVEND", kExtraNotification},
    {"SENDSTART", kExtraNotification},
    {"SENDEND", kExtraNotification},
    {"RADDR", kExtraNotification},
    {"USBLPHYD", kExtraNotification},
  };

  for (auto const & [token, notification] : notification_tokens) {
    if (buffer.find(token) != std::string::npos) {
      return notification;
    }
  }
  return kNoNotification;
}

// Check for a Response in buffer.
CommandResponse UsblParser::findResponse(std::string const & buffer)
{
  if (buffer.find("OK") != std::string::npos) {
    return kCommandReceived;
  } else if (buffer.find("ERROR") != std::string::npos) {
    return kError;
  } else if (buffer.find("BUSY") != std::string::npos) {
    return kBusy;
  } else {
    return kValueRequested;
  }
}

// Validate the number of field of a Notification.
void UsblParser::splitValidateNotification(std::string const & buffer, Notification const & notification)
{
    // No need to separate from DATA to COMMAND mode
    // Analysis of number of fields in <notification>
    // In case the notification is a message, don't let malicious string mess up the validation
  if(notification == kRecvim || notification == kRecvims || notification == kRecvpbm) {
    splitMinimalValidate(buffer, ",", getNumberFields(notification));
  } else {
    splitValidate(buffer, ",", getNumberFields(notification));
  }
}

// Get response or notification content in DATA mode.
std::string UsblParser::getAnswerContent(std::string const & buffer)
{
  std::string msg = buffer;
  if(msg.find("+++AT") == std::string::npos) {
    throw ModeError(
      "USBL UsblParser.cpp getAnswerContent: Function only can be called "
      "in DATA mode, not in COMMAND mode. Problem with answer: \""
            +
              printBuffer(buffer) + "\"");
  }
  return splitMinimalValidate(buffer, ":", 3)[2];
}

// Get notification content in DATA mode and validate with command.
std::string UsblParser::getAnswerContent(std::string const & buffer, std::string const & command)
{
  if(buffer.find("+++AT") == std::string::npos) {
    throw ModeError(
      "USBL UsblParser.cpp getAnswerContent: Function only can be "
      "called in DATA mode, not in COMMAND mode. Problem with command: \""
            +
              command + "\" and  answer: \"" + printBuffer(buffer) + "\"");
  }
  std::vector<std::string> splitted = splitMinimalValidate(buffer, ":", 3);
  boost::algorithm::trim_if(splitted[0], boost::is_any_of("+"));
  if(command.find(splitted[0]) == std::string::npos) {
    throw ValidationError("USBL UsblParser getAnswerContent: string \"" + printBuffer(buffer) +
      "\" is not a response for the command \"" + printBuffer(command) + "\"");
  }
  return splitted[2];
}

// Parse a Instant Message into string to be sent to device.
std::string UsblParser::parseSendIM(SendIM const & im)
{
  std::stringstream ss;
  ss << "AT*SENDIM," << std::to_string(im.buffer.size()) << "," << std::to_string(im.destination) << ",";
  if(im.delivery_report) {
    ss << "ack,";
  } else {
    ss << "noack,";
  }
  ss << std::string(im.buffer.begin(), im.buffer.end());

  return ss.str();
}

// Parse a received Instant Message from buffer to ReceiveIM.
ReceiveIM UsblParser::parseReceivedIM(std::string const & buffer)
{
  ReceiveIM im;
  std::vector<std::string> splitted = splitMinimalValidate(buffer, ",", getNumberFields(kRecvim));

  if(splitted[0].find("RECVIM") == std::string::npos) {
    throw ParseError("UsblParser.cpp parseReceivedIM: Received buffer \"" + printBuffer(buffer) +
      "\" is not a RECVIM notification, but a \"" + printBuffer(splitted[0]) + "\" ");
  }

  im.time = std::chrono::system_clock::now();
  im.source = stoi(splitted[2]);
  im.destination = stoi(splitted[3]);
  if(splitted[4] == "ack") {
    im.delivery_report = true;
  } else {
    im.delivery_report = false;
  }
  im.duration = std::chrono::microseconds(stoi(splitted[5]));
  im.rssi = stoi(splitted[6]);
  im.integrity = stoi(splitted[7]);
  im.velocity = stod(splitted[8]);

    // Remove <end-line> (\r\n) from buffer
  std::string string_buffer = removeEndLine(splitted[9]);
  im.buffer = std::vector<uint8_t>(string_buffer.begin(), string_buffer.end());

  std::string::size_type size = stoi(splitted[1]);
  if(size != im.buffer.size()) {
    throw ParseError("UsblParser.cpp parseReceivedIM: Tried to split a receiving Instant Message, but the message \"" +
      printBuffer(splitted[9]) + "\" has \"" + std::to_string(im.buffer.size()) + "\" characters and not \"" +
      printBuffer(splitted[1]) + "\" as predicted.");
  }
  return im;
}

// Parse a received pose from buffer to Position.
Position UsblParser::parsePosition(std::string const & buffer)
{
  Position pose;
  std::vector<std::string> splitted = splitValidate(buffer, ",", getNumberFields(kUsbllong));

  if(splitted[0].find("USBLLONG") == std::string::npos) {
    throw ParseError("UsblParser.cpp parsePosition: Received buffer \"" + printBuffer(buffer) +
      "\" is not a USBLLONG notification, but a \"" + printBuffer(splitted[0]) + "\" ");
  }

  pose.time = timePointFromSeconds(stod(splitted[1]));
  pose.measurement_time = timePointFromSeconds(stod(splitted[2]));
  pose.remote_address = stoi(splitted[3]);
  pose.x = stod(splitted[4]);
  pose.y = stod(splitted[5]);
  pose.z = stod(splitted[6]);
  pose.E = stod(splitted[7]);
  pose.N = stod(splitted[8]);
  pose.U = stod(splitted[9]);
  pose.roll = stod(splitted[10]);
  pose.pitch = stod(splitted[11]);
  pose.yaw = stod(splitted[12]);
  pose.propagation_time = std::chrono::microseconds(stoi(splitted[13]));
  pose.rssi = stoi(splitted[14]);
  pose.integrity = stoi(splitted[15]);
  pose.accuracy = stod(splitted[16]);

  return pose;
}

// Parse a received direction from buffer to Direction.
Direction UsblParser::parseDirection(std::string const & buffer)
{
  Direction direc;
  std::vector<std::string> splitted = splitValidate(buffer, ",", getNumberFields(kUsblangle));

  if(splitted[0].find("USBLANGLE") == std::string::npos) {
    throw ParseError("UsblParser.cpp parsePosition: Received buffer \"" + printBuffer(buffer) +
      "\" is not a USBLANGLE notification, but a \"" + printBuffer(splitted[0]) + "\" ");
  }

  direc.time = timePointFromSeconds(stod(splitted[1]));
  direc.measurement_time = timePointFromSeconds(stod(splitted[2]));
  direc.remote_address = stoi(splitted[3]);
  direc.local_bearing = stod(splitted[4]);
  direc.local_elevation = stod(splitted[5]);
  direc.bearing = stod(splitted[6]);
  direc.elevation = stod(splitted[7]);
  direc.roll = stod(splitted[8]);
  direc.pitch = stod(splitted[9]);
  direc.yaw = stod(splitted[10]);
  direc.rssi = stoi(splitted[11]);
  direc.integrity = stoi(splitted[12]);
  direc.accuracy = stod(splitted[13]);

  return direc;
}

// Check if Instant Message was delivered.
DeliveryStatus UsblParser::parseIMReport(std::string const & buffer)
{
  std::string ret;
  std::vector<std::string> splitted = splitValidate(buffer, ",", getNumberFields(kDeliveryReport));
  if (splitted[0].find("DELIVEREDIM") != std::string::npos) {
    return kDelivered;
  } else if (splitted[0].find("FAILEDIM") != std::string::npos) {
    return kFailed;
  } else if (splitted[0].find("CANCELEDIM") != std::string::npos) {
    return kCanceled;
  } else {
    throw ParseError("UsblParser.cpp parseIMReport: DELIVERY_REPORT not as expected: \"" + printBuffer(splitted[0]) +
      "\"");
  }
}

// Check if buffer can be splitted in an establish amount.
std::vector<std::string> UsblParser::splitValidate(std::string const & buffer, const char * symbol, size_t const parts)
{
  std::vector<std::string> splitted;
  splitted.clear();

  boost::split(splitted, buffer, boost::is_any_of(symbol), boost::token_compress_on);
  if (splitted.size() != parts) {
    throw ValidationError("UsblParser.cpp splitValidate: Tried to split the string \"" + printBuffer(buffer) +
      "\" at \"" + symbol + "\" in " + std::to_string(parts) + " parts, but get " + std::to_string(splitted.size()) +
        " parts");
  }
  return splitted;
}

// Check if buffer can be splitted at least in a establish amount.
std::vector<std::string> UsblParser::splitMinimalValidate(
  std::string const & buffer, const char * symbol,
  size_t const parts)
{
  std::vector<std::string> splitted;
  splitted.clear();
  std::string msg = buffer;
  std::string::size_type npos = std::string::npos;
    // The number of symbol that should be present is at least number of parts -1
  for(size_t i = 0; i < parts - 1; i++) {
    if ((npos = msg.find(symbol)) != std::string::npos) {
      splitted.push_back(msg.substr(0, npos));
      msg = msg.substr(npos + 1, msg.size() - npos);
    } else {
      std::string error = "UsblParser.cpp splitMinimalValidate: string \"" + printBuffer(buffer) + "\" has not \"" +
        std::to_string(parts) + "\" symbol \"" + symbol + "\" to be splitted in.";
      throw ValidationError(error);
    }
  }
  splitted.push_back(msg.substr(0, msg.size()));
  return splitted;
}

// Remove <end-of-line> "\r\n" from buffer
std::string UsblParser::removeEndLine(std::string const & buffer)
{
    // Guard the length before slicing: a truncated device response (< 2 bytes) would underflow
    // size_t and make substr throw std::out_of_range instead of the package's ValidationError.
  if(buffer.size() >= 2 && buffer.compare(buffer.size() - 2, 2, "\r\n") == 0) {
    return buffer.substr(0, buffer.size() - 2);
  }
  throw ValidationError("UsblParser.cpp removeEndLine: There is no <end-line> \"\r\n\" in string \": " +
    printBuffer(buffer) + "\"");
}

// Get the number of fields in a Notification.
int UsblParser::getNumberFields(Notification const & notification) const
{
  switch (notification) {
    case kRecvim:
    case kRecvims:
      return 10;
      break;
    case kRecvpbm:
      return 9;
      break;
    case kDeliveryReport:
      return 2;
      break;
    case kUsbllong:
      return 17;
      break;
    case kUsblangle:
      return 14;
      break;
    case kDropcnt:
      return 2;
      break;
    case kExtraNotification:
      return 1;
      break;
    case kNoNotification:
      std::stringstream error_string;
      error_string << "UsblParser.cpp getNumberFields: Received  \"" << notification << "\"" << std::flush;
      throw ValidationError(error_string.str());
      break;
  }
  std::stringstream error_string;
  error_string << "UsblParser.cpp getNumberFields: \"" << notification << "\" is not determined" << std::flush;
  throw ValidationError(error_string.str());
}

// Get the integer from a response buffer in COMMAND mode.
int UsblParser::getNumber(std::string const & buffer)
{
  int value;
  std::string buffer_tmp = buffer;
  boost::algorithm::trim_if(buffer_tmp, boost::is_any_of("[*]"));
  std::stringstream ss(buffer_tmp);
  if (!(ss >> value)) {
    throw ParseError("UsblParser.cpp getNumber. Expected an integer response, but read \"" + printBuffer(buffer) +
      "\"");
  }
  return value;
}

// Get the double from a response buffer in COMMAND mode.
double UsblParser::getDouble(std::string const & buffer)
{
  std::string buffer_tmp = buffer;
  boost::algorithm::trim_if(buffer_tmp, boost::is_any_of("[*]"));
  return stod(buffer_tmp);
}

// Get a uint64_t from a response buffer in COMMAND mode.
uint64_t UsblParser::getULLongInt(std::string const & buffer)
{
  uint64_t value;
  std::string buffer_tmp = buffer;
  boost::algorithm::trim_if(buffer_tmp, boost::is_any_of("[*]"));
  std::stringstream ss(buffer_tmp);
  if (!(ss >> value)) {
    throw ParseError("UsblParser.cpp getULLongInt. Expected an uint64_teger response, but read \"" +
      printBuffer(buffer) + "\"");
  }
  return value;
}

// Parse AcousticConnection Status of underwater link.
AcousticConnection UsblParser::parseConnectionStatus(std::string const & buffer)
{
  AcousticConnection connection;
  connection.time = std::chrono::system_clock::now();
  if (buffer.find("OFFLINE") != std::string::npos) {
    if (buffer.find("OFFLINE CONNECTION FAILED") != std::string::npos) {
      connection.status = kOfflineConnectionFailed;
    } else if (buffer.find("OFFLINE TERMINATED") != std::string::npos) {
      connection.status = kOfflineTerminated;
    } else if (buffer.find("OFFLINE ALARM") != std::string::npos) {
      connection.status = kOfflineAlarm;
    } else if (buffer.find("OFFLINE READY") != std::string::npos) {
      connection.status = kOfflineReady;
    }
  } else if (buffer.find("INITIATION") != std::string::npos) {
    if (buffer.find("INITIATION LISTEN") != std::string::npos) {
      connection.status = kInitiationListen;
    } else if (buffer.find("INITIATION ESTABLISH") != std::string::npos) {
      connection.status = kInitiationEstablish;
    } else if (buffer.find("INITIATION DISCONNECT") != std::string::npos) {
      connection.status = kInitiationDisconnect;
    }
  } else if (buffer.find("ONLINE") != std::string::npos) {
    connection.status = kOnline;
  } else if (buffer.find("BACKOFF") != std::string::npos) {
    connection.status = kBackoff;
  } else if (buffer.find("NOISE") != std::string::npos) {
    connection.status = kNoise;
  } else if (buffer.find("DEAF") != std::string::npos) {
    connection.status = kDeaf;
  } else {
    throw ParseError("UsblParser.cpp parseConnectionStatus. Waiting for Connection Status but read \"" +
      printBuffer(buffer) + "\"");
  }
    // Get amount of free buffer of channels
  std::vector<std::string> splitted;
  boost::split(splitted, buffer, boost::algorithm::is_any_of(" ") );
  connection.free_buffer.clear();
  for(size_t i = 0; i < splitted.size(); i++) {
    if(isdigit(splitted.at(i)[0])) {
      connection.free_buffer.push_back(atoi(splitted.at(i).c_str()));
    }
  }
  return connection;
}

// Parse Delivery Status of a Message.
DeliveryStatus UsblParser::parseDeliveryStatus(std::string const & buffer)
{
  if (buffer.find("DELIVERING") != std::string::npos) {
    return kPending;
  } else if (buffer.find("EMPTY") != std::string::npos) {
    return kEmpty;
  } else if (buffer.find("FAILED") != std::string::npos) {
    return kFailed;
  } else if (buffer.find("EXPIRED") != std::string::npos) {
    return kExpired;
  }

  throw ParseError("UsblParser.cpp parseDeliveryStatus. Waiting for Delivery Status but read \"" + printBuffer(buffer) +
    "\"");
}

// Parse current settings.
DeviceSettings UsblParser::parseCurrentSettings(std::string const & buffer)
{
  DeviceSettings settings;
  std::vector<std::string> splitted;
  boost::split(splitted, buffer, boost::algorithm::is_any_of("\r\n"), boost::token_compress_on);
    // Remove last empty string from vector
  splitted.pop_back();

  for(size_t i = 0; i < splitted.size(); i++ ) {
    boost::algorithm::trim_if(splitted.at(i), boost::is_any_of("[*]"));
    std::vector<std::string> splitted2 = splitValidate(splitted.at(i), ":", 2);
    if(splitted2.at(0) == "Source Level Control") {
            // This setting is dealt separately.
      continue;
    } else if(splitted2.at(0) == "Source Level") {
            // This setting is dealt separately.
      continue;
    } else if(splitted2.at(0) == "Gain") {
      if(atoi(splitted2.at(1).c_str()) == 0) {
        settings.low_gain = false;
      } else {
        settings.low_gain = true;
      }
    } else if(splitted2.at(0) == "Carrier Waveform ID") {
      settings.carrier_waveform_id = atoi(splitted2.at(1).c_str());
    } else if(splitted2.at(0) == "Local Address") {
      settings.local_address = atoi(splitted2.at(1).c_str());
    } else if(splitted2.at(0) == "Highest Address") {
      settings.highest_address = atoi(splitted2.at(1).c_str());
    } else if(splitted2.at(0) == "Cluster Size") {
      settings.cluster_size = atoi(splitted2.at(1).c_str());
    } else if(splitted2.at(0) == "Packet Time") {
      settings.packet_time = atoi(splitted2.at(1).c_str());
    } else if(splitted2.at(0) == "Retry Timeout") {
      settings.retry_timeout = atoi(splitted2.at(1).c_str());
    } else if(splitted2.at(0) == "Wake Up Active Time") {
      settings.wu_active_time = atoi(splitted2.at(1).c_str());
    } else if(splitted2.at(0) == "Wake Up Period") {
      settings.wu_period = atoi(splitted2.at(1).c_str());
    } else if(splitted2.at(0) == "Promiscuous Mode") {
      if(atoi(splitted2.at(1).c_str()) == 0) {
        settings.promiscuos_mode = false;
      } else {
        settings.promiscuos_mode = true;
      }
    } else if(splitted2.at(0) == "Sound Speed") {
      settings.sound_speed = atoi(splitted2.at(1).c_str());
    } else if(splitted2.at(0) == "IM Rerty Count" || splitted2.at(0) == "IM Retry Count") {
      settings.im_retry = atoi(splitted2.at(1).c_str());
    } else if(splitted2.at(0) == "Retry Count") {
      settings.retry_count = atoi(splitted2.at(1).c_str());
    } else if(splitted2.at(0) == "Idle Timeout") {
      settings.idle_timeout = atoi(splitted2.at(1).c_str());
    } else if(splitted2.at(0) == "Hold Timeout") {
      settings.wu_hold_timeout = atoi(splitted2.at(1).c_str());
    } else if(splitted2.at(0) == "Pool Size") {
            // Get values for each channels available
      std::vector<std::string> splitted3;
      boost::split(splitted3, splitted2.at(1), boost::algorithm::is_any_of(" ") );
      settings.pool_size.clear();
      for(size_t i = 0; i < splitted3.size(); i++) {
        if(isdigit(splitted3.at(i)[0])) {
          settings.pool_size.push_back(atoi(splitted3.at(i).c_str()));
        }
      }
    } else {
      throw ParseError("UsblParser.cpp parseCurrentSettings. Waiting for attribute to set but read \"" +
        printBuffer(splitted2.at(0)) + "\" in buffer \"" + printBuffer(buffer) + "\"");
    }
  }
  return settings;
}

// Parse Multipath structure
std::vector<MultiPath> UsblParser::parseMultipath(std::string const & buffer)
{
  std::vector<MultiPath> vec_multipath;
    // Ignore last "\n\r\n" from buffer. Guard the length first so a buffer shorter than the
    // trailing separator does not underflow size_t and throw std::out_of_range from substr.
  if(buffer.size() < 3) {
    throw ParseError("UsblParser.cpp parseMultipath: buffer \"" + printBuffer(buffer) +
      "\" is too short to contain a multipath list.");
  }
  std::vector<std::string> splitted = splitValidate(buffer.substr(0, buffer.size() - 3), "\n", 8);

  for(size_t i = 0; i < splitted.size(); i++ ) {
    std::string msg = splitted.at(i);
    std::string::size_type npos = std::string::npos;
        // Find first value
    if ((npos = msg.find_first_of("0123456789")) != std::string::npos) {
      MultiPath multipath;
      multipath.timeline = atoi(msg.c_str());
            // Find separator
      if ((npos = msg.find(" ")) != std::string::npos) {
        msg = msg.substr(npos + 1, msg.size() - npos);
      }
            // Look for second value
      if ((npos = msg.find_first_of("0123456789")) != std::string::npos) {
        multipath.signal_integrity = atoi(msg.c_str());
      }
      vec_multipath.push_back(multipath);
    }
  }
  return vec_multipath;
}
}  // namespace evologics_usbl_driver
