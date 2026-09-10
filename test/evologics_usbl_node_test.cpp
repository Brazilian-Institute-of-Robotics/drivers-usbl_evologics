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
#include <future>
#include <stdexcept>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lifecycle_msgs/msg/state.hpp>
#include <rclcpp/rclcpp.hpp>

#include "evologics_usbl_driver/evologics_usbl_driver_interface.hpp"
#include "evologics_usbl_driver/evologics_usbl_node.hpp"

using testing::_;
using testing::Return;
using testing::Throw;

namespace evologics_usbl_driver
{
class MockEvologicsUsblDriver : public EvologicsUsblDriverInterface
{
public:
  MOCK_METHOD(
    void, openConnection,
    (const std::string & uri, InterfaceType interface_type,
    const std::chrono::milliseconds & read_timeout,
    const std::chrono::milliseconds & write_timeout), (override));

  MOCK_METHOD(
    void, configurePositioning,
    (int local_address, int remote_address, int sound_speed), (override));

  MOCK_METHOD(ResponseInfo, readResponse, (const std::chrono::milliseconds & timeout), (override));

  MOCK_METHOD(bool, hasNotification, (), (override));

  MOCK_METHOD(NotificationInfo, getNotification, (), (override));

  MOCK_METHOD(Position, getPose, (const std::string & buffer), (override));

  MOCK_METHOD(RigidBodyState, getPose, (const Position & pose), (override));
};

struct EvologicsUsblNodeTest : public ::testing::Test
{
  std::shared_ptr<rclcpp::executors::MultiThreadedExecutor> executor;
  std::shared_ptr<evologics_usbl_driver::EvologicsUsblNode> usbl_node_ptr;

  std::thread executor_thread;
  std::shared_ptr<MockEvologicsUsblDriver> mock_usbl_driver;
  std::string node_name = "test_evologics_usbl_node";
  rclcpp::NodeOptions options;

  void SetUp() override
  {
    rclcpp::init(0, nullptr);

    this->mock_usbl_driver =
      std::make_shared<evologics_usbl_driver::MockEvologicsUsblDriver>();
    this->usbl_node_ptr = std::make_shared<evologics_usbl_driver::EvologicsUsblNode>(
      node_name,
      options, mock_usbl_driver);
    this->executor = std::make_shared<rclcpp::executors::MultiThreadedExecutor>();
    this->executor->add_node(usbl_node_ptr->get_node_base_interface());
    this->executor_thread = std::thread(
      [this]() {
        this->executor->spin();
      }
    );
  }

  void TearDown() override
  {
    this->executor->cancel();
    this->executor_thread.join();
    rclcpp::shutdown();
  }

