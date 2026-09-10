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
#include <iostream>
#include <string>
#include <vector>

#include <ros_driver_base/driver.hpp>

#include "evologics_usbl_driver/evologics_usbl_types.hpp"

namespace evologics_usbl_driver
{
class UsblParser
{
private:
  /**
   * @brief Check if buffer can be splitted in an establish amount.
   *
   * @param buffer to be analyzezd.
   * @param symbol of split.
   * @param parts of splitted buffer.
   * @return vector of string with parts size.
   */
  std::vector<std::string> splitValidate(const std::string & buffer, const char * symbol, const size_t parts);

  /**
   * @brief Convert a Unix epoch timestamp, in fractional seconds, as reported by the device, into a
   * system_clock time_point.
   *
   * @param seconds since Unix epoch, with sub-second precision.
   * @return equivalent system_clock::time_point.
   */
  std::chrono::system_clock::time_point timePointFromSeconds(const double seconds);

public:
  UsblParser();
  ~UsblParser();

  /**
   * @brief Print a buffer string that may contain hex that is not a character.
   *
   * Convert data that is not a character to its hex number. Used for debug.
   * @param buffer to be printed
   * @return string without command escape sequence.
   */
  static std::string printBuffer(const std::string & buffer);

  /**
   * @brief Print a buffer vector<uint8_t> that may contain hex that is not a character.
   *
   * Convert data that is not a character to its hex number. Used for debug.
   * @param buffer to be printed
   * @return string without command escape sequence.
   */
  static std::string printBuffer(const std::vector<uint8_t> & buffer);

  /**
   * @brief Check if buffer can be splitted at least in a establish amount.
   *
   * Ignore if buffer has more than 'parts' 'symbol'. They'd be present in the last element of vector.
   * To be used with Instant Messages.
   * @param buffer to be analyzezd.
   * @param symbol of split.
   * @param parts. minimal amount buffer can be spllited.
   * @return vector of string with parts size.
   */
  std::vector<std::string> splitMinimalValidate(const std::string & buffer, const char * symbol, const size_t parts);

  /**
   * @brief Find a Notification in a buffer.
   *
   * @param buffer to be analyzed.
   * @return Kind of notification. If buffer is not a Notification, returns kNoNotification.
   */
  Notification findNotification(const std::string & buffer) const;

  /**
   * @brief Validate the number of field of a Notification.
   *
   * In DATA mode: +++AT:<length>:<notification><end-of-line>
   * In COMMAND mode: <notification><end-of-line>
   * Check the number if fields in <notification>. Can be used in DATA or COMMAND mode.
   * Throw ValidationError in case of failure.
   * @param buffer Notification in DATA or COMMAND mode.
   * @param notification Kind of Notification in buffer.
   */
  void splitValidateNotification(const std::string & buffer, const Notification & notification);

  /**
   * @brief Check for a Response in buffer.
   *
   * Look for particular response: "OK", "ERROR" or "BUSY". If could not find these, return REQUEST_VALUE.
   * @param buffer to be analyzed.
   * @return Kind of response.
   */
  CommandResponse findResponse(const std::string & buffer);

  /**
   * @brief Get response or notification content in DATA mode.
   *
   * In DATA mode:
   * +++<AT command>:<length>:<command response><end-of-line>
   * +++<AT>:<length>:<notification><end-line>
   * Return data like in COMMAND mode.
   * Throy ValidationError or ModeError in case of failure.
   * @param buffer Notification or Response in DATA mode.
   * @return <content><end-line> like in COMMAND mode.
   */
  std::string getAnswerContent(const std::string & buffer);

  /**
   * @brief Get notification content in DATA mode and validate with command.
   *
   * In DATA mode:
   * +++<AT command>:<length>:<command response><end-of-line>
   * Return data like in COMMAND mode.
   * Throy ValidationError or ModeError in case of failure.
   * @param buffer Response in DATA mode.
   * @param command to be validate.
   * @return <content><end-line> like in COMMAND mode.
   */
  std::string getAnswerContent(const std::string & buffer, const std::string & command);

  /**
   * @brief Remove <end-of-line> "\r\n" from buffer
   *
   * @param buffer to be analyzezd.
   * @return string without <end-of-line> if it's present in buffer.
   */
  std::string removeEndLine(const std::string & buffer);

  /**
   * @brief Parse a Instant Message into string to be sent to device.
   *
   * @param im Instant Message.
   * @return string to be sent to device.
   */
  std::string parseSendIM(const SendIM & im);

  /**
   * @brief Parse a received Instant Message from buffer to ReceiveIM.
   *
   * Throw ParseError or ValidationError in case of failure.
   * @param buffer with Instant Message.
   * @return Received Instant Message.
   */
  ReceiveIM parseReceivedIM(const std::string & buffer);

  /**
   * @brief Parse a received pose from buffer to Position.
   *
   * Throw ValidationError in case of failure.
   * @param buffer with Pose.
   * @return Position.
   */
  Position parsePosition(const std::string & buffer);

  /**
   * @brief Parse a received direction from buffer to Direction.
   *
   * Throw ValidationError in case of failure.
   * @param buffer with direction.
   * @return Direction.
   */
  Direction parseDirection(const std::string & buffer);

  /**
   * @brief Check if Instant Message was delivered.
   *
   * Throw ParseError in case of failure.
   * @param buffer from device.
   * @return kDelivered if delivery was successful,
   * kFailed if remote device doesn't confirm receipt.
   * kCanceled if ack is no longer waited.
   */
  DeliveryStatus parseIMReport(const std::string & buffer);

  /**
   * @brief Get the number of fields in a Notification.
   *
   * Notification are splitted by comma ",".
   * Each Notification has a determined amount of fields.
   * Throw ValidationError in case of failure.
   * @param notification.
   * @return number of fields.
   */
  int getNumberFields(const Notification & notification) const;

  /**
   * @brief Get the integer from a response buffer in COMMAND mode.
   *
   * Throw ParseError in case of failure.
   * @param buffer with integer as response.
   * @return integer number.
   */
  int getNumber(const std::string & buffer);

  /**
   * @brief Get the double from a response buffer in COMMAND mode.
   *
   * Throw ou_of_range in case of failure.
   * @param buffer with floating point number as response.
   * @return double number.
   */
  double getDouble(const std::string & buffer);

  /**
   * @brief Get a uint64_t from a response buffer in COMMAND mode.
   *
   * Throw ou_of_range in case of failure.
   * @param buffer with a counter number as response.
   * @return uint64_t number.
   */
  uint64_t getULLongInt(const std::string & buffer);

  /**
   * @brief Parse AcousticConnection Status of underwater link.
   *
   * Throw ParseError in case of failure.
   * @param buffer with Connection Status
   * @return AcousticConnection of underwater link
   */
  AcousticConnection parseConnectionStatus(const std::string & buffer);

  /**
   * @brief Parse Delivery Status of a Message.
   *
   * Throw ParseError in case of failure.
   * @param buffer with Delivery Status.
   * @return DeleviryStatus.
   */
  DeliveryStatus parseDeliveryStatus(const std::string & buffer);

  /**
   * @brief Parse current settings.
   *
   * Throw ParseError in case of failure.
   * @param buffer with list of current device settings.
   * @return DeviceSettings.
   */
  DeviceSettings parseCurrentSettings(const std::string & buffer);

  /**
   * @brief Parse Multipath structure
   *
   * @param buffer with list of last received acoustic signal's propagation.
   * @return vector of Multipath.
   */
  std::vector<MultiPath> parseMultipath(const std::string & buffer);
};
}  // namespace evologics_usbl_driver
