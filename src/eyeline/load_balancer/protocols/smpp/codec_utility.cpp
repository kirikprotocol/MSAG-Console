#include "eyeline/utilx/Exception.hpp"
#include "codec_utility.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

size_t
extractVariableCOctetString(const io_subsystem::Packet* packet, size_t offset,
                            char* value, size_t maxValueSize)
{
  if ( !maxValueSize ) return offset;

  size_t restValueSize = maxValueSize;

  while (restValueSize > 0) {
    offset = packet->extractValue(reinterpret_cast<uint8_t*>(value), offset);
    if (!*value) return offset;
    ++value; --restValueSize;
  }

  if (!restValueSize && *(value-1))
    throw utilx::DeserializationException("extractCOctetString::: cstring value is too long, expected max value=%d", maxValueSize);

  return offset;
}

size_t
extractFixedCOctetString(const io_subsystem::Packet* packet, size_t offset,
                         char* value, size_t expectedValueSize)
{
  if ( !expectedValueSize ) return offset;

  offset = packet->extractValue(reinterpret_cast<uint8_t*>(value), offset);

  if ( !*value )
    return offset;
  else
    ++value;

  size_t restValueSize = expectedValueSize - 1;

  while (restValueSize > 0) {
    offset = packet->extractValue(reinterpret_cast<uint8_t*>(value), offset);
    if (--restValueSize && !*value++)
      throw utilx::DeserializationException("extractFixedCOctetString::: cstring is shorter than expected %d octets", expectedValueSize);
  }

  if ( *value )
    throw utilx::DeserializationException("extractFixedCOctetString::: cstring is not NULL terminated");

  return offset;
}

size_t
addCOctetString(io_subsystem::Packet* packet, const char* value, size_t maxValueSize)
{
  char tmpValue;

  if ( value == NULL || maxValueSize == 0 ) {
    return packet->addValue(uint8_t(0));
  }

  while ( --maxValueSize && (tmpValue=*value++) ) {
    packet->addValue(static_cast<uint8_t>(tmpValue));
  }

  return packet->addValue(uint8_t(0));
}

}}}}

#ifdef __TEST_UNIT__

