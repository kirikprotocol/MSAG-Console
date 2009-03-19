#include "GlobalTitle.hpp"
#include <stdio.h>
#include <string.h>
#include <util/Exception.hpp>

namespace eyeline {
namespace sua {
namespace communication {
namespace sua_messages {

GlobalTitle::GlobalTitle()
{
  memset(_gt_buffer.buf, 0, sizeof(_gt_buffer.buf));
  _gt_buffer.packedDataSz = GT_HEADER_SZ;
}

GlobalTitle::GlobalTitle(const uint8_t* buf, size_t bufSz)
{
  if ( bufSz > sizeof(_gt_buffer.buf) )
    throw smsc::util::Exception("GlobalTitle::GlobalTitle::: too long input buffer =[%d], max buffer size=[%d]", bufSz, sizeof(_gt_buffer.buf));
  memcpy(_gt_buffer.buf, buf, bufSz);
  _gt_buffer.packedDataSz = bufSz;
}

GlobalTitle::GlobalTitle(nature_addr_t natureofAddress, const std::string& gtDigits)
{
  _gt_buffer.buf[GTI_IDX] = NATURE_OF_ADDRESS_TAKENOVER;
  _gt_buffer.buf[TRANS_TYPE_IDX] = 0; // Unknown translation type
  _gt_buffer.buf[NUM_PLAN_IDX] = E164_NUMBERING_PLAN;
  _gt_buffer.buf[NATURE_OF_ADDR_IDX] = natureofAddress;
  _gt_buffer.packedDataSz = packGTDigits(gtDigits) + GT_HEADER_SZ;
}

GlobalTitle::GlobalTitle(uint8_t translation_type, const std::string& gtDigits)
{
  _gt_buffer.buf[GTI_IDX] = GT_INCLUDE_TRANSLATION_TYPE_ONLY;
  _gt_buffer.buf[TRANS_TYPE_IDX] = translation_type;
  _gt_buffer.buf[NUM_PLAN_IDX] = UNKNOWN_PLAN;
  _gt_buffer.buf[NATURE_OF_ADDR_IDX] = UNKNOWN_NATURE_OF_ADDRESS;
  _gt_buffer.packedDataSz = packGTDigits(gtDigits) + GT_HEADER_SZ;
}

GlobalTitle::GlobalTitle(uint8_t translation_type, numbering_plan_t numPlan, const std::string& gtDigits)
{
  _gt_buffer.buf[GTI_IDX] = NUMPLAN_AND_TRNSLTYPE_TAKENOVER;
  _gt_buffer.buf[TRANS_TYPE_IDX] = translation_type;
  _gt_buffer.buf[NUM_PLAN_IDX] = numPlan;
  _gt_buffer.buf[NATURE_OF_ADDR_IDX] = UNKNOWN_NATURE_OF_ADDRESS;
  _gt_buffer.packedDataSz = packGTDigits(gtDigits) + GT_HEADER_SZ;
}
  
GlobalTitle::GlobalTitle(uint8_t translation_type, numbering_plan_t numPlan, nature_addr_t addrNature, const std::string& gtDigits)
{
  _gt_buffer.buf[GTI_IDX] = GT_INCLUDE_FULL_INFO;
  _gt_buffer.buf[TRANS_TYPE_IDX] = translation_type;
  _gt_buffer.buf[NUM_PLAN_IDX] = numPlan;
  _gt_buffer.buf[NATURE_OF_ADDR_IDX] = addrNature;
  _gt_buffer.packedDataSz = packGTDigits(gtDigits) + GT_HEADER_SZ;
}

GlobalTitle::GlobalTitle(uint8_t natureOfAddress, const uint8_t* packedGtDigits, size_t packedGtDigitsSz)
{
  if ( packedGtDigitsSz +  GT_HEADER_SZ > sizeof(_gt_buffer.buf) )
    throw smsc::util::Exception("GlobalTitle::GlobalTitle::: too long input packed gt digits buffer =[%d], max buffer size=[%d]", packedGtDigitsSz, sizeof(_gt_buffer.buf) - GT_HEADER_SZ);

  _gt_buffer.buf[GTI_IDX] = NATURE_OF_ADDRESS_TAKENOVER;
  _gt_buffer.buf[TRANS_TYPE_IDX] = 0; // Unknown translation type
  _gt_buffer.buf[NUM_PLAN_IDX] = E164_NUMBERING_PLAN;
  _gt_buffer.buf[NATURE_OF_ADDR_IDX] = natureOfAddress;
  memcpy(&_gt_buffer.buf[NATURE_OF_ADDR_IDX+1], packedGtDigits, packedGtDigitsSz);
  _gt_buffer.packedDataSz = packedGtDigitsSz + GT_HEADER_SZ;
}

GlobalTitle::GlobalTitle(uint8_t translationType, uint8_t numPlan, uint8_t encScheme, const uint8_t* packedGtDigits, size_t packedGtDigitsSz)
{
  if ( packedGtDigitsSz +  GT_HEADER_SZ > sizeof(_gt_buffer.buf) )
    throw smsc::util::Exception("GlobalTitle::GlobalTitle::: too long input packed gt digits buffer =[%d], max buffer size=[%d]", packedGtDigitsSz, sizeof(_gt_buffer.buf) - GT_HEADER_SZ);

  if ( encScheme != 0x01 /*BCD, odd number of digits*/ &&
       encScheme != 0x02 /*BCD, even number of digits*/ )
    throw smsc::util::Exception("GlobalTitle::GlobalTitle::: unsupported encoding scheme=[%d]", encScheme);

  _gt_buffer.buf[GTI_IDX] = NUMPLAN_AND_TRNSLTYPE_TAKENOVER;
  _gt_buffer.buf[TRANS_TYPE_IDX] = translationType;
  _gt_buffer.buf[NUM_PLAN_IDX] = numPlan;
  _gt_buffer.buf[NATURE_OF_ADDR_IDX] = UNKNOWN_NATURE_OF_ADDRESS;
  memcpy(&_gt_buffer.buf[NATURE_OF_ADDR_IDX+1], packedGtDigits, packedGtDigitsSz);
  _gt_buffer.packedDataSz = packedGtDigitsSz + GT_HEADER_SZ;
}

GlobalTitle::GlobalTitle(uint8_t translationType, uint8_t numPlan, uint8_t encScheme, uint8_t natureOfAddress, const uint8_t* packedGtDigits, size_t packedGtDigitsSz)
{
  if ( packedGtDigitsSz +  GT_HEADER_SZ > sizeof(_gt_buffer.buf) )
    throw smsc::util::Exception("GlobalTitle::GlobalTitle::: too long input packed gt digits buffer =[%d], max buffer size=[%d]", packedGtDigitsSz, sizeof(_gt_buffer.buf) - GT_HEADER_SZ);

  if ( encScheme != ENCODING_BCD_ODD_NUMBER_DIGITS &&
       encScheme != ENCODING_BCD_EVEN_NUMBER_DIGITS )
    throw smsc::util::Exception("GlobalTitle::GlobalTitle::: unsupported encoding scheme=[%d]", encScheme);

  _gt_buffer.buf[GTI_IDX] = GT_INCLUDE_FULL_INFO;
  _gt_buffer.buf[TRANS_TYPE_IDX] = translationType;
  _gt_buffer.buf[NUM_PLAN_IDX] = numPlan;
  _gt_buffer.buf[NATURE_OF_ADDR_IDX] = natureOfAddress;
  memcpy(&_gt_buffer.buf[NATURE_OF_ADDR_IDX+1], packedGtDigits, packedGtDigitsSz);
  _gt_buffer.packedDataSz = packedGtDigitsSz + GT_HEADER_SZ;
  _gt_buffer.buf[DIGIT_NUM_IDX] = static_cast<uint8_t>(packedGtDigitsSz << 1);
  if ( encScheme == ENCODING_BCD_ODD_NUMBER_DIGITS ) --_gt_buffer.buf[DIGIT_NUM_IDX];
}

size_t
GlobalTitle::packGTDigits(const std::string& gtDigits)
{
  size_t digitsNum = gtDigits.size();
  if ( digitsNum > 255 )
    throw smsc::util::Exception("too long address string - max expected size is 255 symbols");

  _gt_buffer.buf[DIGIT_NUM_IDX] = static_cast<uint8_t>(digitsNum);

  uint8_t* nextOctetToPack = &_gt_buffer.buf[NATURE_OF_ADDR_IDX+1];
  size_t evenNumOfDigits = digitsNum & 0xFE;

  size_t totalOctets = ((digitsNum >> 1) + 4) & 0xFC;
  if ( totalOctets > sizeof(_gt_buffer.buf) )
    throw smsc::util::Exception("GlobalTitle::packGTDigits::: packed data too long =[%d], max buffer size=[%d]", totalOctets, sizeof(_gt_buffer.buf));

  size_t totalPackedOctets=0;
  for(int i=0; i<evenNumOfDigits; i += 2) {
    *nextOctetToPack++ = (gtDigits[i+1] - '0') << 4 | ( gtDigits[i] - '0' );
    ++totalPackedOctets;
  }
  if(evenNumOfDigits < digitsNum) {
    *nextOctetToPack++ = gtDigits[digitsNum-1] - '0';
    ++totalPackedOctets;
  }

  while ( totalPackedOctets++ & 0x03 ) {
    *nextOctetToPack++ = 0;
  }
  return totalPackedOctets - 1;
}

uint8_t 
GlobalTitle::getGTI() const
{
  return _gt_buffer.buf[GTI_IDX];
}

uint8_t
GlobalTitle::getTranslationType() const
{
  return _gt_buffer.buf[TRANS_TYPE_IDX];
}

uint8_t
GlobalTitle::getNumberingPlan() const
{
  return _gt_buffer.buf[NUM_PLAN_IDX];
}

uint8_t
GlobalTitle::getNatureOfAddress() const
{
  return _gt_buffer.buf[NATURE_OF_ADDR_IDX];
}

std::string
GlobalTitle::getGlobalTitleDigits() const
{
  uint8_t numOfDigits = _gt_buffer.buf[DIGIT_NUM_IDX];
  const uint8_t* value = &_gt_buffer.buf[GT_HEADER_SZ];

  uint8_t octetValue;
  std::string gtValue;
  if ( getNumberingPlan() == E164_NUMBERING_PLAN &&
       getTranslationType() == 1 )
    gtValue += '+';

  for (int i=0, j = (numOfDigits + 0x01) >> 1; i < j; ++i) {
    octetValue = *(value + i);
    gtValue += '0' + (octetValue & 0x0F);
    gtValue += '0' + (octetValue >> 4);
  }
  if ( numOfDigits & 0x01 ) // if not even
    gtValue.erase(gtValue.length()-1);

  return gtValue;
}

size_t
GlobalTitle::getNumOfGlobalTitleDigits() const
{
  return _gt_buffer.buf[DIGIT_NUM_IDX];
}

const uint8_t*
GlobalTitle::getValue() const
{
  return _gt_buffer.buf;
}

size_t
GlobalTitle::getValueSz() const
{
  return _gt_buffer.packedDataSz;
}

const uint8_t*
GlobalTitle::getPackedGTDigits() const
{
  return &_gt_buffer.buf[GT_HEADER_SZ];
}

size_t
GlobalTitle::getPackedGTDigitsSize() const
{
  return _gt_buffer.packedDataSz - GT_HEADER_SZ;
}

std::string
GlobalTitle::toString() const
{
  char tmpStr[128];
  snprintf(tmpStr, sizeof(tmpStr), "gti=%d,translationType=%d,numPlan=%d,natureOfAddr=%d,gtDigits=", getGTI(), getTranslationType(), getNumberingPlan(), getNatureOfAddress());
  return std::string(tmpStr) + getGlobalTitleDigits();
}

}}}}
