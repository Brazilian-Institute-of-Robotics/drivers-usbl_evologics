#include <boost/test/unit_test.hpp>
#include <usbl_evologics/Driver.hpp>
#include <usbl_evologics/UsblParser.hpp>

#define BOOST_TEST_MODULE "usbl_parser"

using namespace usbl_evologics;
using namespace std;

BOOST_AUTO_TEST_SUITE(UsblParser_parseCurrentSettings)

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


BOOST_AUTO_TEST_CASE(get_current_setting)
{
    UsblParser usblParser;
    string buffer(peer1_8);
    BOOST_REQUIRE_EQUAL(1, usblParser.parseCurrentSettings(buffer).imRetry);
}

BOOST_AUTO_TEST_CASE(get_current_setting_with_pending_sets)
{
    UsblParser usblParser;
    string buffer = "[*]";
    buffer += string(peer1_8);
    BOOST_REQUIRE_EQUAL(1, usblParser.parseCurrentSettings(buffer).imRetry);
}

BOOST_AUTO_TEST_CASE(get_current_setting_with_pending_sets_case_2)
{
    UsblParser usblParser;
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
    BOOST_REQUIRE_EQUAL(1, usblParser.parseCurrentSettings(settings).imRetry);
}

BOOST_AUTO_TEST_CASE(get_position)
{
    UsblParser usblParser;
    string position = "USBLLONG,1464207778.381274,1464207778.075953,2,13.6015,3.1369,1.7311,13.6015,3.1369,1.7311,0.0000,-0.0000,0.0000,9377,-54,106,0.1698\r\n";
    Position pose = usblParser.parsePosition(position);
    BOOST_REQUIRE_EQUAL(pose.rssi, -54 );
    BOOST_REQUIRE_EQUAL(pose.integrity, 106 );
    BOOST_REQUIRE_EQUAL(pose.accuracy, 0.1698 );
    BOOST_REQUIRE_EQUAL(pose.propagationTime.toMicroseconds(), 9377 );
    BOOST_REQUIRE_EQUAL(pose.time.toSeconds(), 1464207778.381274 );
    BOOST_REQUIRE_EQUAL(pose.measurementTime.toSeconds(), 1464207778.075953 );
    BOOST_REQUIRE_EQUAL(pose.remoteAddress, 2 );
    BOOST_REQUIRE_EQUAL(pose.x, 13.6015);
    BOOST_REQUIRE_EQUAL(pose.y, 3.1369);
    BOOST_REQUIRE_EQUAL(pose.z, 1.7311);
}

BOOST_AUTO_TEST_CASE(get_multipath)
{
    UsblParser usblParser;
    string buffer(peer1_5);
    BOOST_REQUIRE_EQUAL(10, usblParser.parseMultipath(buffer).at(0).signalIntegrity);
}

BOOST_AUTO_TEST_CASE(get_number)
{
    UsblParser usblParser;
    string buffer = "65";
    BOOST_REQUIRE_EQUAL(65, usblParser.getNumber(buffer));
}

BOOST_AUTO_TEST_CASE(get_number_with_text)
{
    UsblParser usblParser;
    string buffer = "65ABC";
    BOOST_REQUIRE_EQUAL(65, usblParser.getNumber(buffer));
}

BOOST_AUTO_TEST_CASE(try_to_get_text_instead_of_number)
{
    UsblParser usblParser;
    string buffer = "ABC";
    BOOST_REQUIRE_THROW(usblParser.getNumber(buffer),runtime_error);
}

BOOST_AUTO_TEST_CASE(get_number_with_pending_set)
{
    UsblParser usblParser;
    string buffer = "[*]765";
    BOOST_REQUIRE_EQUAL(765, usblParser.getNumber(buffer));
}

BOOST_AUTO_TEST_CASE(get_double)
{
    UsblParser usblParser;
    string buffer = "65.76";
    BOOST_REQUIRE_EQUAL(65.76, usblParser.getDouble(buffer));
}

