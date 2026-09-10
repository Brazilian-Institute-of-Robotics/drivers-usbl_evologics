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

#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "evologics_usbl_driver/evologics_usbl_driver.hpp"

using namespace evologics_usbl_driver;  // NOLINT
using namespace std;  // NOLINT

struct EvologicsUsblDriverTest : public ::testing::Test
{
  EvologicsUsblDriver driver;
};

TEST_F(EvologicsUsblDriverTest, ExtractRawFromATPacketsSimplePacketAtStart) {
  string buffer("+++AT:5:12345\r\n");
  ASSERT_EQ(15, driver.extractRawFromATPackets(buffer));
}

TEST_F(EvologicsUsblDriverTest, ExtractRawFromATPacketsThrowsWithoutEndline) {
  string buffer("+++AT:5:12345ab\r\n");
  ASSERT_THROW(driver.extractRawFromATPackets(buffer), runtime_error);
}

TEST_F(EvologicsUsblDriverTest, ExtractRawFromATPacketsWrongLength) {
  string buffer("+++AT:30:12345\r\n");
  ASSERT_EQ(0, driver.extractRawFromATPackets(buffer));
}

TEST_F(EvologicsUsblDriverTest, ExtractRawFromATPacketsRawDataBeforePacket) {
  string buffer("12345+++AT:5:12345\r\n");
  ASSERT_EQ(5, driver.extractRawFromATPackets(buffer));
}

TEST_F(EvologicsUsblDriverTest, ExtractRawFromATPacketsRawDataWithOnePlus) {
  string buffer("12345++++AT:5:12345\r\n");
  ASSERT_EQ(6, driver.extractRawFromATPackets(buffer));
}

TEST_F(EvologicsUsblDriverTest, ExtractRawFromATPacketsRawDataWithTwoPlus) {
  string buffer("12345+++++AT:5:12345\r\n");
  ASSERT_EQ(7, driver.extractRawFromATPackets(buffer));
}

TEST_F(EvologicsUsblDriverTest, ExtractRawFromATPacketsRawDataWithThreePlus) {
  string buffer("12345++++++AT:5:12345\r\n");
  ASSERT_EQ(8, driver.extractRawFromATPackets(buffer));
}

TEST_F(EvologicsUsblDriverTest, ExtractRawFromATPacketsRawDataOfPlusA) {
  string buffer("+++A+++A++++++AT:5:12345\r\n");
  ASSERT_EQ(11, driver.extractRawFromATPackets(buffer));
}

TEST_F(EvologicsUsblDriverTest, ExtractRawFromATPacketsPossiblePacketAtEndWithPlusA) {
  string buffer("+++A+++A+++.++AT+");
  ASSERT_EQ(16, driver.extractRawFromATPackets(buffer));
}

TEST_F(EvologicsUsblDriverTest, ExtractRawFromATPacketsPossiblePacketAtEnd) {
  string buffer("12345++");
  ASSERT_EQ(5, driver.extractRawFromATPackets(buffer));
}

TEST_F(EvologicsUsblDriverTest, ExtractRawFromATPacketsPlusInMiddle) {
  string buffer("123++12");
  ASSERT_EQ(7, driver.extractRawFromATPackets(buffer));
}

TEST_F(EvologicsUsblDriverTest, ExtractRawFromATPacketsNullByteInMiddle) {
  string buffer("+++AT:37:RECVIM,5,1,2,ack,312,14,11,0.03,");
  char msg[] = {0x31, 0x32, 0x01, 0x00, 0x35};
  string end_line("\r\n");
  stringstream ss;
  ss << buffer << string(msg, msg + 5) << end_line;
  ASSERT_EQ(48, driver.extractRawFromATPackets(ss.str()));
}

TEST_F(EvologicsUsblDriverTest, ExtractATPacketNestedMessageWithNullByte) {
  string buffer("+++AT");
  string splitter(":");
  string IM("RECVIM,5,1,2,ack,312,14,11,0.03,");
  char msg[] = {0x31, 0x32, 0x01, 0x00, 0x35};
  string end_line("\r\n");
  stringstream inception;
  inception << IM << buffer << splitter << to_string(IM.size() + 5) << splitter << IM << string(msg, msg + 5) <<
    end_line;
  stringstream ss;
  ss << buffer << splitter << to_string(inception.str().size()) << splitter << inception.str() << end_line;
  ASSERT_EQ(ss.str().size(), static_cast<size_t>(driver.extractATPacket(ss.str())));
}

