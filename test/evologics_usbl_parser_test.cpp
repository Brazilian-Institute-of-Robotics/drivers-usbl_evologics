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

#include <chrono>
#include <sstream>
#include <string>
#include <vector>

#include "evologics_usbl_driver/evologics_usbl_parser.hpp"
#include "evologics_usbl_driver/evologics_usbl_types.hpp"
#include "evologics_usbl_driver/evologics_usbl_exceptions.hpp"

using namespace evologics_usbl_driver;  // NOLINT
using namespace std;  // NOLINT

namespace
{
double toSeconds(std::chrono::system_clock::time_point const & time)
{
  return std::chrono::duration<double>(time.time_since_epoch()).count();
}

char peer1_8[] = {
  0x53, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x20, 0x4c,
  0x65, 0x76, 0x65, 0x6c, 0x3a, 0x20, 0x33, 0x0d,
  0x0a, 0x53, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x20,
  0x4c, 0x65, 0x76, 0x65, 0x6c, 0x20, 0x43, 0x6f,
  0x6e, 0x74, 0x72, 0x6f, 0x6c, 0x3a, 0x20, 0x30,
  0x0d, 0x0a, 0x47, 0x61, 0x69, 0x6e, 0x3a, 0x20,
  0x30, 0x0d, 0x0a, 0x43, 0x61, 0x72, 0x72, 0x69,
  0x65, 0x72, 0x20, 0x57, 0x61, 0x76, 0x65, 0x66,
  0x6f, 0x72, 0x6d, 0x20, 0x49, 0x44, 0x3a, 0x20,
  0x31, 0x0d, 0x0a, 0x4c, 0x6f, 0x63, 0x61, 0x6c,
  0x20, 0x41, 0x64, 0x64, 0x72, 0x65, 0x73, 0x73,
  0x3a, 0x20, 0x32, 0x0d, 0x0a, 0x48, 0x69, 0x67,
  0x68, 0x65, 0x73, 0x74, 0x20, 0x41, 0x64, 0x64,
  0x72, 0x65, 0x73, 0x73, 0x3a, 0x20, 0x31, 0x34,
  0x0d, 0x0a, 0x43, 0x6c, 0x75, 0x73, 0x74, 0x65,
  0x72, 0x20, 0x53, 0x69, 0x7a, 0x65, 0x3a, 0x20,
  0x31, 0x30, 0x0d, 0x0a, 0x50, 0x61, 0x63, 0x6b,
  0x65, 0x74, 0x20, 0x54, 0x69, 0x6d, 0x65, 0x3a,
  0x20, 0x37, 0x35, 0x30, 0x0d, 0x0a, 0x52, 0x65,
  0x74, 0x72, 0x79, 0x20, 0x43, 0x6f, 0x75, 0x6e,
  0x74, 0x3a, 0x20, 0x33, 0x0d, 0x0a, 0x52, 0x65,
  0x74, 0x72, 0x79, 0x20, 0x54, 0x69, 0x6d, 0x65,
  0x6f, 0x75, 0x74, 0x3a, 0x20, 0x31, 0x35, 0x30,
  0x30, 0x0d, 0x0a, 0x57, 0x61, 0x6b, 0x65, 0x20,
  0x55, 0x70, 0x20, 0x41, 0x63, 0x74, 0x69, 0x76,
  0x65, 0x20, 0x54, 0x69, 0x6d, 0x65, 0x3a, 0x20,
  0x31, 0x32, 0x0d, 0x0a, 0x57, 0x61, 0x6b, 0x65,
  0x20, 0x55, 0x70, 0x20, 0x50, 0x65, 0x72, 0x69,
  0x6f, 0x64, 0x3a, 0x20, 0x31, 0x32, 0x0d, 0x0a,
  0x50, 0x72, 0x6f, 0x6d, 0x69, 0x73, 0x63, 0x75,
  0x6f, 0x75, 0x73, 0x20, 0x4d, 0x6f, 0x64, 0x65,
  0x3a, 0x20, 0x31, 0x0d, 0x0a, 0x53, 0x6f, 0x75,
  0x6e, 0x64, 0x20, 0x53, 0x70, 0x65, 0x65, 0x64,
  0x3a, 0x20, 0x31, 0x35, 0x30, 0x30, 0x0d, 0x0a,
  0x49, 0x4d, 0x20, 0x52, 0x65, 0x72, 0x74, 0x79,
  0x20, 0x43, 0x6f, 0x75, 0x6e, 0x74, 0x3a, 0x20,
  0x31, 0x0d, 0x0a, 0x50, 0x6f, 0x6f, 0x6c, 0x20,
  0x53, 0x69, 0x7a, 0x65, 0x3a, 0x20, 0x31, 0x36,
  0x33, 0x38, 0x34, 0x0d, 0x0a, 0x48, 0x6f, 0x6c,
  0x64, 0x20, 0x54, 0x69, 0x6d, 0x65, 0x6f, 0x75,
  0x74, 0x3a, 0x20, 0x30, 0x0d, 0x0a, 0x49, 0x64,
  0x6c, 0x65, 0x20, 0x54, 0x69, 0x6d, 0x65, 0x6f,
  0x75, 0x74, 0x3a, 0x20, 0x31, 0x32, 0x30, 0x0d,
  0x0a, 0x0d, 0x0a };

char peer1_5[] = {
  0x31, 0x30, 0x20, 0x20, 0x20,
  0x20, 0x31, 0x30, 0x20, 0x20, 0x20, 0x0a, 0x31,
  0x31, 0x20, 0x20, 0x20, 0x20, 0x31, 0x32, 0x20,
  0x20, 0x20, 0x0a, 0x31, 0x32, 0x20, 0x20, 0x20,
  0x20, 0x31, 0x34, 0x20, 0x20, 0x20, 0x0a, 0x31,
  0x33, 0x20, 0x20, 0x20, 0x20, 0x31, 0x36, 0x20,
  0x20, 0x20, 0x0a, 0x31, 0x35, 0x20, 0x20, 0x20,
  0x20, 0x31, 0x38, 0x20, 0x20, 0x20, 0x0a, 0x31,
  0x36, 0x20, 0x20, 0x20, 0x20, 0x32, 0x30, 0x20,
  0x20, 0x20, 0x0a, 0x31, 0x37, 0x20, 0x20, 0x20,
  0x20, 0x32, 0x32, 0x20, 0x20, 0x20, 0x0a, 0x31,
  0x30, 0x30, 0x20, 0x20, 0x20, 0x32, 0x34, 0x30,
  0x20, 0x20, 0x0a, 0x0d, 0x0a };

char peer1_6[] = {
  0x2b, 0x2b, 0x2b, 0x41, 0x54, 0x3f, 0x50, 0x3a,
  0x39, 0x36, 0x3a,
  0x31, 0x30, 0x20, 0x20, 0x20,
  0x20, 0x31, 0x30, 0x20, 0x20, 0x20, 0x0a, 0x31,
  0x31, 0x20, 0x20, 0x20, 0x20, 0x31, 0x32, 0x20,
  0x20, 0x20, 0x0a, 0x31, 0x32, 0x20, 0x20, 0x20,
  0x20, 0x31, 0x34, 0x20, 0x20, 0x20, 0x0a, 0x31,
  0x33, 0x20, 0x20, 0x20, 0x20, 0x31, 0x36, 0x20,
  0x20, 0x20, 0x0a, 0x31, 0x35, 0x20, 0x20, 0x20,
  0x20, 0x31, 0x38, 0x20, 0x20, 0x20, 0x0a, 0x31,
  0x36, 0x20, 0x20, 0x20, 0x20, 0x32, 0x30, 0x20,
  0x20, 0x20, 0x0a, 0x31, 0x37, 0x20, 0x20, 0x20,
  0x20, 0x32, 0x32, 0x20, 0x20, 0x20, 0x0a, 0x31,
  0x30, 0x30, 0x20, 0x20, 0x20, 0x32, 0x34, 0x30,
  0x20, 0x20, 0x0a, 0x0d, 0x0a };
}  // namespace