BOOST_AUTO_TEST_CASE(get_double_with_text)
{
    UsblParser usblParser;
    string buffer = "65.51ABC";
    BOOST_REQUIRE_EQUAL(65.51, usblParser.getDouble(buffer));
}

BOOST_AUTO_TEST_CASE(try_to_get_text_instead_of_double)
{
    UsblParser usblParser;
    string buffer = "ABC";
    BOOST_REQUIRE_THROW(usblParser.getDouble(buffer),invalid_argument);
}

BOOST_AUTO_TEST_CASE(get_double_with_pending_set)
{
    UsblParser usblParser;
    string buffer = "[*]7.65";
    BOOST_REQUIRE_EQUAL(7.65, usblParser.getDouble(buffer));
}

BOOST_AUTO_TEST_CASE(get_uLLongInt)
{
    UsblParser usblParser;
    string buffer = "65876786";
    BOOST_REQUIRE_EQUAL(65876786, usblParser.getULLongInt(buffer));
}

BOOST_AUTO_TEST_CASE(get_uLLongInt_with_text)
{
    UsblParser usblParser;
    string buffer = "65876786ABC";
    BOOST_REQUIRE_EQUAL(65876786, usblParser.getULLongInt(buffer));
}

BOOST_AUTO_TEST_CASE(try_to_get_text_instead_of_uLLongInt)
{
    UsblParser usblParser;
    string buffer = "ABC";
    BOOST_REQUIRE_THROW(usblParser.getULLongInt(buffer),runtime_error);
}

BOOST_AUTO_TEST_CASE(get_uLLongInt_with_pending_set)
{
    UsblParser usblParser;
    string buffer = "[*]65876786";
    BOOST_REQUIRE_EQUAL(65876786, usblParser.getULLongInt(buffer));
}

BOOST_AUTO_TEST_CASE(get_answer_content)
{
    UsblParser usblParser;
    string buffer(peer1_6);
    string buffer1(peer1_5);
    BOOST_REQUIRE_EQUAL(buffer1, usblParser.getAnswerContent(buffer));
}

BOOST_AUTO_TEST_CASE(get_a_fuzzy_message_corret)
{
    UsblParser usblParser;
    stringstream ss;
    string buffer("+++AT:34:RECVIM,2,1,2,ack,312,14,11,0.03,36");
    ss << buffer << 0x0D0A; // << 0x10 << 0x02 << 0xff ;
    //BOOST_REQUIRE_THROW(usblParser.splitValidateNotification(buffer, RECVIM), std::runtime_error);
    BOOST_REQUIRE_NO_THROW(usblParser.splitValidateNotification(buffer, RECVIM));
}

BOOST_AUTO_TEST_CASE(get_a_fuzzy_message_colon)
{
    UsblParser usblParser;
    stringstream ss;
    string buffer("+++AT:34:RECVIM,2,1,2,ack,312,14,11,0.03,:6");
    ss << buffer << 0x0D0A; // << 0x10 << 0x02 << 0xff ;
    //BOOST_REQUIRE_THROW(usblParser.splitValidateNotification(buffer, RECVIM), std::runtime_error);
    BOOST_REQUIRE_NO_THROW(usblParser.splitValidateNotification(buffer, RECVIM));
}
BOOST_AUTO_TEST_CASE(get_a_fuzzy_message_comma)
{
    UsblParser usblParser;
    stringstream ss;
    string buffer("+++AT:34:RECVIM,2,1,2,ack,312,14,11,0.03,,6");
    ss << buffer << 0x0D0A; // << 0x10 << 0x02 << 0xff ;
    //BOOST_REQUIRE_THROW(usblParser.splitValidateNotification(buffer, RECVIM), std::runtime_error);
    BOOST_REQUIRE_NO_THROW(usblParser.splitValidateNotification(buffer, RECVIM));
}
BOOST_AUTO_TEST_CASE(get_a_fuzzy_message_endline)
{
    UsblParser usblParser;
    stringstream ss;
    string buffer("+++AT:34:RECVIM,2,1,2,ack,312,14,11,0.03,");
    char end_line[] = {0x0d, 0x0a};
    char msg[] = {0x20, 0x35};
    //ss << buffer << hex << 0x0D0A; // << 0x10 << 0x02 << 0xff ;
    ss << buffer << msg << end_line; // << 0x10 << 0x02 << 0xff ;
    BOOST_REQUIRE_NO_THROW(usblParser.splitValidateNotification(ss.str(), RECVIM));
    //BOOST_REQUIRE_NO_THROW(usblParser.splitValidateNotification(buffer, RECVIM));
}

