#ifndef __EYELINE_SS7NA_SUAGW_SUASTACK_MESSAGES_SCCPADDRESS_HPP__
# define __EYELINE_SS7NA_SUAGW_SUASTACK_MESSAGES_SCCPADDRESS_HPP__

# include <sys/types.h>
# include "eyeline/ss7na/sua_gw/sua_stack/messages/SuaTLV.hpp"
# include "core/buffers/TmpBuf.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

class SCCPAddress {
public:
  explicit SCCPAddress(const TLV_GlobalTitle& gt);
  SCCPAddress(const TLV_PointCode& pc, const TLV_SSN& ssn);
  SCCPAddress(const TLV_GlobalTitle& gt, const TLV_SSN& ssn);
  SCCPAddress(const uint8_t* addressValue, uint8_t addressSize);

  size_t serialize(uint8_t* dstBuf, size_t dstBufSz) const;

  template<class TLV_ADDRESS>
  TLV_ADDRESS make_TLV_Address();
private:
  static const uint8_t ROUTE_ON_SSN_INDICATOR = 0x40;
  static const uint8_t SSN_INDICATOR = 0x02;
  static const uint8_t POINT_CODE_INDICATOR = 0x01;

  static const uint8_t NO_GT_INCLUDED_INDICATOR = 0x0;
  static const uint8_t GT_INCLUDES_NATURE_OF_ADRESS_INDICATOR = 0x01;
  static const uint8_t GT_INCLUDES_TRANSLATION_TYPE_ONLY_INDICATOR = 0x02;
  static const uint8_t GT_INCLUDES_TRANSTYPE_NUMPLAN_ENCODSCHEME = 0x03;
  static const uint8_t GT_INCLUDES_TRANSTYPE_NUMPLAN_ENCODSCHEME_AND_NATUREOFADDR_INDICATOR = 0x04;

  uint8_t _addressIndicator;
  enum { MAX_ESTIMATED_ADDRESS_SZ = 32, ITU_PC_SZ = 2, ITU_PC_OFFSET_IN_SUA_PC = 2 };
  smsc::core::buffers::TmpBuf<uint8_t,MAX_ESTIMATED_ADDRESS_SZ> _address;

  void setGtIndicatorInAddressIndicator(uint8_t gti);
  void transformGT2ITUFormat(const TLV_GlobalTitle& gt);
  void parseGtForGTI_0001(const GlobalTitle& gtValue);
  void parseGtForGTI_0010(const GlobalTitle& gtValue);
  void parseGtForGTI_0011(const GlobalTitle& gtValue);
  void parseGtForGTI_0100(const GlobalTitle& gtValue);

  GlobalTitle makeGlobalTitle(uint8_t gti, const uint8_t* gtValue);
};

template<class TLV_ADDRESS>
TLV_ADDRESS
SCCPAddress::make_TLV_Address()
{
  const uint8_t* addressValue = _address.get();
  uint8_t addressIndicator = addressValue[0];

  if ( addressIndicator & 0x01 &&
       addressIndicator & 0x02 ) {
    uint8_t pcBuf[4] = {0};
    memcpy(pcBuf+2, &addressValue[1], 2);

    return TLV_ADDRESS(TLV_PointCode(common::ITU_PC(uint32_t(pcBuf[2])<<8 | uint32_t(pcBuf[3]))), TLV_SSN(addressValue[3]));
  } else {
    uint8_t gti = (addressIndicator >> 2) & 0x0F;

    if ( gti != NO_GT_INCLUDED_INDICATOR ) { // if global title indicator value is not 0
      unsigned int whereSSNStart = 1;
      if ( addressIndicator & 0x01 )
        whereSSNStart = 3;

      const uint8_t* gtValue = &addressValue[whereSSNStart+1];
      if ( addressIndicator & 0x02 )
        return TLV_ADDRESS(TLV_GlobalTitle(makeGlobalTitle(gti, gtValue)),
                           TLV_SSN(addressValue[whereSSNStart]));
      else
        return TLV_ADDRESS(TLV_GlobalTitle(makeGlobalTitle(gti, gtValue)));
    } else
      throw smsc::util::Exception("SCCPAddress::make_TLV_Address::: invalid addressIndicator value=[%02X]", addressIndicator);
  }
}

}}}}}

#endif