struct UsblParserTest : public ::testing::Test
{
  UsblParser usblParser;
};

TEST_F(UsblParserTest, GetCurrentSetting) {
  string buffer(peer1_8);
  ASSERT_EQ(1, usblParser.parseCurrentSettings(buffer).imRetry);
}

TEST_F(UsblParserTest, GetCurrentSettingWithPendingSets) {
  string buffer = "[*]";
  buffer += string(peer1_8);
  ASSERT_EQ(1, usblParser.parseCurrentSettings(buffer).imRetry);
}

TEST_F(UsblParserTest, GetCurrentSettingWithPendingSetsCase2) {
  string settings = "[*]Source Level: 3\r\n";
  settings += "[*]Source Level Control: 0\r\n";
  settings += "[*]Gain: 1\r\n";
  settings += "[*]Carrier Waveform ID: 1\r\n";
  settings += "Local Address: 2\r\n";
  settings += "[*]Highest Address: 14\r\n";
  settings += "Cluster Size: 10\r\n";
  settings += "Packet Time: 750\r\n";
  settings += "[*]Retry Count: 3\r\n";
  settings += "Retry Timeout: 500\r\n";
  settings += "Wake Up Active Time: 12\r\n";
  settings += "[*]Wake Up Period: 12\r\n";
  settings += "Promiscuous Mode: 1\r\n";
  settings += "[*]Sound Speed: 1500\r\n";
  settings += "IM Rerty Count: 1\r\n";
  settings += "[*]Pool Size: 16384\r\n";
  settings += "[*]Hold Timeout: 0\r\n";
  settings += "[*]Idle Timeout: 0\r\n\r\n";
  ASSERT_EQ(1, usblParser.parseCurrentSettings(settings).imRetry);
}

