/* ************************************************************************** *
 * Abonent Providers constants definitions.
 * ************************************************************************** */
#ifndef SMSC_INMAN_IAPROVIDER_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_INMAN_IAPROVIDER_DEFS_HPP

#include "core/buffers/FixedLengthString.hpp"

namespace smsc {
namespace inman {
namespace iaprvd { //(I)NMan (A)bonent (P)roviders

struct IAPAbility {
  enum Option_e { abContract = 0x80, abSCF = 0x40, abIMSI = 0x20, abVLR = 0x10, abODB = 0x08 };

  struct AbilityFlags {
    uint8_t contract : 1;
    uint8_t gsmSCF   : 1;
    uint8_t imsi     : 1;
    uint8_t vlrNum   : 1;
    uint8_t odbGD    : 1; //Operator Determined Barring general data
    /* -- */
    uint8_t reserved : 4;
  };

  static const size_t _maxIAPAbilityStrSZ = sizeof("{contract,gsmSCF,IMSI,VLR,ODB}") + 1;
  typedef smsc::core::buffers::FixedLengthString<_maxIAPAbilityStrSZ>
    AbilityStr_t;

  union {
    uint8_t       ui;
    AbilityFlags  st;
  } value;

  IAPAbility()
  {
    value.ui = 0;
  }
  explicit IAPAbility(uint8_t op_flags /* = OR-ed Option_e values */)
  {
    value.ui = 0;
    if (op_flags & abContract)
      value.st.contract = 1;
    if (op_flags & abSCF)
      value.st.gsmSCF = 1;
    if (op_flags & abIMSI)
      value.st.imsi = 1;
    if (op_flags & abVLR)
      value.st.vlrNum = 1;
    if (op_flags & abODB)
      value.st.odbGD = 1;
  }

  AbilityStr_t toString(void) const;

  bool hasOption(Option_e op_id) const
  {
    return value.ui & op_id;
  }
};

typedef IAPAbility::AbilityStr_t IAPAbilityStr_t;


struct IAPProperty {
  static const unsigned _maxIAPIdentSZ = 32;
  static const unsigned _maxIAPPropertyStrSZ = _maxIAPIdentSZ + IAPAbility::_maxIAPAbilityStrSZ;

  typedef smsc::core::buffers::FixedLengthString<_maxIAPIdentSZ>
    IdentStr_t;
  typedef smsc::core::buffers::FixedLengthString<_maxIAPPropertyStrSZ>
    TypeStr_t;

  enum IAPKind_e {
    iapUnknown = 0
    , iapDB             //obsolete
    , iapCHSRI
    , iapATSI
    , iapReserved       //just a max cap
  };

  IAPKind_e   _iapKind;
  IdentStr_t  _iapIdent;
  IAPAbility  _iapAbility;

  IAPProperty(IAPKind_e iap_kind, const char * iap_ident,
          uint8_t iap_ability /* = OR-ed Option_e values */)
    : _iapKind(iap_kind), _iapIdent(iap_ident), _iapAbility(iap_ability)
  { }
  ~IAPProperty()
  { }

  static IAPKind_e val2Kind(uint8_t ui_val)
  {
    return (ui_val >= iapReserved) ? iapUnknown : static_cast<IAPKind_e>(ui_val);
  }

  bool isEqual(IAPKind_e iap_kind) const { return _iapKind == iap_kind; }
  
  TypeStr_t toString(void) const;
};
typedef IAPProperty::IAPKind_e  IAPType_e;
typedef IAPProperty::TypeStr_t  IAPTypeStr_t;

//
extern IAPProperty  _knownIAPType[];

} //iaprvd
} //inman
} //smsc

#endif /* SMSC_INMAN_IAPROVIDER_DEFS_HPP */

