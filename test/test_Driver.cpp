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
    BOOST_REQUIRE_EQUAL(17, driver.extractRawFromATPackets(buffer));
}

BOOST_AUTO_TEST_CASE(AT_packet_with_wrong_length)
{
    Driver driver;
    string buffer("+++AT:30:12345\r\n");
    BOOST_REQUIRE_EQUAL(15, driver.extractRawFromATPackets(buffer));
}

BOOST_AUTO_TEST_CASE(it_should_interpret_everything_up_to_the_at_packet_as_raw_data)
{
    Driver driver;
    string buffer("12345+++AT:5:12345\r\n");
    BOOST_REQUIRE_EQUAL(5, driver.extractRawFromATPackets(buffer));
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

BOOST_AUTO_TEST_SUITE_END();