TEST_F(EvologicsUsblDriverTest, ExtractATPacketNestedNotificationString) {
  string buffer("+++AT");
  string splitter(":");
  string IM("RECVIM,5,1,2,ack,312,14,11,0.03,");
  string other_notification(
    "+++AT:118:USBLLONG,455.600823,455.346033,1,1.2601,0.9197,-0.3958,1.2601,0.9197,-0.3958,0.0000,-0.0000,0.0000,"
    "1073,-37,146,0.0022\r\n");
  string end_line("\r\n");
  stringstream inception;
  inception << IM << other_notification;
  stringstream ss;
  ss << buffer << splitter << to_string(inception.str().size()) << splitter << inception.str() << end_line;
  ASSERT_EQ(ss.str().size(), static_cast<size_t>(driver.extractATPacket(ss.str())));
  ASSERT_EQ(kRecvim, driver.isNotification(ss.str()));
}

TEST_F(EvologicsUsblDriverTest, CheckNotificationCommandModeRecvim) {
  string buffer("RECVIM,5,1,2,ack,312,14,11,0.03,");
  char msg[] = {0x31, 0x32, 0x01, 0x00, 0x35};
  string end_line("\r\n");
  stringstream ss;
  ss << buffer << string(msg, msg + 5) << end_line;
  ASSERT_EQ(ss.str().size(), static_cast<size_t>(driver.checkNotificationCommandMode(ss.str())));
}

TEST_F(EvologicsUsblDriverTest, CheckNotificationCommandModeIncompleteMessage) {
  string buffer("RECVIM,5,1,2,ack,312,14,11,0.03");
  ASSERT_EQ(0, driver.checkNotificationCommandMode(buffer));
}

TEST_F(EvologicsUsblDriverTest, CheckNotificationCommandModeEndlineInsideMessage) {
  string buffer("RECVIM,7,1,2,ack,312,14,11,0.03,");
  char msg[] = {0x31, 0x32, 0x01, 0x00, 0x35};
  string end_line("\r\n");
  stringstream ss;
  ss << buffer << string(msg, msg + 5) << end_line << end_line;
  ASSERT_EQ(ss.str().size(), static_cast<size_t>(driver.checkNotificationCommandMode(ss.str())));
}

TEST_F(EvologicsUsblDriverTest, CheckNotificationCommandModeThrowsOnWrongSize) {
  string buffer("RECVIM,1,1,2,ack,312,14,11,0.03,");
  char msg[] = {0x31, 0x32, 0x01, 0x00, 0x35};
  string end_line("\r\n");
  stringstream ss;
  ss << buffer << string(msg, msg + 5) << end_line;
  ASSERT_THROW(driver.checkNotificationCommandMode(ss.str()), runtime_error);
}

TEST_F(EvologicsUsblDriverTest, CheckNotificationCommandModeThrowsOnLongField) {
  string buffer("RECVIM,1,1,2,ack,3128775663861479831,14,11,0.03,");
  char msg[] = {0x31, 0x32, 0x01, 0x00, 0x35};
  string end_line("\r\n");
  stringstream ss;
  ss << buffer << string(msg, msg + 5) << end_line;
  ASSERT_THROW(driver.checkNotificationCommandMode(ss.str()), runtime_error);
}

TEST_F(EvologicsUsblDriverTest, CheckNotificationCommandModeMissingComma) {
  string buffer("RECVIM,1,1,2,ack,312,1411,0.03,");
  char msg[] = {0x31, 0x32, 0x01, 0x00, 0x35};
  string end_line("\r\n");
  stringstream ss;
  ss << buffer << string(msg, msg + 5) << end_line;
  ASSERT_EQ(0, driver.checkNotificationCommandMode(ss.str()));
}

TEST_F(EvologicsUsblDriverTest, CheckNotificationCommandModeFollowedByAnotherNotification) {
  string buffer("RECVIM,6,1,2,ack,312,14,11,0.03,,");
  char msg[] = {0x31, 0x32, 0x01, 0x00, 0x35};
  string end_line("\r\n");
  stringstream ss;
  ss << buffer << string(msg, msg + 5) << end_line;
  ss << ss.str();
  ASSERT_EQ(ss.str().size() / 2, static_cast<size_t>(driver.checkNotificationCommandMode(ss.str())));
}

TEST_F(EvologicsUsblDriverTest, CheckNotificationCommandModeNestedMessage) {
  string notification("RECVIM,");
  string parameters(",1,2,ack,312,14,11,0.03,");
  char msg[] = {0x31, 0x32, 0x01, 0x00, 0x35};
  string end_line("\r\n");
  stringstream message;
  message << notification << "5" << parameters << string(msg, msg + 5) << end_line;
  stringstream ss;
  ss << notification << to_string(message.str().size()) << parameters << message.str() << end_line;
  ASSERT_EQ(ss.str().size(), static_cast<size_t>(driver.checkNotificationCommandMode(ss.str())));
}

TEST_F(EvologicsUsblDriverTest, CheckNotificationCommandModeRecvend) {
  string buffer("RECVEND,3349740869,182272,-40,120,0.1000");
  string end_line("\r\n");
  stringstream ss;
  ss << buffer << end_line;
  ASSERT_EQ(ss.str().size(), static_cast<size_t>(driver.checkNotificationCommandMode(ss.str())));
}
