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

#include "evologics_usbl_driver/evologics_usbl_node.hpp"

namespace evologics_usbl_driver
{

EvologicsUsblNode::EvologicsUsblNode(
  const std::string & node_name, rclcpp::NodeOptions node_options,
  std::shared_ptr<EvologicsUsblDriverInterface> usbl_driver)
: rclcpp_lifecycle::LifecycleNode(node_name, node_options),
  usbl_driver_(std::move(usbl_driver))
{
  this->declareParameters();
}

EvologicsUsblNode::~EvologicsUsblNode() {}

/* *INDENT-OFF* */
rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn EvologicsUsblNode::on_configure(
  const rclcpp_lifecycle::State & state)
/* *INDENT-ON* */
{
  this->loadParameters();

  pose_publisher_ =
    this->create_publisher<geometry_msgs::msg::PoseWithCovarianceStamped>(pose_topic_name_, 10);

  usbl_driver_->openConnection(
    uri_, interfaceTypeFromUri(uri_), std::chrono::milliseconds(read_timeout_),
    std::chrono::milliseconds(write_timeout_));

  usbl_driver_->configurePositioning(local_address_, remote_address_, sound_speed_);

  RCLCPP_INFO(
    this->get_logger(),
    "Evologics USBL driver configured successfully. Previous state was %s", state.label().c_str());
  return rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn::SUCCESS;
}

rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn EvologicsUsblNode::on_activate(
  const rclcpp_lifecycle::State & state)
{
  timer_ = this->create_wall_timer(
    std::chrono::milliseconds(acquisition_timeout_),
    std::bind(&EvologicsUsblNode::readAndPublish, this));

  RCLCPP_INFO(
    this->get_logger(),
    "Evologics USBL driver activated successfully. Previous state was %s", state.label().c_str());
  return rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn::SUCCESS;
}

rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn EvologicsUsblNode::on_deactivate(
  const rclcpp_lifecycle::State & state)
{
  timer_.reset();
  RCLCPP_INFO(
    this->get_logger(),
    "Evologics USBL driver deactivated successfully. Previous state was %s", state.label().c_str());
  return rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn::SUCCESS;
}

rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn EvologicsUsblNode::on_cleanup(
  const rclcpp_lifecycle::State & state)
{
  pose_publisher_.reset();

  RCLCPP_INFO(
    this->get_logger(),
    "Evologics USBL driver cleanup successfully. Previous state was %s", state.label().c_str());
  return rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn::SUCCESS;
}

rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn EvologicsUsblNode::on_shutdown(
  const rclcpp_lifecycle::State & state)
{
  RCLCPP_INFO(
    this->get_logger(),
    "Evologics USBL driver shutdown successfully. Previous state was %s", state.label().c_str());
  return rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn::SUCCESS;
}

void EvologicsUsblNode::readAndPublish()
{
  try {
    usbl_driver_->readResponse(std::chrono::milliseconds(read_timeout_));
  } catch (const std::exception & e) {
    return;
  }

    // Drain every other packet already buffered, without waiting: reading only one packet per tick
    // let a backlog build up whenever fixes arrived faster than the timer ticked, which made the
    // published pose lag behind the live data.
  while (true) {
    try {
      usbl_driver_->readResponse(std::chrono::milliseconds(0));
    } catch (const std::exception & e) {
      break;
    }
  }

  while (usbl_driver_->hasNotification()) {
    NotificationInfo notification_info = usbl_driver_->getNotification();
    if (notification_info.notification != USBLLONG) {
      continue;
    }

    try {
      publishPose(notification_info.buffer);
    } catch (const std::exception & e) {
      RCLCPP_WARN(this->get_logger(), "%s", e.what());
    }
  }
}

void EvologicsUsblNode::publishPose(const std::string & buffer)
{
  Position position = usbl_driver_->getPose(buffer);
  RigidBodyState rigid_body_state = usbl_driver_->getPose(position);

  geometry_msgs::msg::PoseWithCovarianceStamped pose_msg =
    geometry_msgs::msg::PoseWithCovarianceStamped();

  pose_msg.header.stamp = this->get_clock()->now();
  pose_msg.header.frame_id = sensor_frame_;

  pose_msg.pose.pose.position.x = rigid_body_state.position.x();
  pose_msg.pose.pose.position.y = rigid_body_state.position.y();
  pose_msg.pose.pose.position.z = rigid_body_state.position.z();

  pose_msg.pose.pose.orientation.x = rigid_body_state.orientation.x();
  pose_msg.pose.pose.orientation.y = rigid_body_state.orientation.y();
  pose_msg.pose.pose.orientation.z = rigid_body_state.orientation.z();
  pose_msg.pose.pose.orientation.w = rigid_body_state.orientation.w();

  double position_variance = position.accuracy * position.accuracy;

  pose_msg.pose.covariance = {
    position_variance, 0, 0, 0, 0, 0,
    0, position_variance, 0, 0, 0, 0,
    0, 0, position_variance, 0, 0, 0,
    0, 0, 0, orientation_covariance_, 0, 0,
    0, 0, 0, 0, orientation_covariance_, 0,
    0, 0, 0, 0, 0, orientation_covariance_};

  pose_publisher_->publish(pose_msg);
}

InterfaceType EvologicsUsblNode::interfaceTypeFromUri(const std::string & uri) const
{
  const std::string serial_scheme = "serial://";
  if (uri.compare(0, serial_scheme.size(), serial_scheme) == 0) {
    return SERIAL;
  }
  return ETHERNET;
}

void EvologicsUsblNode::declareParameters()
{
  this->declare_parameter("uri", "/");
  this->declare_parameter("acquisition_timeout", 0);
  this->declare_parameter("write_timeout", 0);
  this->declare_parameter("read_timeout", 0);
  this->declare_parameter("pose_topic_name", "");
  this->declare_parameter("sensor_frame", "evologics_usbl_link");
  this->declare_parameter("local_address", 1);
  this->declare_parameter("remote_address", 2);
  this->declare_parameter("sound_speed", 1500);
  this->declare_parameter("orientation_covariance", 0.1);
}

void EvologicsUsblNode::loadParameters()
{
  uri_ = this->get_parameter("uri").get_parameter_value().get<std::string>();
  acquisition_timeout_ =
    this->get_parameter("acquisition_timeout").get_parameter_value().get<int>();
  write_timeout_ = this->get_parameter("write_timeout").get_parameter_value().get<int>();
  read_timeout_ = this->get_parameter("read_timeout").get_parameter_value().get<int>();
  pose_topic_name_ =
    this->get_parameter("pose_topic_name").get_parameter_value().get<std::string>();
  sensor_frame_ = this->get_parameter("sensor_frame").get_parameter_value().get<std::string>();
  local_address_ = this->get_parameter("local_address").get_parameter_value().get<int>();
  remote_address_ = this->get_parameter("remote_address").get_parameter_value().get<int>();
  sound_speed_ = this->get_parameter("sound_speed").get_parameter_value().get<int>();
  orientation_covariance_ =
    this->get_parameter("orientation_covariance").get_parameter_value().get<double>();
}
}  // namespace evologics_usbl_driver
