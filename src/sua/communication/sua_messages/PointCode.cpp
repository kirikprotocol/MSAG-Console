#include <string.h>
#include "PointCode.hpp"

namespace eyeline {
namespace sua {
namespace communication {
namespace sua_messages {

PointCode::PointCode(const uint8_t pcValue[])
{
  memcpy(_pcBuf.value, pcValue, sizeof(_pcBuf.value));
}

uint8_t
PointCode::getMask()
{
  return _pcBuf.value[0];
}

const uint8_t*
PointCode::getValue() const
{
  return _pcBuf.value;
}

ANSI_PC::ANSI_PC(const uint8_t pcValue[])
  : PointCode(pcValue) {}

ANSI_PC::ANSI_PC(uint8_t mask, uint8_t network, uint8_t cluster, uint8_t member)
  : PointCode(converter(mask, network, cluster, member).buf)
{}

uint8_t
ANSI_PC::getNetwork() const
{
  const uint8_t* value = getValue();
  return value[1];
}

uint8_t
ANSI_PC::getCluster() const
{
  const uint8_t* value = getValue();
  return value[2];
}

uint8_t
ANSI_PC::getMember() const
{
  const uint8_t* value = getValue();
  return value[3];
}

ITU_PC::ITU_PC(const uint8_t pcValue[])
  : PointCode(pcValue)
{}

ITU_PC::ITU_PC(uint8_t mask, uint8_t zone, uint8_t region, uint8_t sp)
  : PointCode(converter(mask, zone, region, sp).buf)
{}

uint8_t
ITU_PC::getZone() const
{
  const uint8_t* value = getValue();
  return (value[2] >> 3) & 0x07;
}

uint8_t
ITU_PC::getRegion() const
{
  const uint8_t* value = getValue();
  return (value[2] & 0x07) | ((value[3] >> 3) & 0x1F);
}

uint8_t
ITU_PC::getSP() const
{
  const uint8_t* value = getValue();
  return value[3] & 0x07;
}

}}}}