BOOST_AUTO_TEST_CASE(get_a_fuzzy_message_corret_command)
{
    UsblParser usblParser;
    stringstream ss;
    string buffer("RECVIM,2,1,2,ack,312,14,11,0.03,36");
    ss << buffer << 0x0D0A; // << 0x10 << 0x02 << 0xff ;
    //BOOST_REQUIRE_THROW(usblParser.splitValidateNotification(buffer, RECVIM), std::runtime_error);
    BOOST_REQUIRE_NO_THROW(usblParser.splitValidateNotification(buffer, RECVIM));
}

BOOST_AUTO_TEST_CASE(get_a_fuzzy_message_colon_command)
{
    UsblParser usblParser;
    stringstream ss;
    string buffer("RECVIM,2,1,2,ack,312,14,11,0.03,:6");
    ss << buffer << 0x0D0A; // << 0x10 << 0x02 << 0xff ;
    //BOOST_REQUIRE_THROW(usblParser.splitValidateNotification(buffer, RECVIM), std::runtime_error);
    BOOST_REQUIRE_NO_THROW(usblParser.splitValidateNotification(buffer, RECVIM));
}
BOOST_AUTO_TEST_CASE(get_a_fuzzy_message_comma_command)
{
    UsblParser usblParser;
    stringstream ss;
    string buffer("RECVIM,2,1,2,ack,312,14,11,0.03,,6");
    ss << buffer << 0x0D0A; // << 0x10 << 0x02 << 0xff ;
    //BOOST_REQUIRE_THROW(usblParser.splitValidateNotification(buffer, RECVIM), std::runtime_error);
    BOOST_REQUIRE_NO_THROW(usblParser.splitValidateNotification(buffer, RECVIM));
}
BOOST_AUTO_TEST_CASE(get_a_fuzzy_message_endline_command)
{
    UsblParser usblParser;
    stringstream ss;
    string buffer("RECVIM,2,1,2,ack,312,14,11,0.03,");
    char end_line[] = {0x0d, 0x0a};
    char msg[] = {0x20, 0x35};
    //ss << buffer << hex << 0x0D0A; // << 0x10 << 0x02 << 0xff ;
    ss << buffer << msg << end_line; // << 0x10 << 0x02 << 0xff ;
    BOOST_REQUIRE_NO_THROW(usblParser.splitValidateNotification(ss.str(), RECVIM));
    //BOOST_REQUIRE_NO_THROW(usblParser.splitValidateNotification(buffer, RECVIM));
}


BOOST_AUTO_TEST_CASE(get_answer_content_corret)
{
    UsblParser usblParser;
    stringstream ss;
    string buffer("+++AT:34:RECVIM,2,1,2,ack,312,14,11,0.03,36");
    string content("RECVIM,2,1,2,ack,312,14,11,0.03,36");
    ss << buffer << 0x0D0A; // << 0x10 << 0x02 << 0xff ;
    BOOST_REQUIRE_EQUAL(content, usblParser.getAnswerContent(buffer));
}