int main()
{
  try {
    eyeline::load_balancer::io_subsystem::Packet packet;

    uint8_t test_variable_coctet_string [] = { 'h', 'e', 'l', 'l', 'o', 0 , 1 , 2, 3};

    memcpy(packet.packet_data, test_variable_coctet_string, sizeof(test_variable_coctet_string));
    packet.packet_data_len = sizeof(test_variable_coctet_string);

    size_t offset;
    char resultString[32];
    offset = eyeline::load_balancer::protocols::smpp::extractVariableCOctetString(&packet, 0, resultString, 6);

    printf("OK: resultString='%s'\n", resultString);
  } catch (std::exception& ex) {
    printf("FAILED: catched exception: [%s]\n", ex.what());
  }

  try {
    eyeline::load_balancer::io_subsystem::Packet packet;

    uint8_t test_variable_coctet_string [] = { 'h', 'e', 'l', 'l',  0 , 1 , 2, 3};

    memcpy(packet.packet_data, test_variable_coctet_string, sizeof(test_variable_coctet_string));
    packet.packet_data_len = sizeof(test_variable_coctet_string);

    size_t offset;
    char resultString[32];
    offset = eyeline::load_balancer::protocols::smpp::extractVariableCOctetString(&packet, 0, resultString, 6);

    printf("OK: resultString='%s'\n", resultString);
  } catch (std::exception& ex) {
    printf("FAILED: catched exception: [%s]\n", ex.what());
  }

  try {
    eyeline::load_balancer::io_subsystem::Packet packet;

    uint8_t test_variable_coctet_string [] = { 'h', 'e', 'l', 'l',  1 , 1 , 2, 3};

    memcpy(packet.packet_data, test_variable_coctet_string, sizeof(test_variable_coctet_string));
    packet.packet_data_len = sizeof(test_variable_coctet_string);

    size_t offset;
    char resultString[32];
    offset = eyeline::load_balancer::protocols::smpp::extractVariableCOctetString(&packet, 0, resultString, sizeof(test_variable_coctet_string));

    printf("FAILED: resultString='%s'\n", resultString);
  } catch (std::exception& ex) {
    printf("OK: caught exception: [%s]\n", ex.what());
  }

  try {
    eyeline::load_balancer::io_subsystem::Packet packet;

    uint8_t test_variable_coctet_string [] = { 0 };

    memcpy(packet.packet_data, test_variable_coctet_string, sizeof(test_variable_coctet_string));
    packet.packet_data_len = sizeof(test_variable_coctet_string);

    size_t offset;
    char resultString[32];
    offset = eyeline::load_balancer::protocols::smpp::extractVariableCOctetString(&packet, 0, resultString, 6);

    printf("OK: empty resultString='%s'\n", resultString);
  } catch (std::exception& ex) {
    printf("FAILED: caught exception: [%s]\n", ex.what());
  }

  try {
    eyeline::load_balancer::io_subsystem::Packet packet;

    uint8_t test_fixed_coctet_string[] = { 'h', 'e', 'l', 'l',  'o', 0, 2, 3};

    memcpy(packet.packet_data, test_fixed_coctet_string, sizeof(test_fixed_coctet_string));
    packet.packet_data_len = sizeof(test_fixed_coctet_string);

    size_t offset;
    char resultString[32];
    offset = eyeline::load_balancer::protocols::smpp::extractFixedCOctetString(&packet, 0, resultString, 6);

    printf("OK: fixed resultString='%s'\n", resultString);
  } catch (std::exception& ex) {
    printf("FAILED: fixed caught exception: [%s]\n", ex.what());
  }

  try {
    eyeline::load_balancer::io_subsystem::Packet packet;

    uint8_t test_fixed_coctet_string[] = { 'h', 'e', 'l', 'l',  'o', '!', 0, 2, 3};

    memcpy(packet.packet_data, test_fixed_coctet_string, sizeof(test_fixed_coctet_string));
    packet.packet_data_len = sizeof(test_fixed_coctet_string);

    size_t offset;
    char resultString[32];
    offset = eyeline::load_balancer::protocols::smpp::extractFixedCOctetString(&packet, 0, resultString, 6);

    printf("FAILED: fixed resultString='%s'\n", resultString);
  } catch (std::exception& ex) {
    printf("OK: fixed caught exception: [%s]\n", ex.what());
  }

  try {
    eyeline::load_balancer::io_subsystem::Packet packet;

    uint8_t test_fixed_coctet_string[] = { 'h', 'e', 'l',  'o', 0, 2, 3};

    memcpy(packet.packet_data, test_fixed_coctet_string, sizeof(test_fixed_coctet_string));
    packet.packet_data_len = sizeof(test_fixed_coctet_string);

    size_t offset;
    char resultString[32];
    offset = eyeline::load_balancer::protocols::smpp::extractFixedCOctetString(&packet, 0, resultString, 6);

    printf("FAILED: fixed resultString='%s'\n", resultString);
  } catch (std::exception& ex) {
    printf("OK: fixed caught exception: [%s]\n", ex.what());
  }

  try {
    eyeline::load_balancer::io_subsystem::Packet packet;

    uint8_t test_fixed_coctet_string[] = { 0, 2, 3};

    memcpy(packet.packet_data, test_fixed_coctet_string, sizeof(test_fixed_coctet_string));
    packet.packet_data_len = sizeof(test_fixed_coctet_string);

    size_t offset;
    char resultString[32];
    offset = eyeline::load_balancer::protocols::smpp::extractFixedCOctetString(&packet, 0, resultString, sizeof(test_fixed_coctet_string));

    printf("OK: fixed empty resultString='%s'\n", resultString);
  } catch (std::exception& ex) {
    printf("FAILED: fixed caught exception: [%s]\n", ex.what());
  }

  try {
    eyeline::load_balancer::io_subsystem::Packet packet;
    const char value[] = { '1', '2', '3' };
    eyeline::load_balancer::protocols::smpp::addCOctetString(&packet, value, sizeof(value));
    const char standardValue[] = { '1', '2', 0 };
    if (!memcmp(standardValue, packet.packet_data, sizeof(standardValue)) &&
        sizeof(standardValue) == packet.packet_data_len)
      printf("OK: addOCteteString('1','2','3')\n");
    else
      printf("addOCteteString('1','2','3') FAILED\n");
  } catch (std::exception& ex) {
    printf("addOCteteString('1','2','3') FAILED: fixed caught exception: [%s]\n", ex.what());
  }

  try {
    eyeline::load_balancer::io_subsystem::Packet packet;
    const char value[] = { '1', '2', 0, '4' };
    eyeline::load_balancer::protocols::smpp::addCOctetString(&packet, value, sizeof(value));
    const char standardValue[] = { '1', '2', 0 };
    if (!memcmp(standardValue, packet.packet_data, sizeof(standardValue)) &&
        sizeof(standardValue) == packet.packet_data_len)
      printf("OK: addOCteteString('1','2',0,'4')\n");
    else
      printf("addOCteteString('1','2',0,'4') FAILED\n");
  } catch (std::exception& ex) {
    printf("addOCteteString('1','2',0,'4') FAILED: fixed caught exception: [%s]\n", ex.what());
  }

  try {
    eyeline::load_balancer::io_subsystem::Packet packet;
    const char *value = "123";
    eyeline::load_balancer::protocols::smpp::addCOctetString(&packet, value, 4);
    const char standardValue[] = { '1', '2', '3', 0 };
    if (!memcmp(standardValue, packet.packet_data, sizeof(standardValue)) &&
        sizeof(standardValue) == packet.packet_data_len)
      printf("OK: addOCteteString(\"123\")\n");
    else
      printf("addOCteteString(\"123\") FAILED\n");
  } catch (std::exception& ex) {
    printf("addOCteteString(\"123\") FAILED: fixed caught exception: [%s]\n", ex.what());
  }

  return 0;
}

#endif