TEST_F(UsblParserTest, GetPosition) {
  string position =
    "USBLLONG,1464207778.381274,1464207778.075953,2,13.6015,3.1369,1.7311,13.6015,3.1369,1.7311,"
    "0.0000,-0.0000,0.0000,9377,-54,106,0.1698\r\n";
  Position pose = usblParser.parsePosition(position);
  ASSERT_EQ(pose.rssi, -54);
  ASSERT_EQ(pose.integrity, 106);
  ASSERT_DOUBLE_EQ(pose.accuracy, 0.1698);
  ASSERT_EQ(pose.propagationTime.count(), 9377);
  ASSERT_NEAR(toSeconds(pose.time), 1464207778.381274, 1e-6);
  ASSERT_NEAR(toSeconds(pose.measurementTime), 1464207778.075953, 1e-6);
  ASSERT_EQ(pose.remoteAddress, 2);
  ASSERT_DOUBLE_EQ(pose.x, 13.6015);
  ASSERT_DOUBLE_EQ(pose.y, 3.1369);
  ASSERT_DOUBLE_EQ(pose.z, 1.7311);
}

TEST_F(UsblParserTest, GetMultipath) {
  string buffer(peer1_5);
  ASSERT_EQ(10, usblParser.parseMultipath(buffer).at(0).signalIntegrity);
}

TEST_F(UsblParserTest, GetNumber) {
  string buffer = "65";
  ASSERT_EQ(65, usblParser.getNumber(buffer));
}

TEST_F(UsblParserTest, GetNumberWithText) {
  string buffer = "65ABC";
  ASSERT_EQ(65, usblParser.getNumber(buffer));
}

TEST_F(UsblParserTest, GetNumberThrowsOnText) {
  string buffer = "ABC";
  ASSERT_THROW(usblParser.getNumber(buffer), runtime_error);
}

TEST_F(UsblParserTest, GetNumberWithPendingSet) {
  string buffer = "[*]765";
  ASSERT_EQ(765, usblParser.getNumber(buffer));
}

