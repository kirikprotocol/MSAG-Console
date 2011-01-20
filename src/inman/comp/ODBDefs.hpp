/* ************************************************************************* *
 * ODB (Operator Determined Barring) data definition.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_ODB_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_ODB_DEFS_HPP

namespace smsc {
namespace inman {
namespace comp {

class ODBGeneralData { //BIT STRING (SIZE (15..32))
public:
  static const uint16_t _max_BITS = 32;
  static const uint8_t _max_SZO = (_max_BITS + 7)/8;   //maximum length of BIT STRING in bytes

  typedef uint8_t value_type[_max_SZO];

protected:
  value_type  _value;

public:
  enum NamedBit_e {
    bit_allOG_CallsBarred = 0
    , bit_internationalOGCallsBarred  = 1
    , bit_internationalOGCallsNotToHPLMN_CountryBarred  = 2
    , bit_premiumRateInformationOGCallsBarred = 3
    /**/
    , bit_premiumRateEntertainementOGCallsBarred = 4
    , bit_ss_AccessBarred = 5
    , bit_interzonalOGCallsBarred = 6
    , bit_interzonalOGCallsNotToHPLMN_CountryBarred	= 7
    /**/
    , bit_interzonalOGCallsAndInternationalOGCallsNotToHPLMN_CountryBarred = 8
    , bit_allECT_Barred	= 9
    , bit_chargeableECT_Barred = 10
    , bit_internationalECT_Barred = 11
    /**/
    , bit_interzonalECT_Barred = 12
    , bit_doublyChargeableECT_Barred = 13
    , bit_multipleECT_Barred = 14
    , bit_allPacketOrientedServicesBarred = 15
    /**/
    , bit_roamerAccessToHPLMN_AP_Barred = 16
    , bit_roamerAccessToVPLMN_AP_Barred = 17
    , bit_roamingOutsidePLMNOG_CallsBarred = 18
    , bit_allIC_CallsBarred = 19
    /**/
    , bit_roamingOutsidePLMNIC_CallsBarred = 20
    , bit_roamingOutsidePLMNICountryIC_CallsBarred = 21
    , bit_roamingOutsidePLMN_Barred	= 22
    , bit_roamingOutsidePLMN_CountryBarred = 23
    /**/
    , bit_registrationAllCF_Barred = 24
    , bit_registrationCFNotToHPLMN_Barred = 25
    , bit_registrationInterzonalCF_Barred = 26
    , bit_registrationInterzonalCFNotToHPLMN_Barred	= 27
    /**/
    , bit_registrationInternationalCF_Barred = 28
    /* reserved bits */
    , bit_reserved_29 = 29
    , bit_reserved_30 = 30
    , bit_reserved_31 = 31
  };

  ODBGeneralData()
  {
    clear();
  }
  ODBGeneralData(uint8_t * use_oct, uint16_t num_bits)
  {
    init(use_oct, num_bits);
  }
  //
  ~ODBGeneralData()
  { }


  void clear(uint16_t start_bit = 0)
  {
    if (start_bit < _max_BITS) {
      uint16_t startOct = start_bit/8;

      _value[startOct] &= (uint8_t)(0xFF << (8 - (start_bit % 8)));
      for (uint16_t i = startOct + 1; i < _max_SZO; ++i)
        _value[i] = 0;
    }
  }

  void init(uint8_t * use_oct, uint16_t num_bits)
  {
    uint16_t numOcts = num_bits > _max_BITS ? _max_SZO : (num_bits + 7)/8;

    for (uint16_t i = 0; i < numOcts; ++i)
      _value[i] = use_oct[i];
    //clear unused bits
    clear(num_bits);
  }

  bool hasBit(NamedBit_e bit_name) const
  {
    return (_value[bit_name/8] & (1 << ( 7 - bit_name % 8))) != 0 ;
  }

  bool empty(void) const
  {
    for (uint8_t i = 0; i < _max_SZO; ++i) {
      if (_value[i])
        return false;
    }
    return true;
  }
};

}//namespace comp
}//namespace inman
}//namespace smsc

#endif /* __SMSC_INMAN_ODB_DEFS_HPP */
