#include <boost/test/unit_test.hpp>
#include <usbl_evologics/Driver.hpp>

using namespace usbl_evologics;
using namespace std;

BOOST_AUTO_TEST_SUITE(Driver_extractRawFromATPacket)

BOOST_AUTO_TEST_CASE(it_should_find_a_simple_AT_packet_at_the_beginning_of_the_buffer)
{
    Driver driver;
    string buffer("+++AT:5:12345\r\n");
    BOOST_REQUIRE_EQUAL(15, driver.extractRawFromATPackets(buffer));
}

BOOST_AUTO_TEST_CASE(no_end_of_line_present)
{
    Driver driver;
    string buffer("+++AT:5:12345ab\r\n");
    BOOST_REQUIRE_THROW(driver.extractRawFromATPackets(buffer),runtime_error);
//    BOOST_REQUIRE_EQUAL(-1, driver.extractRawFromATPackets(buffer));
}

BOOST_AUTO_TEST_CASE(AT_packet_with_wrong_length)
{
    Driver driver;
    string buffer("+++AT:30:12345\r\n");
    BOOST_REQUIRE_EQUAL(0, driver.extractRawFromATPackets(buffer));
}

BOOST_AUTO_TEST_CASE(it_should_interpret_everything_up_to_the_at_packet_as_raw_data)
{
    Driver driver;
    string buffer("12345+++AT:5:12345\r\n");
    BOOST_REQUIRE_EQUAL(5, driver.extractRawFromATPackets(buffer));
}

BOOST_AUTO_TEST_CASE(it_should_interpret_everything_up_to_the_at_packet_as_raw_data_even_with_plus)
{
    Driver driver;
    string buffer("12345++++AT:5:12345\r\n");
    BOOST_REQUIRE_EQUAL(6, driver.extractRawFromATPackets(buffer));
}

BOOST_AUTO_TEST_CASE(it_should_interpret_everything_up_to_the_at_packet_as_raw_data_even_with_2plus)
{
    Driver driver;
    string buffer("12345+++++AT:5:12345\r\n");
    BOOST_REQUIRE_EQUAL(7, driver.extractRawFromATPackets(buffer));
}

BOOST_AUTO_TEST_CASE(it_should_interpret_everything_up_to_the_at_packet_as_raw_data_even_with_3plus)
{
    Driver driver;
    string buffer("12345++++++AT:5:12345\r\n");
    BOOST_REQUIRE_EQUAL(8, driver.extractRawFromATPackets(buffer));
}

BOOST_AUTO_TEST_CASE(it_should_interpret_everything_up_to_the_at_packet_as_raw_data_of_plusA)
{
    Driver driver;
    string buffer("+++A+++A++++++AT:5:12345\r\n");
    BOOST_REQUIRE_EQUAL(11, driver.extractRawFromATPackets(buffer));
}

BOOST_AUTO_TEST_CASE(it_should_try_to_extract_a_raw_packet_before_a_possible_at_packet_at_the_end_of_buffer_even_with_plusA)
{
    Driver driver;
    string buffer("+++A+++A+++.++AT+");
    BOOST_REQUIRE_EQUAL(16, driver.extractRawFromATPackets(buffer));
}
BOOST_AUTO_TEST_CASE(it_should_try_to_extract_a_raw_packet_before_a_possible_at_packet_at_the_end_of_buffer)
{
    Driver driver;
    string buffer("12345++");
    BOOST_REQUIRE_EQUAL(5, driver.extractRawFromATPackets(buffer));
}

BOOST_AUTO_TEST_CASE(it_should_not_be_confused_by_a_plus_sign_in_the_middle_of_buffer)
{
    Driver driver;
    string buffer("123++12");
    BOOST_REQUIRE_EQUAL(7, driver.extractRawFromATPackets(buffer));
}

BOOST_AUTO_TEST_CASE(it_should_not_be_confused_by_a_0x00_byte_in_the_middle_of_buffer)
{
    Driver driver;
    //string buffer("123++12");
    string buffer("+++AT:37:RECVIM,5,1,2,ack,312,14,11,0.03,");
    char msg[] = { 0x31, 0x32, 0x01, 0x00, 0x35};
    string end_line("\r\n");
    stringstream ss;
    ss << buffer << string(msg, msg+5) << end_line;
    BOOST_REQUIRE_EQUAL(48, driver.extractRawFromATPackets(ss.str()));
}

BOOST_AUTO_TEST_CASE(it_should_not_be_confused_by_a_received_message_inception_with_0x00_byte)
{
    Driver driver;
    //string buffer("123++12");
    string buffer("+++AT");
    string splitter(":");
    string IM("RECVIM,5,1,2,ack,312,14,11,0.03,");
    char msg[] = { 0x31, 0x32, 0x01, 0x00, 0x35};
    string end_line("\r\n");
    stringstream inception;
    inception << IM << buffer << splitter << to_string(IM.size()+ 5) << splitter << IM << string(msg, msg+5) << end_line;
    stringstream ss;
    ss << buffer << splitter << to_string(inception.str().size()) << splitter << inception.str() << end_line;
    BOOST_REQUIRE_EQUAL(ss.str().size(), driver.extractATPacket(ss.str()));
}