TEST_F(UsblParserTest, GetDouble) {
  string buffer = "65.76";
  ASSERT_DOUBLE_EQ(65.76, usblParser.getDouble(buffer));
}

TEST_F(UsblParserTest, GetDoubleWithText) {
  string buffer = "65.51ABC";
  ASSERT_DOUBLE_EQ(65.51, usblParser.getDouble(buffer));
}

TEST_F(UsblParserTest, GetDoubleThrowsOnText) {
  string buffer = "ABC";
  ASSERT_THROW(usblParser.getDouble(buffer), invalid_argument);
}

TEST_F(UsblParserTest, GetDoubleWithPendingSet) {
  string buffer = "[*]7.65";
  ASSERT_DOUBLE_EQ(7.65, usblParser.getDouble(buffer));
}

TEST_F(UsblParserTest, GetULLongInt) {
  string buffer = "65876786";
  ASSERT_EQ(65876786u, usblParser.getULLongInt(buffer));
}

TEST_F(UsblParserTest, GetULLongIntWithText) {
  string buffer = "65876786ABC";
  ASSERT_EQ(65876786u, usblParser.getULLongInt(buffer));
}

TEST_F(UsblParserTest, GetULLongIntThrowsOnText) {
  string buffer = "ABC";
  ASSERT_THROW(usblParser.getULLongInt(buffer), runtime_error);
}

TEST_F(UsblParserTest, GetULLongIntWithPendingSet) {
  string buffer = "[*]65876786";
  ASSERT_EQ(65876786u, usblParser.getULLongInt(buffer));
}

TEST_F(UsblParserTest, GetAnswerContent) {
  string buffer(peer1_6);
  string buffer1(peer1_5);
  ASSERT_EQ(buffer1, usblParser.getAnswerContent(buffer));
}

TEST_F(UsblParserTest, FuzzyMessageCorrectDataMode) {
  stringstream ss;
  string buffer("+++AT:34:RECVIM,2,1,2,ack,312,14,11,0.03,36");
  ss << buffer << 0x0D0A;
  ASSERT_NO_THROW(usblParser.splitValidateNotification(buffer, RECVIM));
}

TEST_F(UsblParserTest, FuzzyMessageColonDataMode) {
  stringstream ss;
  string buffer("+++AT:34:RECVIM,2,1,2,ack,312,14,11,0.03,:6");
  ss << buffer << 0x0D0A;
  ASSERT_NO_THROW(usblParser.splitValidateNotification(buffer, RECVIM));
}

TEST_F(UsblParserTest, FuzzyMessageCommaDataMode) {
  stringstream ss;
  string buffer("+++AT:34:RECVIM,2,1,2,ack,312,14,11,0.03,,6");
  ss << buffer << 0x0D0A;
  ASSERT_NO_THROW(usblParser.splitValidateNotification(buffer, RECVIM));
}

TEST_F(UsblParserTest, FuzzyMessageEndlineDataMode) {
  stringstream ss;
  string buffer("+++AT:34:RECVIM,2,1,2,ack,312,14,11,0.03,");
  char end_line[] = {0x0d, 0x0a};
  char msg[] = {0x20, 0x35};
  ss << buffer << msg << end_line;
  ASSERT_NO_THROW(usblParser.splitValidateNotification(ss.str(), RECVIM));
}

TEST_F(UsblParserTest, FuzzyMessageCorrectCommandMode) {
  stringstream ss;
  string buffer("RECVIM,2,1,2,ack,312,14,11,0.03,36");
  ss << buffer << 0x0D0A;
  ASSERT_NO_THROW(usblParser.splitValidateNotification(buffer, RECVIM));
}

TEST_F(UsblParserTest, FuzzyMessageColonCommandMode) {
  stringstream ss;
  string buffer("RECVIM,2,1,2,ack,312,14,11,0.03,:6");
  ss << buffer << 0x0D0A;
  ASSERT_NO_THROW(usblParser.splitValidateNotification(buffer, RECVIM));
}

