# Copyright (c) 2026, SENAI Cimatec
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():

    default_yaml_params = os.path.join(
        get_package_share_directory('evologics_usbl_driver'),
        'config',
        'evologics_usbl_driver.yaml'
    )

    return LaunchDescription([
        Node(
            package='evologics_usbl_driver',
            executable='evologics_usbl_driver_node',
            output='screen',
            parameters=[default_yaml_params]
        ),
    ])
