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

#include "evologics_usbl_driver/evologics_usbl_node.hpp"
#include "evologics_usbl_driver/evologics_usbl_driver.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  try {
    rclcpp::NodeOptions options;
    auto usbl_driver = std::make_shared<evologics_usbl_driver::EvologicsUsblDriver>();
    std::shared_ptr<rclcpp_lifecycle::LifecycleNode> usbl_node =
      std::make_shared<evologics_usbl_driver::EvologicsUsblNode>(
      "evologics_usbl_driver",
      options, usbl_driver);
    auto executor = std::make_shared<rclcpp::executors::SingleThreadedExecutor>();
    executor->add_node(usbl_node->get_node_base_interface());
    executor->spin();
  } catch (const std::exception & e) {
    RCLCPP_ERROR(rclcpp::get_logger("evologics_usbl_driver"), "%s", e.what());
    rclcpp::shutdown();
    return 1;
  }

  rclcpp::shutdown();
  return 0;
}