BOOST_AUTO_TEST_CASE(get_answer_content_colon)
{
    UsblParser usblParser;
    stringstream ss;
    string buffer("+++AT:34:RECVIM,2,1,2,ack,312,14,11,0.03,:6");
    string content("RECVIM,2,1,2,ack,312,14,11,0.03,:6");
    ss << buffer << 0x0D0A; // << 0x10 << 0x02 << 0xff ;
    BOOST_REQUIRE_EQUAL(content, usblParser.getAnswerContent(buffer));
}

BOOST_AUTO_TEST_CASE(get_answer_content_endline_command)
{
    UsblParser usblParser;
    stringstream ss;
    stringstream ss2;
    string buffer("+++AT:34:RECVIM,2,1,2,ack,312,14,11,0.03,");
    string content("RECVIM,2,1,2,ack,312,14,11,0.03,");
    char end_line[] = {0x0d, 0x0a};
    char msg[] = {0x20, 0x35};
    ss << buffer << msg << end_line; // << 0x10 << 0x02 << 0xff ;
    ss2 << content << msg << end_line; // << 0x10 << 0x02 << 0xff ;
    BOOST_REQUIRE_EQUAL(ss2.str(), usblParser.getAnswerContent(ss.str()));
}

BOOST_AUTO_TEST_CASE(get_validate_answer_content_endline_command)
{
    UsblParser usblParser;
    stringstream ss;
    stringstream ss2;
    string command("+++AT*SENDIM,8,1,ack,test1234");
    string buffer("+++AT*SENDIM:2:OK");
    string content("OK");
    char end_line[] = {0x0d, 0x0a};
    ss << buffer << end_line;
    ss2 << content << end_line;
    BOOST_REQUIRE_EQUAL(ss2.str(), usblParser.getAnswerContent(ss.str(),command));
}

BOOST_AUTO_TEST_CASE(get_receveid_message)
{
    UsblParser usblParser;
    char msg[] = { 0x31, 0x32, 0x00, 0x30, 0x35};
    char end_line[] = { 0x0d, 0x0a};

    ReceiveIM im;
    im.buffer = vector<uint8_t>(msg, msg+5);
    im.deliveryReport = true;
    im.destination = 2;
    im.duration = base::Time::fromMicroseconds(312);
    im.integrity = 11;
    im.rssi = 14;
    im.source = 1;
    im.time = base::Time::now();
    im.velocity = 0.03;

    string buffer = "+++AT:37:RECVIM,5,1,2,ack,312,14,11,0.03,";

    vector<uint8_t> vec_buffer(buffer.begin(), buffer.end());
    vec_buffer.insert(vec_buffer.end(), msg, msg+5);
    vec_buffer.insert(vec_buffer.end(), end_line, end_line+2);
    string sbuffer(vec_buffer.begin(), vec_buffer.end());

    vector<uint8_t> got_im = usblParser.parseReceivedIM(sbuffer).buffer;
    BOOST_REQUIRE_EQUAL(im.buffer.size(), got_im.size());
    for( size_t i=0; i<im.buffer.size(); i++)
        BOOST_REQUIRE_EQUAL(im.buffer[i], got_im[i]);
}

BOOST_AUTO_TEST_CASE(get_receveid_message_zero)
{
    UsblParser usblParser;
    stringstream ss;
    ReceiveIM im;

    im.deliveryReport = true;
    im.destination = 2;
    im.duration = base::Time::fromMicroseconds(312);
    im.integrity = 11;
    im.rssi = 14;
    im.source = 1;
    im.time = base::Time::now();
    im.velocity = 0.03;

    string buffer("+++AT:37:RECVIM,5,1,2,ack,312,14,11,0.03,");
    char msg[] = { 0x31, 0x32, 0x01, 0x00, 0x35};
    string end_line("\r\n");
    im.buffer = vector<uint8_t>(msg, msg+5);
    //char end_line[2] = {0x0d, 0x0a};
    ss << buffer << string(msg, msg+5) << end_line;

    vector<uint8_t> got_im = usblParser.parseReceivedIM(ss.str()).buffer;
    BOOST_REQUIRE_EQUAL(im.buffer.size(), got_im.size());
    for( size_t i=0; i<im.buffer.size(); i++)
        BOOST_REQUIRE_EQUAL(im.buffer[i], got_im[i]);
}

