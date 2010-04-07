/* ************************************************************************** *
 * SUA Message properties: SCCP transmission parameters
 * ************************************************************************** */
#ifndef __EYELINE_SS7NA_LIBSCCP_MESSAGEPROPERTIES_HPP__
#ident "@(#)$Id$"
# define __EYELINE_SS7NA_LIBSCCP_MESSAGEPROPERTIES_HPP__

#include <inttypes.h>

namespace eyeline {
namespace ss7na {
namespace libsccp {

class MessageProperties {
protected:
  enum FieldMaskBit_e {
    bit_SEQUENCE_CONTROL = 0x01, bit_IMPORTANCE = 0x02, bit_HOP_COUNT = 0x04
  };

  uint8_t _fieldsMask;
  bool    _returnOnError;   //mandatory, by default is true
  uint8_t _importance;      //optional, by default not used
  uint8_t _hopCount;        //optional, by default not used
  uint32_t _sequenceControlValue; //optional, used to indicate that
                                  //'in-sequence delivery' is required and
                                  //select appropriate SLS

public:
  MessageProperties()
    : _fieldsMask(0), _returnOnError(true), _importance(0), _hopCount(0)
    , _sequenceControlValue(0)
  { }

  bool getReturnOnError(void) const { return _returnOnError; }
  void setReturnOnError(bool do_return = true) { _returnOnError = do_return; }

  bool hasHopCount(void) const { return (_fieldsMask & bit_HOP_COUNT) != 0; }
  uint8_t getHopCount(void) const { return _hopCount; }
  
  bool hasImportance(void) const { return (_fieldsMask & bit_IMPORTANCE) != 0; }
  uint8_t getImportance(void) const { return _hopCount; }

  bool hasSequenceControl(void) const { return (_fieldsMask & bit_SEQUENCE_CONTROL) != 0; }
  uint32_t getSequenceControl(void) const { return _hopCount; }

  void setHopCount(uint8_t use_count) { _hopCount = use_count; _fieldsMask |= bit_HOP_COUNT; }
  void setImportance(uint8_t use_count) { _hopCount = use_count; _fieldsMask |= bit_IMPORTANCE; }
  void setSequenceControl(uint32_t use_val)
  {
    _sequenceControlValue = use_val; _fieldsMask |= bit_SEQUENCE_CONTROL;
  }
};

}}}

#endif