  void setDefaultParameters()
  {
    usbl_node_ptr->set_parameter({"uri", "tcp://127.0.0.1:9200"});
    usbl_node_ptr->set_parameter({"read_timeout", 200});
    usbl_node_ptr->set_parameter({"write_timeout", 200});
    usbl_node_ptr->set_parameter({"acquisition_timeout", 50});
    usbl_node_ptr->set_parameter({"pose_topic_name", "evologics/pose"});
    usbl_node_ptr->set_parameter({"sensor_frame", "cimatec_auv/USBL"});
    usbl_node_ptr->set_parameter({"local_address", 1});
    usbl_node_ptr->set_parameter({"remote_address", 2});
    usbl_node_ptr->set_parameter({"sound_speed", 1500});
    usbl_node_ptr->set_parameter({"orientation_covariance", 0.1});
  }
};

TEST_F(EvologicsUsblNodeTest, ConfigureValidParametersSucceeds) {
  this->setDefaultParameters();

  EXPECT_CALL(
    *mock_usbl_driver,
    openConnection(
      "tcp://127.0.0.1:9200", kEthernet, std::chrono::milliseconds(200),
      std::chrono::milliseconds(200))).Times(1);

  EXPECT_CALL(*mock_usbl_driver, configurePositioning(1, 2, 1500)).Times(1);

  usbl_node_ptr->configure();

  rclcpp_lifecycle::State state = this->usbl_node_ptr->get_current_state();

  ASSERT_EQ(lifecycle_msgs::msg::State::PRIMARY_STATE_INACTIVE, state.id());
}

TEST_F(EvologicsUsblNodeTest, ConfigureMissingParametersFails) {
  auto state = usbl_node_ptr->configure();

  ASSERT_EQ(state.id(), lifecycle_msgs::msg::State::PRIMARY_STATE_UNCONFIGURED);
}

TEST_F(EvologicsUsblNodeTest, ActivateAfterConfigureSucceeds) {
  this->setDefaultParameters();

  EXPECT_CALL(*mock_usbl_driver, openConnection(_, _, _, _)).Times(1);
  EXPECT_CALL(*mock_usbl_driver, configurePositioning(1, 2, 1500)).Times(1);
  EXPECT_CALL(*mock_usbl_driver, readResponse(_)).WillRepeatedly(Throw(std::runtime_error("timeout")));

  usbl_node_ptr->configure();

  rclcpp_lifecycle::State state = this->usbl_node_ptr->get_current_state();

  ASSERT_EQ(lifecycle_msgs::msg::State::PRIMARY_STATE_INACTIVE, state.id());

  usbl_node_ptr->activate();

  state = this->usbl_node_ptr->get_current_state();

  ASSERT_EQ(lifecycle_msgs::msg::State::PRIMARY_STATE_ACTIVE, state.id());
}

TEST_F(EvologicsUsblNodeTest, DeactivateSucceeds) {
  this->setDefaultParameters();

  EXPECT_CALL(*mock_usbl_driver, openConnection(_, _, _, _)).Times(1);
  EXPECT_CALL(*mock_usbl_driver, configurePositioning(1, 2, 1500)).Times(1);
  EXPECT_CALL(*mock_usbl_driver, readResponse(_)).WillRepeatedly(Throw(std::runtime_error("timeout")));

  usbl_node_ptr->configure();

  rclcpp_lifecycle::State state = this->usbl_node_ptr->get_current_state();

  ASSERT_EQ(lifecycle_msgs::msg::State::PRIMARY_STATE_INACTIVE, state.id());

  usbl_node_ptr->activate();

  state = this->usbl_node_ptr->get_current_state();

  ASSERT_EQ(lifecycle_msgs::msg::State::PRIMARY_STATE_ACTIVE, state.id());

  usbl_node_ptr->deactivate();

  state = this->usbl_node_ptr->get_current_state();

  ASSERT_EQ(lifecycle_msgs::msg::State::PRIMARY_STATE_INACTIVE, state.id());
}

TEST_F(EvologicsUsblNodeTest, UsblLongNotificationPublishesPose) {
  this->setDefaultParameters();

  EXPECT_CALL(*mock_usbl_driver, openConnection(_, _, _, _)).Times(1);
  EXPECT_CALL(*mock_usbl_driver, configurePositioning(1, 2, 1500)).Times(1);

  std::string usbllong_buffer =
    "+++AT:118:USBLLONG,455.600823,455.346033,1,1.2601,0.9197,-0.3958,1.2601,0.9197,-0.3958,"
    "0.0000,-0.0000,0.0000,1073,-37,146,0.0022\r\n";

  Position position{};
  position.accuracy = 0.0022;

  RigidBodyState rigid_body_state{};
  rigid_body_state.position = Eigen::Vector3d(1.2601, 0.9197, -0.3958);
  rigid_body_state.orientation = Eigen::Quaterniond::Identity();

  EXPECT_CALL(*mock_usbl_driver, readResponse(_))
  .WillOnce(Return(ResponseInfo{kNoResponse, ""}))
  .WillRepeatedly(Throw(std::runtime_error("timeout")));

  EXPECT_CALL(*mock_usbl_driver, hasNotification())
  .WillOnce(Return(true))
  .WillRepeatedly(Return(false));

  EXPECT_CALL(*mock_usbl_driver, getNotification())
  .WillOnce(Return(NotificationInfo{kUsbllong, usbllong_buffer}));

  EXPECT_CALL(*mock_usbl_driver, getPose(usbllong_buffer)).WillOnce(Return(position));
  EXPECT_CALL(*mock_usbl_driver, getPose(::testing::A<const Position &>())).WillOnce(Return(rigid_body_state));

  usbl_node_ptr->configure();
  usbl_node_ptr->activate();

  std::promise<geometry_msgs::msg::PoseWithCovarianceStamped> pose_promise;
  auto pose_future = pose_promise.get_future();
  bool promise_set = false;

  auto subscriber_node = std::make_shared<rclcpp::Node>("test_pose_subscriber");
  auto subscription = subscriber_node->create_subscription<geometry_msgs::msg::PoseWithCovarianceStamped>(
    "evologics/pose", 10,
    [&pose_promise, &promise_set](const geometry_msgs::msg::PoseWithCovarianceStamped & msg) {
      if (!promise_set) {
        promise_set = true;
        pose_promise.set_value(msg);
      }
    });

  executor->add_node(subscriber_node);

  ASSERT_EQ(pose_future.wait_for(std::chrono::seconds(2)), std::future_status::ready);

  geometry_msgs::msg::PoseWithCovarianceStamped received_pose = pose_future.get();
  EXPECT_EQ(received_pose.header.frame_id, "cimatec_auv/USBL");
  EXPECT_DOUBLE_EQ(received_pose.pose.pose.position.x, 1.2601);
  EXPECT_DOUBLE_EQ(received_pose.pose.pose.position.y, 0.9197);
  EXPECT_DOUBLE_EQ(received_pose.pose.pose.position.z, -0.3958);
  EXPECT_DOUBLE_EQ(received_pose.pose.covariance[0], 0.0022 * 0.0022);

  executor->remove_node(subscriber_node);
}
}  // namespace evologics_usbl_driver
