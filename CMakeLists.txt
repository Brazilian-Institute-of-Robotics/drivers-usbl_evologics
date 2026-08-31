# Copyright (c) 2026, SENAI Cimatec
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.12)
project(evologics_usbl_driver)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_ros REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(ros_driver_base REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(Boost REQUIRED)

add_library(evologics_usbl_driver
    src/evologics_usbl_driver.cpp
    src/evologics_usbl_parser.cpp
)
target_compile_features(evologics_usbl_driver PUBLIC c_std_99 cxx_std_20)

target_include_directories(evologics_usbl_driver PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

set(PACKAGE_INCLUDE_DEPS
    rclcpp
    std_msgs
    geometry_msgs
    ros_driver_base
    rclcpp_lifecycle
)

ament_target_dependencies(evologics_usbl_driver
    ${PACKAGE_INCLUDE_DEPS}
)

target_link_libraries(evologics_usbl_driver Eigen3::Eigen Boost::boost)

target_compile_definitions(evologics_usbl_driver PRIVATE "EVOLOGICS_USBL_DRIVER_BUILDING_LIBRARY")

add_library(evologics_usbl_node src/evologics_usbl_node.cpp)

target_compile_features(evologics_usbl_node PUBLIC c_std_99 cxx_std_20)

target_include_directories(evologics_usbl_node PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(evologics_usbl_node
    ${PACKAGE_INCLUDE_DEPS}
)

target_link_libraries(evologics_usbl_node Eigen3::Eigen)

target_compile_definitions(evologics_usbl_node PRIVATE "EVOLOGICS_USBL_DRIVER_BUILDING_LIBRARY")

add_executable(evologics_usbl_driver_node src/main.cpp)

target_include_directories(evologics_usbl_driver_node PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(evologics_usbl_driver_node evologics_usbl_node evologics_usbl_driver)

install(DIRECTORY include/
    DESTINATION include
)

install(TARGETS evologics_usbl_driver
    EXPORT export_${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(TARGETS evologics_usbl_node
    EXPORT export_${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(TARGETS evologics_usbl_driver_node
    DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY
    launch
    config
    DESTINATION share/${PROJECT_NAME}/
)

if(BUILD_TESTING)
    find_package(ament_lint_auto REQUIRED)
    find_package(ament_cmake_gtest REQUIRED)
    find_package(ament_cmake_gmock REQUIRED)

    set(ament_cmake_cppcheck_FOUND TRUE)
    list(APPEND AMENT_LINT_AUTO_EXCLUDE ament_cmake_cpplint)
    find_package(ament_cmake_cpplint REQUIRED)

    ament_cpplint(MAX_LINE_LENGTH "120")
    ament_lint_auto_find_test_dependencies()

    ament_add_gtest(evologics_usbl_driver_test
      test/evologics_usbl_driver_test.cpp
    )

    ament_target_dependencies(evologics_usbl_driver_test
      ${PACKAGE_INCLUDE_DEPS}
    )

    target_link_libraries(evologics_usbl_driver_test
      evologics_usbl_driver
    )

    target_include_directories(evologics_usbl_driver_test PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>
    )

    ament_add_gtest(evologics_usbl_parser_test
      test/evologics_usbl_parser_test.cpp
    )

    ament_target_dependencies(evologics_usbl_parser_test
      ${PACKAGE_INCLUDE_DEPS}
    )

    target_link_libraries(evologics_usbl_parser_test
      evologics_usbl_driver
    )

    target_include_directories(evologics_usbl_parser_test PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>
    )

    ament_add_gmock(evologics_usbl_node_test
      test/evologics_usbl_node_test.cpp
    )

    ament_target_dependencies(evologics_usbl_node_test
      ${PACKAGE_INCLUDE_DEPS}
    )

    target_link_libraries(evologics_usbl_node_test
      evologics_usbl_driver
      evologics_usbl_node
    )

    target_include_directories(evologics_usbl_node_test PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>
    )
endif()

if(BIR_ENABLE_DOCS)
    bir_add_doxygen_doc(DOC_TITLE ${PROJECT_NAME})
endif()

ament_export_include_directories(include)
ament_export_libraries(evologics_usbl_driver evologics_usbl_node)
ament_export_targets(export_${PROJECT_NAME})
ament_export_dependencies(rclcpp std_msgs geometry_msgs rclcpp_lifecycle ros_driver_base Eigen3 Boost)

ament_package()
