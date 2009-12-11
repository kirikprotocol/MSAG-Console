/* ************************************************************************** *
 * Various Address packing utilities for SCCP, TCAP, MAP, CAP.
 * ************************************************************************** */
#ifndef __SMSC_CONVERSION_ADDRESS_UTL_HPP__
#ident "@(#)$Id$"
#define __SMSC_CONVERSION_ADDRESS_UTL_HPP__

#include "util/TonNpiAddress.hpp"
using smsc::util::TonNpiAddress;
using smsc::util::CAPConst;

#include "inman/inap/SS7Types.hpp"

namespace smsc {
namespace cvtutil {

struct TONNPI_OCT { // MSB -> LSB
  uint8_t reserved_1  : 1;
  uint8_t ton         : 3;
  uint8_t npi         : 4;

  static uint8_t pack2Oct(uint8_t use_ton, uint8_t use_npi)
  {
    return use_npi | (use_ton << 4) | (uint8_t)(0x80);
  }

  static uint8_t pack2Oct(const TONNPI_OCT & use_st)
  {
    return use_st.npi | (use_st.ton << 4) | (use_st.reserved_1 << 7);
  }
  //
  static void unpackOct(uint8_t use_oct, TONNPI_OCT & use_st)
  {
    use_st.reserved_1 = use_oct >> 7;
    use_st.npi = use_oct & 0x0F;
    use_st.ton = (use_oct >> 4) & 0x07;
  }

  TONNPI_OCT() : reserved_1(1), ton(0), npi(0)
  { }

  TONNPI_OCT(uint8_t use_ton, uint8_t use_npi)
    : reserved_1(1), ton(use_ton), npi(use_npi)
  { }

  TONNPI_OCT(uint8_t use_oct)
  {
    unpackOct(use_oct);
  }

  uint8_t pack2Oct(void) const
  {
    return pack2Oct(*this);
  }
  //
  void unpackOct(uint8_t use_oct)
  {
    unpackOct(use_oct, *this);
  }
};

struct TONNPI_ADDRESS_OCTS {
  TONNPI_OCT  b0;
  uint8_t     val[CAPConst::MAX_SMS_AddressStringLength];  //MAX_ADDRESS_VALUE_LENGTH/2

  TONNPI_ADDRESS_OCTS()
  { val[0] = 0; }
};

struct LOCATION_ADDRESS_INDS {
  struct { /* type and length of number */
    uint8_t oddAdrLen : 1; /* odd/even number of chars in address */
    uint8_t ton       : 7; /* type of number */
  } b0;
  struct { /* octet of various indicators */
    uint8_t INNid     : 1;
    uint8_t npi       : 3;
    uint8_t presRestr : 2;
    uint8_t screenId  : 2;
  } b1;


  static unsigned pack2Octs(const LOCATION_ADDRESS_INDS & use_st, uint8_t * use_octs)
  {
    use_octs[0] = use_st.b0.ton | (use_st.b0.oddAdrLen << 7);
    use_octs[1] = use_st.b1.screenId | (use_st.b1.presRestr << 2)
                  | (use_st.b1.npi << 4) | (use_st.b1.INNid << 7);
    return 2;
  }
  //
  static unsigned unpackOcts(const uint8_t * use_octs, LOCATION_ADDRESS_INDS & use_st)
  { //b0
    use_st.b0.oddAdrLen = use_octs[0] >> 7;
    use_st.b0.ton = use_octs[0] & 0x7F;
    //b1
    use_st.b1.INNid = use_octs[1] >> 7;
    use_st.b1.npi = (use_octs[1] >> 4) & 0x07;
    use_st.b1.presRestr = (use_octs[1] >> 2) & 0x03;
    use_st.b1.presRestr = (use_octs[1]) & 0x03;
    return 2;
  }

  LOCATION_ADDRESS_INDS()
  {
    reset();
  }

  void reset(void)
  {
    b0.ton = b0.oddAdrLen = 0;
    b1.npi = b1.presRestr = b1.screenId = b1.INNid = 0;
  }

  unsigned pack2Octs(uint8_t * use_octs) const
  {
    return pack2Octs(*this, use_octs);
  }
  //
  unsigned unpackOcts(const uint8_t * use_octs)
  {
    return unpackOcts(use_octs, *this);
  }
};

struct LOCATION_ADDRESS_OCTS {
  LOCATION_ADDRESS_INDS   ind;
  uint8_t                 val[CAPConst::MAX_LocationNumberLength]; /* address */
};


//Packs TonNpiAddress to Address-String octets.
//NOTE: provided buffer must be at least sizeof(TONNPI_ADDRESS_OCTS) bytes long
extern unsigned packMAPAddress2OCTS(const TonNpiAddress & addr, uint8_t * use_octs);

extern unsigned unpackOCTS2MAPAddress(TonNpiAddress & addr, uint8_t * use_octs,
                                      unsigned octs_len);

//Packs TonNpiAddress to LocationAddress octets.
//Returns 0 in case of error, number of packed octets otherwise
//NOTE: alphanumeric numbers is forbidden! 
//NOTE: provided buffer must be at least sizeof(LOCATION_ADDRESS_OCTS) bytes long
extern unsigned packMAPAddress2LocationOCTS(const TonNpiAddress& addr, uint8_t * use_octs);

//according to Q.713 clause 3.4.2 (with GT & SSN)
extern unsigned packSCCPAddress(SCCP_ADDRESS_T* dst, const char *saddr, unsigned char ssn);
extern unsigned unpackSCCP2SSN_GT(const SCCP_ADDRESS_T* dst, unsigned char & ssn, char *addr);
}//namespace cvtutil
}//namespace smsc

#endif /* __SMSC_CONVERSION_ADDRESS_UTL_HPP__ */