BOOST_AUTO_TEST_CASE(it_should_not_be_confused_by_a_received_message_with_other_Notification_string_on_it)
{
    Driver driver;
    string buffer("+++AT");
    string splitter(":");
    string IM("RECVIM,5,1,2,ack,312,14,11,0.03,");
    //char msg[] = { 0x31, 0x32, 0x01, 0x00, 0x35};
    string other_notification(("+++AT:118:USBLLONG,455.600823,455.346033,1,1.2601,0.9197,-0.3958,1.2601,0.9197,-0.3958,0.0000,-0.0000,0.0000,1073,-37,146,0.0022\r\n"));
    string end_line("\r\n");
    stringstream inception;
    inception << IM << other_notification;
    stringstream ss;
    ss << buffer << splitter << to_string(inception.str().size()) << splitter << inception.str() << end_line;
    BOOST_REQUIRE_EQUAL(ss.str().size(), driver.extractATPacket(ss.str()));
    BOOST_REQUIRE_EQUAL(RECVIM, driver.isNotification(ss.str()));
}

BOOST_AUTO_TEST_CASE(it_should_get_a_Message_Notification_in_command_mode)
{
    Driver driver;
    //string buffer("123++12");
    string buffer("RECVIM,5,1,2,ack,312,14,11,0.03,");
    char msg[] = { 0x31, 0x32, 0x01, 0x00, 0x35};
    string end_line("\r\n");
    stringstream ss;
    ss << buffer << string(msg, msg+5) << end_line;
    BOOST_REQUIRE_EQUAL(ss.str().size(), driver.checkNotificationCommandMode(ss.str()));
}

BOOST_AUTO_TEST_CASE(it_should_wait_a_Message_Notification_in_command_mode)
{
    Driver driver;
    //string buffer("123++12");
    string buffer("RECVIM,5,1,2,ack,312,14,11,0.03");
    BOOST_REQUIRE_EQUAL(0, driver.checkNotificationCommandMode(buffer));
}

BOOST_AUTO_TEST_CASE(it_should_get_a_Message_Notification_in_command_mode_with_endline_in_msg)
{
    Driver driver;
    //string buffer("123++12");
    string buffer("RECVIM,7,1,2,ack,312,14,11,0.03,");
    char msg[] = { 0x31, 0x32, 0x01, 0x00, 0x35};
    string end_line("\r\n");
    stringstream ss;
    ss << buffer << string(msg, msg+5) << end_line << end_line;
    BOOST_REQUIRE_EQUAL(ss.str().size(), driver.checkNotificationCommandMode(ss.str()));
}

BOOST_AUTO_TEST_CASE(it_should_crash_in_getting_a_Message_Notification_in_command_mode_with_wrong_size)
{
    Driver driver;
    //string buffer("123++12");
    string buffer("RECVIM,1,1,2,ack,312,14,11,0.03,");
    char msg[] = { 0x31, 0x32, 0x01, 0x00, 0x35};
    string end_line("\r\n");
    stringstream ss;
    ss << buffer << string(msg, msg+5) << end_line;
    BOOST_REQUIRE_THROW(driver.checkNotificationCommandMode(ss.str()),runtime_error);
}

BOOST_AUTO_TEST_CASE(it_should_crash_in_getting_a_Notification_in_command_mode_with_a_long_filed_beetween_commas)
{
    Driver driver;
    //string buffer("123++12");
    string buffer("RECVIM,1,1,2,ack,3128775663861479831,14,11,0.03,");
    char msg[] = { 0x31, 0x32, 0x01, 0x00, 0x35};
    string end_line("\r\n");
    stringstream ss;
    ss << buffer << string(msg, msg+5) << end_line;
    BOOST_REQUIRE_THROW(driver.checkNotificationCommandMode(ss.str()),runtime_error);
}

BOOST_AUTO_TEST_CASE(it_should_wait_a_Notification_in_command_mode_missing_a_comma)
{
    Driver driver;
    //string buffer("123++12");
    string buffer("RECVIM,1,1,2,ack,312,1411,0.03,");
    char msg[] = { 0x31, 0x32, 0x01, 0x00, 0x35};
    string end_line("\r\n");
    stringstream ss;
    ss << buffer << string(msg, msg+5) << end_line;
    BOOST_REQUIRE_EQUAL(0, driver.checkNotificationCommandMode(ss.str()));
}

BOOST_AUTO_TEST_CASE(it_should_get_a_Message_Notification_in_command_mode_followed_by_other_notification)
{
    Driver driver;
    //string buffer("123++12");
    string buffer("RECVIM,6,1,2,ack,312,14,11,0.03,,");
    char msg[] = { 0x31, 0x32, 0x01, 0x00, 0x35};
    string end_line("\r\n");
    stringstream ss;
    ss << buffer << string(msg, msg+5) << end_line;
    ss << ss.str();
    BOOST_REQUIRE_EQUAL(ss.str().size()/2, driver.checkNotificationCommandMode(ss.str()));
}

BOOST_AUTO_TEST_CASE(it_should_get_a_Message_Notification_in_command_mode_in_inception_case)
{
    Driver driver;
    //string buffer("123++12");
    string notification("RECVIM,");
    string parameters(",1,2,ack,312,14,11,0.03,");
    char msg[] = { 0x31, 0x32, 0x01, 0x00, 0x35};
    string end_line("\r\n");
    stringstream message;
    message << notification << "5" << parameters << string(msg, msg+5) << end_line;
    stringstream ss;
    ss << notification << to_string(message.str().size()) << parameters << message.str() << end_line;
    BOOST_REQUIRE_EQUAL(ss.str().size(), driver.checkNotificationCommandMode(ss.str()));
}

BOOST_AUTO_TEST_CASE(it_ready_a_Notification_from_a_Message)
{
    Driver driver;
    string buffer("RECVEND,3349740869,182272,-40,120,0.1000");
    //char msg[] = { 0x31, 0x32, 0x01, 0x00, 0x35};
    string end_line("\r\n");
    stringstream ss;
    ss << buffer << end_line;
    BOOST_REQUIRE_EQUAL(ss.str().size(), driver.checkNotificationCommandMode(ss.str()));
}



BOOST_AUTO_TEST_SUITE_END();
