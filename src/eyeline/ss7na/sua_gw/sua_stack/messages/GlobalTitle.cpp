#include <stdio.h>
#include <string.h>

#include "util/Exception.hpp"
#include "GlobalTitle.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

GlobalTitle::GlobalTitle()
{
  memset(_gt_buffer.buf, 0, sizeof(_gt_buffer.buf));
  _gt_buffer.packedDataSz = GT_HEADER_SZ;
}

GlobalTitle::GlobalTitle(const uint8_t* buf, size_t buf_sz)
{
  if ( buf_sz > sizeof(_gt_buffer.buf) )
    throw smsc::util::Exception("GlobalTitle::GlobalTitle::: too long input buffer =[%d], max buffer size=[%d]", buf_sz, sizeof(_gt_buffer.buf));
  memcpy(_gt_buffer.buf, buf, buf_sz);
  _gt_buffer.packedDataSz = buf_sz;
}

GlobalTitle::GlobalTitle(nature_addr_t nature_of_address, const std::string& gt_digits)
{
  _gt_buffer.buf[GTI_IDX] = NATURE_OF_ADDRESS_TAKENOVER;
  _gt_buffer.buf[TRANS_TYPE_IDX] = 0; // Unknown translation type
  _gt_buffer.buf[NUM_PLAN_IDX] = E164_NUMBERING_PLAN;
  _gt_buffer.buf[NATURE_OF_ADDR_IDX] = nature_of_address;
  _gt_buffer.packedDataSz = packGTDigits(gt_digits) + GT_HEADER_SZ;
}

GlobalTitle::GlobalTitle(uint8_t translation_type, const std::string& gt_digits)
{
  _gt_buffer.buf[GTI_IDX] = GT_INCLUDE_TRANSLATION_TYPE_ONLY;
  _gt_buffer.buf[TRANS_TYPE_IDX] = translation_type;
  _gt_buffer.buf[NUM_PLAN_IDX] = UNKNOWN_PLAN;
  _gt_buffer.buf[NATURE_OF_ADDR_IDX] = UNKNOWN_NATURE_OF_ADDRESS;
  _gt_buffer.packedDataSz = packGTDigits(gt_digits) + GT_HEADER_SZ;
}

GlobalTitle::GlobalTitle(uint8_t translation_type,
                         numbering_plan_t num_plan,
                         const std::string& gt_digits)
{
  _gt_buffer.buf[GTI_IDX] = NUMPLAN_AND_TRNSLTYPE_TAKENOVER;
  _gt_buffer.buf[TRANS_TYPE_IDX] = translation_type;
  _gt_buffer.buf[NUM_PLAN_IDX] = num_plan;
  _gt_buffer.buf[NATURE_OF_ADDR_IDX] = UNKNOWN_NATURE_OF_ADDRESS;
  _gt_buffer.packedDataSz = packGTDigits(gt_digits) + GT_HEADER_SZ;
}
  
GlobalTitle::GlobalTitle(uint8_t translation_type,
                         numbering_plan_t num_plan, nature_addr_t addr_nature,
                         const std::string& gt_digits)
{
  _gt_buffer.buf[GTI_IDX] = GT_INCLUDE_FULL_INFO;
  _gt_buffer.buf[TRANS_TYPE_IDX] = translation_type;
  _gt_buffer.buf[NUM_PLAN_IDX] = num_plan;
  _gt_buffer.buf[NATURE_OF_ADDR_IDX] = addr_nature;
  _gt_buffer.packedDataSz = packGTDigits(gt_digits) + GT_HEADER_SZ;
}

GlobalTitle::GlobalTitle(uint8_t nature_of_address,
                         const uint8_t* packed_gt_digits,
                         size_t packed_gt_digits_sz)
{
  if ( packed_gt_digits_sz +  GT_HEADER_SZ > sizeof(_gt_buffer.buf) )
    throw smsc::util::Exception("GlobalTitle::GlobalTitle::: too long input packed gt digits buffer =[%d], max buffer size=[%d]", packed_gt_digits_sz, sizeof(_gt_buffer.buf) - GT_HEADER_SZ);

  _gt_buffer.buf[GTI_IDX] = NATURE_OF_ADDRESS_TAKENOVER;
  _gt_buffer.buf[TRANS_TYPE_IDX] = 0; // Unknown translation type
  _gt_buffer.buf[NUM_PLAN_IDX] = E164_NUMBERING_PLAN;
  _gt_buffer.buf[NATURE_OF_ADDR_IDX] = nature_of_address;
  memcpy(&_gt_buffer.buf[NATURE_OF_ADDR_IDX+1], packed_gt_digits, packed_gt_digits_sz);
  _gt_buffer.packedDataSz = packed_gt_digits_sz + GT_HEADER_SZ;
}