BOOST_AUTO_TEST_CASE(parse_send_message)
{
    UsblParser usblParser;
    stringstream ss;
    SendIM im;
    im.deliveryReport = true;
    im.destination = 1;
    char msg[] = { 0x31, 0x32, 0x33, 0x34, 0x35 };

    im.buffer = vector<uint8_t>(msg, msg+5);
    string buffer = "AT*SENDIM,5,1,ack,";

    ss << buffer << string(msg, msg+5);

    BOOST_REQUIRE_EQUAL(ss.str(), usblParser.parseSendIM(im));
}

BOOST_AUTO_TEST_CASE(parse_send_message_with_zero)
{
    UsblParser usblParser;
    stringstream ss;
    SendIM im;
    im.deliveryReport = true;
    im.destination = 1;
    char msg[] = { 0x31, 0x00, 0x33, 0x00, 0x35 };

    im.buffer = vector<uint8_t>(msg, msg+5);
    string buffer = "AT*SENDIM,5,1,ack,";

    ss << buffer << string(msg, msg+5);
    string result = usblParser.parseSendIM(im);

    BOOST_REQUIRE_EQUAL(msg[4], result.at(result.size()-1));
    BOOST_REQUIRE_EQUAL(ss.str(), usblParser.parseSendIM(im));
}

BOOST_AUTO_TEST_CASE(test_splitMinimalValidate)
{
    UsblParser usblParser;
    stringstream ss;

    string buffer("+++AT:37:RECVIM,5,1,2,ack,312,14,11,0.03,");
    char msg[] = { 0x31, 0x32, 0x01, 0x00, 0x35};
    string end_line("\r\n");
    ss << buffer << string(msg, msg+5) << end_line;

    vector<string> test1 = usblParser.splitMinimalValidate(ss.str(),":", 3);
    BOOST_REQUIRE_EQUAL(test1.size(), 3);
    BOOST_REQUIRE_EQUAL(test1[0], "+++AT");
    BOOST_REQUIRE_EQUAL(test1[1], "37");
    BOOST_REQUIRE_EQUAL(test1[2], "RECVIM,5,1,2,ack,312,14,11,0.03," + string(msg, msg+5) + end_line);

    vector<string> test2 = usblParser.splitMinimalValidate(ss.str(),":", 2);
    BOOST_REQUIRE_EQUAL(test2.size(), 2);
    BOOST_REQUIRE_EQUAL(test2[0], "+++AT");
    BOOST_REQUIRE_EQUAL(test2[1], "37:RECVIM,5,1,2,ack,312,14,11,0.03," + string(msg, msg+5) + end_line);

    vector<string> test3 = usblParser.splitMinimalValidate(ss.str(),",", 3);
    BOOST_REQUIRE_EQUAL(test3.size(), 3);
    BOOST_REQUIRE_EQUAL(test3[0], "+++AT:37:RECVIM");
    BOOST_REQUIRE_EQUAL(test3[1], "5");
    BOOST_REQUIRE_EQUAL(test3[2], "1,2,ack,312,14,11,0.03," + string(msg, msg+5) + end_line);

    vector<string> test4 = usblParser.splitMinimalValidate(ss.str(),",", 1);
    BOOST_REQUIRE_EQUAL(test4.size(), 1);
    BOOST_REQUIRE_EQUAL(test4[0], ss.str());

    BOOST_REQUIRE_THROW(usblParser.splitMinimalValidate(ss.str(),"&", 2), runtime_error);

}

BOOST_AUTO_TEST_SUITE_END();