TEST_F(UsblParserTest, FuzzyMessageCommaCommandMode) {
  stringstream ss;
  string buffer("RECVIM,2,1,2,ack,312,14,11,0.03,,6");
  ss << buffer << 0x0D0A;
  ASSERT_NO_THROW(usblParser.splitValidateNotification(buffer, RECVIM));
}

TEST_F(UsblParserTest, FuzzyMessageEndlineCommandMode) {
  stringstream ss;
  string buffer("RECVIM,2,1,2,ack,312,14,11,0.03,");
  char end_line[] = {0x0d, 0x0a};
  char msg[] = {0x20, 0x35};
  ss << buffer << msg << end_line;
  ASSERT_NO_THROW(usblParser.splitValidateNotification(ss.str(), RECVIM));
}

TEST_F(UsblParserTest, GetAnswerContentCorrect) {
  stringstream ss;
  string buffer("+++AT:34:RECVIM,2,1,2,ack,312,14,11,0.03,36");
  string content("RECVIM,2,1,2,ack,312,14,11,0.03,36");
  ss << buffer << 0x0D0A;
  ASSERT_EQ(content, usblParser.getAnswerContent(buffer));
}

TEST_F(UsblParserTest, GetAnswerContentColon) {
  stringstream ss;
  string buffer("+++AT:34:RECVIM,2,1,2,ack,312,14,11,0.03,:6");
  string content("RECVIM,2,1,2,ack,312,14,11,0.03,:6");
  ss << buffer << 0x0D0A;
  ASSERT_EQ(content, usblParser.getAnswerContent(buffer));
}

TEST_F(UsblParserTest, GetAnswerContentEndlineCommand) {
  stringstream ss;
  stringstream ss2;
  string buffer("+++AT:34:RECVIM,2,1,2,ack,312,14,11,0.03,");
  string content("RECVIM,2,1,2,ack,312,14,11,0.03,");
  char end_line[] = {0x0d, 0x0a};
  char msg[] = {0x20, 0x35};
  ss << buffer << msg << end_line;
  ss2 << content << msg << end_line;
  ASSERT_EQ(ss2.str(), usblParser.getAnswerContent(ss.str()));
}

TEST_F(UsblParserTest, GetAnswerContentWithCommandValidation) {
  stringstream ss;
  stringstream ss2;
  string command("+++AT*SENDIM,8,1,ack,test1234");
  string buffer("+++AT*SENDIM:2:OK");
  string content("OK");
  char end_line[] = {0x0d, 0x0a};
  ss << buffer << end_line;
  ss2 << content << end_line;
  ASSERT_EQ(ss2.str(), usblParser.getAnswerContent(ss.str(), command));
}

TEST_F(UsblParserTest, ParseReceivedIM) {
  char msg[] = { 0x31, 0x32, 0x00, 0x30, 0x35};
  char end_line[] = { 0x0d, 0x0a};

  ReceiveIM im;
  im.buffer = vector<uint8_t>(msg, msg + 5);
  im.deliveryReport = true;
  im.destination = 2;
  im.duration = std::chrono::microseconds(312);
  im.integrity = 11;
  im.rssi = 14;
  im.source = 1;
  im.time = std::chrono::system_clock::now();
  im.velocity = 0.03;

  string buffer = "+++AT:37:RECVIM,5,1,2,ack,312,14,11,0.03,";

  vector<uint8_t> vec_buffer(buffer.begin(), buffer.end());
  vec_buffer.insert(vec_buffer.end(), msg, msg + 5);
  vec_buffer.insert(vec_buffer.end(), end_line, end_line + 2);
  string sbuffer(vec_buffer.begin(), vec_buffer.end());

  vector<uint8_t> got_im = usblParser.parseReceivedIM(sbuffer).buffer;
  ASSERT_EQ(im.buffer.size(), got_im.size());
  for (size_t i = 0; i < im.buffer.size(); i++) {
    ASSERT_EQ(im.buffer[i], got_im[i]);
  }
}