GlobalTitle::GlobalTitle(uint8_t translation_type, uint8_t num_plan,
                         uint8_t enc_scheme, const uint8_t* packed_gt_digits,
                         size_t packed_gt_digits_sz)
{
  if ( packed_gt_digits_sz +  GT_HEADER_SZ > sizeof(_gt_buffer.buf) )
    throw smsc::util::Exception("GlobalTitle::GlobalTitle::: too long input packed gt digits buffer =[%d], max buffer size=[%d]", packed_gt_digits_sz, sizeof(_gt_buffer.buf) - GT_HEADER_SZ);

  if ( enc_scheme != 0x01 /*BCD, odd number of digits*/ &&
       enc_scheme != 0x02 /*BCD, even number of digits*/ )
    throw smsc::util::Exception("GlobalTitle::GlobalTitle::: unsupported encoding scheme=[%d]", enc_scheme);

  _gt_buffer.buf[GTI_IDX] = NUMPLAN_AND_TRNSLTYPE_TAKENOVER;
  _gt_buffer.buf[TRANS_TYPE_IDX] = translation_type;
  _gt_buffer.buf[NUM_PLAN_IDX] = num_plan;
  _gt_buffer.buf[NATURE_OF_ADDR_IDX] = UNKNOWN_NATURE_OF_ADDRESS;
  memcpy(&_gt_buffer.buf[NATURE_OF_ADDR_IDX+1], packed_gt_digits, packed_gt_digits_sz);
  _gt_buffer.packedDataSz = packed_gt_digits_sz + GT_HEADER_SZ;
}

GlobalTitle::GlobalTitle(uint8_t translation_type, uint8_t num_plan,
                         uint8_t enc_scheme, uint8_t nature_of_address,
                         const uint8_t* packed_gt_digits, size_t packed_gt_digits_sz)
{
  if ( packed_gt_digits_sz +  GT_HEADER_SZ > sizeof(_gt_buffer.buf) )
    throw smsc::util::Exception("GlobalTitle::GlobalTitle::: too long input packed gt digits buffer =[%d], max buffer size=[%d]", packed_gt_digits_sz, sizeof(_gt_buffer.buf) - GT_HEADER_SZ);

  if ( enc_scheme != ENCODING_BCD_ODD_NUMBER_DIGITS &&
       enc_scheme != ENCODING_BCD_EVEN_NUMBER_DIGITS )
    throw smsc::util::Exception("GlobalTitle::GlobalTitle::: unsupported encoding scheme=[%d]", enc_scheme);

  _gt_buffer.buf[GTI_IDX] = GT_INCLUDE_FULL_INFO;
  _gt_buffer.buf[TRANS_TYPE_IDX] = translation_type;
  _gt_buffer.buf[NUM_PLAN_IDX] = num_plan;
  _gt_buffer.buf[NATURE_OF_ADDR_IDX] = nature_of_address;
  memcpy(&_gt_buffer.buf[NATURE_OF_ADDR_IDX+1], packed_gt_digits, packed_gt_digits_sz);
  _gt_buffer.packedDataSz = packed_gt_digits_sz + GT_HEADER_SZ;
  _gt_buffer.buf[DIGIT_NUM_IDX] = static_cast<uint8_t>(packed_gt_digits_sz << 1);
  if ( enc_scheme == ENCODING_BCD_ODD_NUMBER_DIGITS ) --_gt_buffer.buf[DIGIT_NUM_IDX];
}

size_t
GlobalTitle::packGTDigits(const std::string& gt_digits)
{
  size_t digitsNum = gt_digits.size();
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
    *nextOctetToPack++ = (gt_digits[i+1] - '0') << 4 | ( gt_digits[i] - '0' );
    ++totalPackedOctets;
  }
  if(evenNumOfDigits < digitsNum) {
    *nextOctetToPack++ = gt_digits[digitsNum-1] - '0';
    ++totalPackedOctets;
  }

  while ( totalPackedOctets++ & 0x03 )
    *nextOctetToPack++ = 0;

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

}}}}}
