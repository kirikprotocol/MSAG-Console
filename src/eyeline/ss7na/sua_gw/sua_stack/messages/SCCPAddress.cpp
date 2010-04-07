#include <util/Exception.hpp>
#include "SCCPAddress.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

void
SCCPAddress::setGtIndicatorInAddressIndicator(uint8_t gti)
{
  _addressIndicator |= gti << 2;
}

void
SCCPAddress::transformGT2ITUFormat(const TLV_GlobalTitle& gt)
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
SCCPAddress::parseGtForGTI_0001(const GlobalTitle& gt_value)
{
  uint8_t natureOfAddress = gt_value.getNatureOfAddress();
  if ( gt_value.getNumOfGlobalTitleDigits() & 0x01 )
    natureOfAddress |= 0x80; // set 'odd number of address signals' indication in first octet of gt
  _address.Append(&natureOfAddress, sizeof(natureOfAddress));
  _address.Append(gt_value.getPackedGTDigits(), gt_value.getPackedGTDigitsSize());
}

void
SCCPAddress::parseGtForGTI_0010(const GlobalTitle& gt_value)
{
  uint8_t translationType = gt_value.getTranslationType();
  _address.Append(&translationType, sizeof(translationType));
  _address.Append(gt_value.getPackedGTDigits(), gt_value.getPackedGTDigitsSize());
}

void
SCCPAddress::parseGtForGTI_0011(const GlobalTitle& gt_value)
{
  uint8_t translationType = gt_value.getTranslationType();
  _address.Append(&translationType, sizeof(translationType));
  uint8_t secondOctet = gt_value.getNumberingPlan() << 4;
  if ( gt_value.getNumOfGlobalTitleDigits() & 0x01 )
    secondOctet |= 0x01; // BCD, odd number of digits
  else
    secondOctet |= 0x02; // BCD, even number of digits
  _address.Append(gt_value.getPackedGTDigits(), gt_value.getPackedGTDigitsSize());
}

void
SCCPAddress::parseGtForGTI_0100(const GlobalTitle& gt_value)
{
  uint8_t translationType = gt_value.getTranslationType();
  _address.Append(&translationType, sizeof(translationType));
  uint8_t secondOctet = gt_value.getNumberingPlan() << 4;
  if ( gt_value.getNumOfGlobalTitleDigits() & 0x01 )
    secondOctet |= 0x01; // BCD, odd number of digits
  else
    secondOctet |= 0x02; // BCD, even number of digits
  _address.Append(&secondOctet, sizeof(secondOctet));
  uint8_t natureOfAddress = gt_value.getNatureOfAddress();
  _address.Append(&natureOfAddress, sizeof(natureOfAddress));
  _address.Append(gt_value.getPackedGTDigits(), gt_value.getPackedGTDigitsSize());
}

SCCPAddress::SCCPAddress(const TLV_GlobalTitle& gt)
  : _addressIndicator(0), _address(MAX_ESTIMATED_ADDRESS_SZ)
{
  setGtIndicatorInAddressIndicator(gt.getGlobalTitleValue().getGTI());
  _address.Append(&_addressIndicator, sizeof(_addressIndicator));

  uint8_t ssnValue=0;
  _address.Append(&ssnValue, sizeof(ssnValue));

  transformGT2ITUFormat(gt);
}

SCCPAddress::SCCPAddress(const TLV_PointCode& pc,
                         const TLV_SSN& ssn)
  : _addressIndicator(ROUTE_ON_SSN_INDICATOR | SSN_INDICATOR | POINT_CODE_INDICATOR),
    _address(MAX_ESTIMATED_ADDRESS_SZ)
{
  _address.Append(&_addressIndicator, sizeof(_addressIndicator));

  common::ITU_PC itu_pc = pc.get_ITU_PC();
  uint8_t pcValue[ITU_PC_SZ];
  pcValue[0] = uint8_t(itu_pc.getValue() >> 8);
  pcValue[1] = uint8_t(itu_pc.getValue() & 0xff);
  _address.Append(pcValue, ITU_PC_SZ);

  uint8_t ssnValue = static_cast<uint8_t>(ssn.getValue());
  _address.Append(&ssnValue, sizeof(ssnValue));
}

SCCPAddress::SCCPAddress(const TLV_GlobalTitle& gt,
                         const TLV_SSN& ssn)
  : _addressIndicator(SSN_INDICATOR), _address(MAX_ESTIMATED_ADDRESS_SZ)
{
  setGtIndicatorInAddressIndicator(gt.getGlobalTitleValue().getGTI());
  _address.Append(&_addressIndicator, sizeof(_addressIndicator));

  uint8_t ssnValue = static_cast<uint8_t>(ssn.getValue());
  _address.Append(&ssnValue, sizeof(ssnValue));

  transformGT2ITUFormat(gt);
}

SCCPAddress::SCCPAddress(const uint8_t* address_value, uint8_t address_size)
  : _address(MAX_ESTIMATED_ADDRESS_SZ)
{
  if ( !address_size )
    throw smsc::util::Exception("SCCPAddress::SCCPAddress::: invalid addressSize value=[0]");

  _address.Append(address_value, address_size);
}

size_t
SCCPAddress::serialize(uint8_t* dst_buf, size_t dst_buf_sz) const
{
  if ( dst_buf_sz < _address.GetPos() ) throw smsc::util::Exception("SCCPAddress::serialize::: destination buffer too small");
  memcpy(dst_buf, _address.get(), _address.GetPos());
  return _address.GetPos();
}

GlobalTitle
SCCPAddress::makeGlobalTitle(uint8_t gti, const uint8_t* gt_value)
{
  if (gti == GT_INCLUDES_NATURE_OF_ADRESS_INDICATOR) {
    return GlobalTitle(*gt_value, &gt_value[1], _address.GetCurPtr() - &gt_value[1]);
  } else if (gti == GT_INCLUDES_TRANSLATION_TYPE_ONLY_INDICATOR) {
    throw smsc::util::Exception("SCCPAddress::makeGlobalTitle::: Global title indicator = 0010 isn't supported");
  } else if (gti == GT_INCLUDES_TRANSTYPE_NUMPLAN_ENCODSCHEME) {
    return GlobalTitle(gt_value[0]  /*trans type*/,
                       gt_value[1] >> 4 /*num plan*/,
                       gt_value[1] & 0x0F /*enc scheme*/,
                       &gt_value[2], _address.GetCurPtr() - &gt_value[2]);
  } else if (gti == GT_INCLUDES_TRANSTYPE_NUMPLAN_ENCODSCHEME_AND_NATUREOFADDR_INDICATOR) {
    return GlobalTitle(gt_value[0]  /*trans type*/,
                       gt_value[1] >> 4 /*num plan*/,
                       gt_value[1] & 0x0F /*enc scheme*/,
                       gt_value[2] /*nature of addess*/,
                       &gt_value[3], _address.GetCurPtr() - &gt_value[3]);
  } else
    throw smsc::util::Exception("SCCPAddress::makeGlobalTitle::: invalid gti value=[%d]", gti);
}

}}}}}
