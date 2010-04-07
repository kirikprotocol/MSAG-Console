#include <stdio.h>
#include "eyeline/utilx/Exception.hpp"
#include "eyeline/ss7na/common/TP.hpp"
#include "N_COORD_IND_Message.hpp"

namespace eyeline {
namespace ss7na {
namespace libsccp {

N_COORD_IND_Message::N_COORD_IND_Message()
  : LibsccpMessage(_MSG_CODE),
    _fieldsMask(0), _pointCode(0), _ssn(0),
    _subsystemMultiplicityIndicator(0)
{}

size_t
N_COORD_IND_Message::serialize(common::TP* result_buf) const
{
  size_t offset = LibsccpMessage::serialize(result_buf);

  offset = common::addField(result_buf, offset, _fieldsMask);

  offset = common::addField(result_buf, offset, _pointCode);

  offset = common::addField(result_buf, offset, _ssn);

  if ( _fieldsMask & SET_SUBSYSTEM_MULTIPLICITY_IND )
    offset = common::addField(result_buf, offset, _subsystemMultiplicityIndicator);

  return offset;
}

size_t
N_COORD_IND_Message::serialize(uint8_t* result_buf, size_t result_buf_max_sz) const
{
  common::TP tp(0, 0, result_buf, result_buf_max_sz);
  return serialize(&tp);
}

size_t
N_COORD_IND_Message::deserialize(const common::TP& packet_buf)
{
  size_t offset = LibsccpMessage::deserialize(packet_buf);

  offset = common::extractField(packet_buf, offset, &_fieldsMask);

  offset = common::extractField(packet_buf, offset, &_pointCode);

  offset = common::extractField(packet_buf, offset, &_ssn);

  if ( _fieldsMask & SET_SUBSYSTEM_MULTIPLICITY_IND )
    offset = common::extractField(packet_buf, offset, &_subsystemMultiplicityIndicator);

  return offset;
}

size_t
N_COORD_IND_Message::deserialize(const uint8_t* packet_buf, size_t packet_buf_sz)
{
  common::TP tp(0, packet_buf_sz, const_cast<uint8_t*>(packet_buf), packet_buf_sz);

  return deserialize(tp);
}

std::string
N_COORD_IND_Message::toString() const
{
  std::string result(LibsccpMessage::toString());

  char strBuf[128];
  snprintf(strBuf, sizeof(strBuf), ",fieldsMask=[%02X]", _fieldsMask);
  result += strBuf;

  snprintf(strBuf, sizeof(strBuf), ",pointCode=[%d]", _pointCode);
  result += strBuf;

  snprintf(strBuf, sizeof(strBuf), ",ssn=[%d]", _ssn);
  result += strBuf;

  if ( _fieldsMask & SET_SUBSYSTEM_MULTIPLICITY_IND ) {
    snprintf(strBuf, sizeof(strBuf), ",subsystemMultiplicityIndicator=[%d]", _subsystemMultiplicityIndicator);
    result += strBuf;
  }

  return result;
}

void
N_COORD_IND_Message::setSubsystemMultiplicityInd(uint8_t subsystem_mult_ind)
{
  _subsystemMultiplicityIndicator = subsystem_mult_ind;
  _fieldsMask |= SET_SUBSYSTEM_MULTIPLICITY_IND;
}

uint8_t
N_COORD_IND_Message::getSubsystemMultiplicityInd() const
{
  if ( _fieldsMask & SET_SUBSYSTEM_MULTIPLICITY_IND )
    return _subsystemMultiplicityIndicator;
  else
    throw utilx::FieldNotSetException("N_COORD_IND_Message::getSubsystemMultiplicityInd::: subsystemMultiplicityIndicator wasn't set");
}

uint32_t
N_COORD_IND_Message::getLength() const
{
  return LibsccpMessage::getLength() + 
    static_cast<uint32_t>(sizeof(_fieldsMask) + sizeof(_pointCode) + sizeof(_ssn) +
                          ( (_fieldsMask & SET_SUBSYSTEM_MULTIPLICITY_IND) ? sizeof(_subsystemMultiplicityIndicator) : 0 ));
}

}}}
