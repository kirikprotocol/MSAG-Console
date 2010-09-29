/* ************************************************************************** *
 * Alerting Pattern type definition.
 * ************************************************************************** */
#ifndef __EYELINE_MAP_7F0_ALERTING_PATTERN_HPP
#ident "@(#)$Id$"
#define __EYELINE_MAP_7F0_ALERTING_PATTERN_HPP

namespace eyeline {
namespace map {
namespace common {

/* This type is used to represent Alerting Pattern
  --  bits 8765 : 0000 (unused)

  --  bits 43 : type of Pattern
  --            00 level
  --            01 category
  --            10 category
  --            all other values are reserved.

  --  bits 21 : type of alerting
*/
struct AlertingPattern {
  typedef unsigned char value_type;

  enum Id_e {
    // -- Alerting Levels are defined in GSM 02.07
    alertingLevel_0 = 0x00
    , alertingLevel_1 = 0x01
    , alertingLevel_2 = 0x02
    , alertingLevel_Rsrv = 0x03
    // -- Alerting categories are defined in GSM 02.07
    , alertingCategory_1 = 0x04
    , alertingCategory_2 = 0x05
    , alertingCategory_3 = 0x06
    , alertingCategory_4 = 0x07
    , alertingCategory_5 = 0x08
    // -- all other values of Alerting Category are reserved
    , alertingCategory_Rsrv = 0x0F
  };

  value_type _value;

  static Id_e value2Id(value_type use_val);

  Id_e  value2Id(void) const { return value2Id(_value); }
  bool  isLevelPattern(void) const { return (_value & 0x0C) == 0; }
  bool  isCategoryPattern(void) const { return !isLevelPattern() && ((_value & 0x0C) != 0x0C); }


  AlertingPattern() : _value(alertingCategory_Rsrv)
  { }
};

typedef AlertingPattern::value_type AlertingPattern_t;

} //common
} //map
} //eyeline

#endif /* __EYELINE_MAP_7F0_ALERTING_PATTERN_HPP */