TEST_F(UsblParserTest, ParseReceivedIMWithZeroByte) {
  stringstream ss;
  ReceiveIM im;

  im.deliveryReport = true;
  im.destination = 2;
  im.duration = std::chrono::microseconds(312);
  im.integrity = 11;
  im.rssi = 14;
  im.source = 1;
  im.time = std::chrono::system_clock::now();
  im.velocity = 0.03;

  string buffer("+++AT:37:RECVIM,5,1,2,ack,312,14,11,0.03,");
  char msg[] = { 0x31, 0x32, 0x01, 0x00, 0x35};
  string end_line("\r\n");
  im.buffer = vector<uint8_t>(msg, msg + 5);
  ss << buffer << string(msg, msg + 5) << end_line;

  vector<uint8_t> got_im = usblParser.parseReceivedIM(ss.str()).buffer;
  ASSERT_EQ(im.buffer.size(), got_im.size());
  for (size_t i = 0; i < im.buffer.size(); i++) {
    ASSERT_EQ(im.buffer[i], got_im[i]);
  }
}

TEST_F(UsblParserTest, ParseSendIM) {
  stringstream ss;
  SendIM im;
  im.deliveryReport = true;
  im.destination = 1;
  char msg[] = { 0x31, 0x32, 0x33, 0x34, 0x35 };

  im.buffer = vector<uint8_t>(msg, msg + 5);
  string buffer = "AT*SENDIM,5,1,ack,";

  ss << buffer << string(msg, msg + 5);

  ASSERT_EQ(ss.str(), usblParser.parseSendIM(im));
}

TEST_F(UsblParserTest, ParseSendIMWithZeroByte) {
  stringstream ss;
  SendIM im;
  im.deliveryReport = true;
  im.destination = 1;
  char msg[] = { 0x31, 0x00, 0x33, 0x00, 0x35 };

  im.buffer = vector<uint8_t>(msg, msg + 5);
  string buffer = "AT*SENDIM,5,1,ack,";

  ss << buffer << string(msg, msg + 5);
  string result = usblParser.parseSendIM(im);

  ASSERT_EQ(msg[4], result.at(result.size() - 1));
  ASSERT_EQ(ss.str(), usblParser.parseSendIM(im));
}

TEST_F(UsblParserTest, SplitMinimalValidate) {
  stringstream ss;

  string buffer("+++AT:37:RECVIM,5,1,2,ack,312,14,11,0.03,");
  char msg[] = { 0x31, 0x32, 0x01, 0x00, 0x35};
  string end_line("\r\n");
  ss << buffer << string(msg, msg + 5) << end_line;

  vector<string> test1 = usblParser.splitMinimalValidate(ss.str(), ":", 3);
  ASSERT_EQ(test1.size(), 3u);
  ASSERT_EQ(test1[0], "+++AT");
  ASSERT_EQ(test1[1], "37");
  ASSERT_EQ(test1[2], "RECVIM,5,1,2,ack,312,14,11,0.03," + string(msg, msg + 5) + end_line);

  vector<string> test2 = usblParser.splitMinimalValidate(ss.str(), ":", 2);
  ASSERT_EQ(test2.size(), 2u);
  ASSERT_EQ(test2[0], "+++AT");
  ASSERT_EQ(test2[1], "37:RECVIM,5,1,2,ack,312,14,11,0.03," + string(msg, msg + 5) + end_line);

  vector<string> test3 = usblParser.splitMinimalValidate(ss.str(), ",", 3);
  ASSERT_EQ(test3.size(), 3u);
  ASSERT_EQ(test3[0], "+++AT:37:RECVIM");
  ASSERT_EQ(test3[1], "5");
  ASSERT_EQ(test3[2], "1,2,ack,312,14,11,0.03," + string(msg, msg + 5) + end_line);

  vector<string> test4 = usblParser.splitMinimalValidate(ss.str(), ",", 1);
  ASSERT_EQ(test4.size(), 1u);
  ASSERT_EQ(test4[0], ss.str());

  ASSERT_THROW(usblParser.splitMinimalValidate(ss.str(), "&", 2), runtime_error);
}
