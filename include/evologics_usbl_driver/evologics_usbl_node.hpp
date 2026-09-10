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

#include <memory>
#include <string>

#include <geometry_msgs/msg/pose_with_covariance_stamped.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>

#include "evologics_usbl_driver/evologics_usbl_driver_interface.hpp"
#include "evologics_usbl_driver/evologics_usbl_types.hpp"

namespace evologics_usbl_driver
{
/**
 * @class EvologicsUsblNode
 * @brief Class responsible for publishing USBL position fixes in a ROS 2 environment.
 *
 * On configuration, it runs the device's Command Mode initialization sequence (local/remote address, sound
 * speed and automatic positioning output) and leaves the device in Data Mode. While active, it periodically
 * reads the device output and publishes every USBLLONG fix as a geometry_msgs::msg::PoseWithCovarianceStamped.
 */
class EvologicsUsblNode : public rclcpp_lifecycle::LifecycleNode
{
public:
  /**
   * @brief Construct a new EvologicsUsblNode object.
   *
   * @param node_name Name of the ROS2 Lifecycle node
   * @param node_options Node options to configure the lifecycle node
   * @param usbl_driver Shared pointer to the Evologics USBL driver instance
   */
  EvologicsUsblNode(
    const std::string & node_name, rclcpp::NodeOptions node_options,
    std::shared_ptr<EvologicsUsblDriverInterface> usbl_driver);

  /**
   * @brief Destroy the EvologicsUsblNode object
   *
   */
  virtual ~EvologicsUsblNode();

private:
  /**
   * @brief Callback to handle configuration Lifecycle transition.
   * Detailed information, see <a href="https://design.ros2.org/articles/node_lifecycle.html">Lifecycle Article</a>.
   * @param state Previous Lifecycle state.
   * @return Lifecycle callback return code.
   */
  rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
  on_configure(const rclcpp_lifecycle::State & state) override;

  /**
   * @brief Callback to handle activation Lifecycle transition.
   * Detailed information, see <a href="https://design.ros2.org/articles/node_lifecycle.html">Lifecycle Article</a>.
   * @param state Previous Lifecycle state.
   * @return Lifecycle callback return code.
   */
  rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
  on_activate(const rclcpp_lifecycle::State & state) override;

  /**
   * @brief Callback to handle deactivation Lifecycle transition.
   * Detailed information, see <a href="https://design.ros2.org/articles/node_lifecycle.html">Lifecycle Article</a>.
   * @param state Previous Lifecycle state.
   * @return Lifecycle callback return code.
   */
  rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
  on_deactivate(const rclcpp_lifecycle::State & state) override;

  /**
   * @brief Callback to handle shutdown Lifecycle transition.
   * Detailed information, see <a href="https://design.ros2.org/articles/node_lifecycle.html">Lifecycle Article</a>.
   * @param state Previous Lifecycle state.
   * @return Lifecycle callback return code.
   */
  rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
  on_shutdown(const rclcpp_lifecycle::State & state) override;

  /**
   * @brief Callback to handle cleanup Lifecycle transition.
   * Detailed information, see <a href="https://design.ros2.org/articles/node_lifecycle.html">Lifecycle Article</a>.
   * @param state Previous Lifecycle state.
   * @return Lifecycle callback return code.
   */
  rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
  on_cleanup(const rclcpp_lifecycle::State & state) override;

  /**
   * @brief Timer callback that reads the device output and publishes any USBLLONG fix found.
   * This method is called periodically by the ROS 2 timer while the node is active.
   */
  void readAndPublish();

  /**
   * @brief Parse a USBLLONG notification buffer and publish it as a PoseWithCovarianceStamped.
   *
   * @param buffer USBLLONG notification buffer.
   */
  void publishPose(const std::string & buffer);

  /**
   * @brief Determine the device interface (SERIAL or ETHERNET) from the uri scheme.
   *
   * @param uri Uniform Resource Identifier used to connect to the device.
   * @return InterfaceType SERIAL for "serial://" uris, ETHERNET otherwise.
   */
  InterfaceType interfaceTypeFromUri(const std::string & uri) const;

  /**
   * @brief Declare the ROS2 parameters used in this node
   *
   */
  void declareParameters();

  /**
   * @brief Load the ROS2 parameters used in this node
   *
   */
  void loadParameters();

  /**
   * @brief Shared pointer to the ROS 2 timer object.
   */
  rclcpp::TimerBase::SharedPtr timer_;

  /**
   * @brief Time in milliseconds between reads of the device output.
   */
  int acquisition_timeout_;

  /**
   * @brief Timeout in milliseconds. Used in writePacket calls inside ros_driver_base.
   */
  int write_timeout_;

  /**
   * @brief Timeout in milliseconds. Used in readPacket calls inside ros_driver_base.
   */
  int read_timeout_;

  /**
   * @brief URI to connect to the device.
   */
  std::string uri_;

  /**
   * @brief Topic name to publish the pose data.
   */
  std::string pose_topic_name_;

  /**
   * @brief USBL sensor frame.
   */
  std::string sensor_frame_;

  /**
   * @brief Address of the local device (AT!AL).
   */
  int local_address_;

  /**
   * @brief Address of the remote device (AT!AR).
   */
  int remote_address_;

  /**
   * @brief Speed of sound in water, in m/s (AT!CA).
   */
  int sound_speed_;

  /**
   * @brief Diagonal orientation covariance applied to every published pose.
   */
  double orientation_covariance_;

  /**
   * @brief Publisher for the pose data.
   */
  rclcpp::Publisher<geometry_msgs::msg::PoseWithCovarianceStamped>::SharedPtr pose_publisher_;

  /**
   * @brief EvologicsUsblDriver instance for reading position fixes from the device.
   */
  std::shared_ptr<EvologicsUsblDriverInterface> usbl_driver_;
};
}  // namespace evologics_usbl_driver
