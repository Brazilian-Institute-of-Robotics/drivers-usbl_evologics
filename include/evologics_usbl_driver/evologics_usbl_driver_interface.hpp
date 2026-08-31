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

#include <chrono>
#include <string>

#include "evologics_usbl_driver/evologics_usbl_types.hpp"

namespace evologics_usbl_driver
{
/**
 * @class EvologicsUsblDriverInterface
 * @brief Abstract class to define the contract used by EvologicsUsblNode to interact with the device.
 *
 * Only exposes the subset of EvologicsUsblDriver that the ROS 2 node needs, so that a mock implementation
 * can be injected in node-level tests.
 */
class EvologicsUsblDriverInterface
{
public:
  /**
   * @brief Destroy the Evologics Usbl Driver Interface object
   *
   */
  virtual ~EvologicsUsblDriverInterface() = default;

  /**
   * @brief Open the connection with the device and set the timeouts used by readPacket/writePacket.
   *
   * @param uri Uniform Resource Identifier. See ros_driver_base::Driver::openURI for the supported formats.
   * @param interface_type SERIAL (RS-232) or ETHERNET (TCP), used to select the command end-line character.
   * @param read_timeout Read timeout in milliseconds
   * @param write_timeout Write timeout in milliseconds
   */
  virtual void openConnection(
    const std::string & uri, InterfaceType interface_type,
    const std::chrono::milliseconds & read_timeout,
    const std::chrono::milliseconds & write_timeout) = 0;

  /**
   * @brief Run the device initialization sequence and leave the device in Data Mode.
   *
   * Enters Command Mode via TIES, applies the local/remote address and sound speed, enables automatic
   * positioning output, stores the settings and switches back to Data Mode.
   *
   * @param local_address Address of the local device (AT!AL)
   * @param remote_address Address of the remote device (AT!AR)
   * @param sound_speed Speed of sound in water, in m/s (AT!CA)
   */
  virtual void configurePositioning(int local_address, int remote_address, int sound_speed) = 0;

  /**
   * @brief Read one packet from the device, queueing it as a notification, response or raw data.
   *
   * @param timeout Read timeout in milliseconds. Use 0 to poll without blocking, to drain packets that
   * have already been received without waiting for a new one to arrive.
   * @return ResponseInfo. If the incoming buffer is not a response, ResponseInfo.response = NO_RESPONSE.
   */
  virtual ResponseInfo readResponse(const std::chrono::milliseconds & timeout) = 0;

  /**
   * @brief Verify if the notification queue has any notification.
   *
   * @return TRUE if the queue has a notification, FALSE otherwise.
   */
  virtual bool hasNotification() = 0;

  /**
   * @brief Pop out a Notification from the notification queue.
   *
   * @return NotificationInfo
   */
  virtual NotificationInfo getNotification() = 0;

  /**
   * @brief Parse a USBLLONG notification buffer into a Position.
   *
   * @param buffer USBLLONG notification buffer.
   * @return Position pose.
   */
  virtual Position getPose(const std::string & buffer) = 0;

  /**
   * @brief Convert a Position into a RigidBodyState.
   *
   * @param pose Position to be converted.
   * @return RigidBodyState pose.
   */
  virtual RigidBodyState getPose(const Position & pose) = 0;
};
}  // namespace evologics_usbl_driver
