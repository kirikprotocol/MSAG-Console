#include <util/Exception.hpp>
#include "SCCPAddress.hpp"

namespace sua_messages {

void
SCCPAddress::setGtIndicatorInAddressIndicator(uint8_t gti)
{
  _addressIndicator |= gti << 2;
}

void
SCCPAddress::transformGT2ITUFormat(const sua_messages::TLV_GlobalTitle& gt)
{
  const GlobalTitle& gtValue = gt.getGlobalTitleValue();
  uint8_t gti = gtValue.getGTI();
  if ( gti == GlobalTitle::NATURE_OF_ADDRESS_TAKENOVER )
    parseGtForGTI_0001(gtValue);
  else if ( gti == GlobalTitle::GT_INCLUDE_TRANSLATION_TYPE_ONLY )
    parseGtForGTI_0010(gtValue);
  else if ( gti == GlobalTitle::NUMPLAN_AND_TRNSLTYPE_TAKENOVER )
    parseGtForGTI_0011(gtValue);
  else if ( gti == GlobalTitle::GT_INCLUDE_FULL_INFO )
    parseGtForGTI_0100(gtValue);
}

void
SCCPAddress::parseGtForGTI_0001(const GlobalTitle& gtValue)
{
  uint8_t natureOfAddress = gtValue.getNatureOfAddress();
  if ( gtValue.getNumOfGlobalTitleDigits() & 0x01 )
    natureOfAddress |= 0x80; // set 'odd number of address signals' indication in first octet of gt
  _address.Append(&natureOfAddress, sizeof(natureOfAddress));
  _address.Append(gtValue.getPackedGTDigits(), gtValue.getPackedGTDigitsSize());
}

void
SCCPAddress::parseGtForGTI_0010(const GlobalTitle& gtValue)
{
  uint8_t translationType = gtValue.getTranslationType();
  _address.Append(&translationType, sizeof(translationType));
  _address.Append(gtValue.getPackedGTDigits(), gtValue.getPackedGTDigitsSize());
}

void
SCCPAddress::parseGtForGTI_0011(const GlobalTitle& gtValue)
{
  uint8_t translationType = gtValue.getTranslationType();
  _address.Append(&translationType, sizeof(translationType));
  uint8_t secondOctet = gtValue.getNumberingPlan() << 4;
  if ( gtValue.getNumOfGlobalTitleDigits() & 0x01 )
    secondOctet |= 0x01; // BCD, odd number of digits
  else
    secondOctet |= 0x02; // BCD, even number of digits
  _address.Append(gtValue.getPackedGTDigits(), gtValue.getPackedGTDigitsSize());
}

void
SCCPAddress::parseGtForGTI_0100(const GlobalTitle& gtValue)
{
  uint8_t translationType = gtValue.getTranslationType();
  _address.Append(&translationType, sizeof(translationType));
  uint8_t secondOctet = gtValue.getNumberingPlan() << 4;
  if ( gtValue.getNumOfGlobalTitleDigits() & 0x01 )
    secondOctet |= 0x01; // BCD, odd number of digits
  else
    secondOctet |= 0x02; // BCD, even number of digits
  _address.Append(&secondOctet, sizeof(secondOctet));
  uint8_t natureOfAddress = gtValue.getNatureOfAddress();
  _address.Append(&natureOfAddress, sizeof(natureOfAddress));
  _address.Append(gtValue.getPackedGTDigits(), gtValue.getPackedGTDigitsSize());
}

SCCPAddress::SCCPAddress(const sua_messages::TLV_GlobalTitle& gt)
  : _addressIndicator(0), _address(MAX_ESTIMATED_ADDRESS_SZ)
{
  setGtIndicatorInAddressIndicator(gt.getGlobalTitleValue().getGTI());
  _address.Append(&_addressIndicator, sizeof(_addressIndicator));

  uint8_t ssnValue=0;
  _address.Append(&ssnValue, sizeof(ssnValue));

  transformGT2ITUFormat(gt);
}

SCCPAddress::SCCPAddress(const sua_messages::TLV_PointCode& pc, const sua_messages::TLV_SSN& ssn)
  : _addressIndicator(ROUTE_ON_SSN_INDICATOR | SSN_INDICATOR | POINT_CODE_INDICATOR),
    _address(MAX_ESTIMATED_ADDRESS_SZ)
{
  _address.Append(&_addressIndicator, sizeof(_addressIndicator));

  sua_messages::ITU_PC itu_pc = pc.get_ITU_PC();
  const uint8_t* pcValue = itu_pc.getValue();
  _address.Append(pcValue+ITU_PC_OFFSET_IN_SUA_PC, ITU_PC_SZ);

  uint8_t ssnValue = static_cast<uint8_t>(ssn.getValue());
  _address.Append(&ssnValue, sizeof(ssnValue));
}

SCCPAddress::SCCPAddress(const sua_messages::TLV_GlobalTitle& gt, const sua_messages::TLV_SSN& ssn)
  : _addressIndicator(SSN_INDICATOR), _address(MAX_ESTIMATED_ADDRESS_SZ)
{
  setGtIndicatorInAddressIndicator(gt.getGlobalTitleValue().getGTI());
  _address.Append(&_addressIndicator, sizeof(_addressIndicator));

  uint8_t ssnValue = static_cast<uint8_t>(ssn.getValue());
  _address.Append(&ssnValue, sizeof(ssnValue));

  transformGT2ITUFormat(gt);
}

SCCPAddress::SCCPAddress(const uint8_t* addressValue, uint8_t addressSize)
  : _address(MAX_ESTIMATED_ADDRESS_SZ)
{
  if ( !addressSize )
    throw smsc::util::Exception("SCCPAddress::SCCPAddress::: invalid addressSize value=[0]");

  _address.Append(addressValue, addressSize);
}

size_t
SCCPAddress::serialize(uint8_t* dstBuf, size_t dstBufSz) const
{
  if ( dstBufSz < _address.GetPos() ) throw smsc::util::Exception();
  memcpy(dstBuf, _address.get(), _address.GetPos());
  return _address.GetPos();
}

GlobalTitle
SCCPAddress::makeGlobalTitle(uint8_t gti, const uint8_t* gtValue)
{
  if (gti == GT_INCLUDES_NATURE_OF_ADRESS_INDICATOR) {
    return GlobalTitle(*gtValue, &gtValue[1], _address.GetCurPtr() - &gtValue[1]);
  } else if (gti == GT_INCLUDES_TRANSLATION_TYPE_ONLY_INDICATOR) {
    throw smsc::util::Exception("SCCPAddress::makeGlobalTitle::: Global title indicator = 0010 isn't supported");
  } else if (gti == GT_INCLUDES_TRANSTYPE_NUMPLAN_ENCODSCHEME) {
    return GlobalTitle(gtValue[0]  /*trans type*/,
                       gtValue[1] >> 4 /*num plan*/,
                       gtValue[1] & 0x0F /*enc scheme*/,
                       &gtValue[2], _address.GetCurPtr() - &gtValue[2]);
  } else if (gti == GT_INCLUDES_TRANSTYPE_NUMPLAN_ENCODSCHEME_AND_NATUREOFADDR_INDICATOR) {
    return GlobalTitle(gtValue[0]  /*trans type*/,
                       gtValue[1] >> 4 /*num plan*/,
                       gtValue[1] & 0x0F /*enc scheme*/,
                       gtValue[2] /*nature of addess*/,
                       &gtValue[3], _address.GetCurPtr() - &gtValue[3]);
  } else
    throw smsc::util::Exception("SCCPAddress::makeGlobalTitle::: invalid gti value=[%d]", gti);
}

// TLV_Address
// SCCPAddress::make_TLV_Address(uint16_t tag) const
// {
//   const uint8_t* addressValue = _address.get();
//   uint8_t addressIndicator = addressValue[0];

//   if ( addressIndicator & 0x01 &&
//        addressIndicator & 0x02 ) {
//     uint8_t pcBuf[4] = {0};
//     memcpy(pcBuf+2, &addressValue[1], 2);

//     return TLV_Address(tag, TLV_PointCode(ITU_PC(pcBuf)), TLV_SSN(addressValue[3]));
//   } else {
//     uint8_t gti = (addressIndicator >> 2) & 0x0F;

//     if ( gti != NO_GT_INCLUDED_INDICATOR ) { // if global title indicator value is not 0
//       unsigned int whereSSNStart = 1;
//       if ( addressIndicator & 0x01 )
//         whereSSNStart = 3;

//       uint8_t* gtValue = &addressValue[whereSSNStart+1];
//       if ( addressIndicator & 0x02 )
//         return TLV_Address(tag, TLV_GlobalTitle(makeGlobalTitle(gti, gtValue)),
//                            TLV_SSN(addressValue[whereSSNStart]));
//       else
//         return TLV_Address(tag, TLV_GlobalTitle(makeGlobalTitle(gti, gtValue)));
//     } else
//       throw smsc::util::Exception("SCCPAddress::make_TLV_Address::: invalid addressIndicator value=[%02X]", addressIndicator);
//   }
// }

}
