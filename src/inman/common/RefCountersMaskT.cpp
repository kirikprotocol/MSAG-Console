#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <memory.h>
#include <stdio.h>
#include "inman/common/RefCountersMaskT.hpp"

namespace smsc  {
namespace util  {
/* ************************************************************************* *
 * class RefCountersMaskAC implementation.
 * ************************************************************************* */
//Returns true if all _refMask bits have given value
bool RefCountersMaskAC::_isMaskBits(bool bit_value) const
{
  for (uint8_t i = 0; i < _maxRefs; ++i) {
    if (_bitValue(_refMask, i) != bit_value)
      return false;
  }
  return true;
}

//Returns true if all _refMask bits, excluding specified one, have given value
bool RefCountersMaskAC::_isMaskBitsExcept(uint8_t ref_idx, bool bit_value) const
{
  for (uint8_t i = 0; i < _maxRefs; ++i) {
    if ((i != ref_idx) && (_bitValue(_refMask, i) != bit_value))
      return false;
  }
  return true;
}


//Returns true if all counter bits have given value
bool RefCountersMaskAC::_isCounterBits(uint8_t ref_idx, bool bit_value) const
{
  unsigned minBit = ref_idx*_bitsPerCnt;
  for (uint8_t i = 0; i < _bitsPerCnt; ++i, ++minBit) {
    if (_bitValue(_refCnt, minBit) != bit_value)
      return false;
  }
  return true;
}

//increases counter value,
//returns false if number of recursive refs is exceeded the maximum
bool RefCountersMaskAC::_incCounter(uint8_t ref_idx)
{
  if (_isCounterBits(ref_idx, true)) //all '1' - already max counter
    return false;

  //bool doShift = true;
  unsigned minBit = ref_idx*_bitsPerCnt;
  for (uint8_t i = 0; /*doShift && */(i < _bitsPerCnt); ++i, ++minBit) {
    if (!_bitValue(_refCnt, minBit)) {
      _bitSet(_refCnt, minBit);
      //doShift = false;
      break;
    } else
      _bitUnset(_refCnt, minBit);
  }
  return true;
}

//decreases counter value,
//NOTE: counter MUST BE checked fo zero prior to this call
void RefCountersMaskAC::_decCounter(uint8_t ref_idx)
{
  //bool doShift = true;
  unsigned minBit = ref_idx*_bitsPerCnt;
  for (uint8_t i = 0; /*doShift && */(i < _bitsPerCnt); ++i, ++minBit) {
    if (_bitValue(_refCnt, minBit)) {
      _bitUnset(_refCnt, minBit);
      //doShift = false;
      break;
    } else
      _bitSet(_refCnt, minBit);
  }
}

//returns number of references stored in counter
uint16_t RefCountersMaskAC::_getCounter(uint8_t ref_idx) const
{
  uint16_t rval = 0;
  unsigned minBit = ref_idx*_bitsPerCnt;
  for (uint8_t i = 0; (i < _bitsPerCnt); ++i, ++minBit) {
    if (_bitValue(_refCnt, minBit))
      rval += (1 << i);
  }
  return rval;
}

//returns false if counter of reference with specified id
//is exceeded the maximum, or referenced object is already
//marked for deletion.
bool RefCountersMaskAC::addRef(uint8_t ref_idx)
{
  if (_isToKill)
    return false;
  if (hasRefs(ref_idx))
    return _incCounter(ref_idx);

  _bitSet(_refMask, ref_idx);
  return true;
}

//returns true if no more reference with specified id is present.
bool RefCountersMaskAC::unRef(uint8_t ref_idx)
{
  bool nRef = _hasCounter(ref_idx);
  if (nRef)
    _decCounter(ref_idx);
  else
    _bitUnset(_refMask, ref_idx);

  _isToKill = _isMaskBits(false);
  return !nRef;
}

//Returns counter of references with specified id.
//NOTE: may return 0 if maximum number of refs is reached, so call
//      hasRefs() to distinguish NULL and MAX counters.
uint16_t RefCountersMaskAC::getRefsCount(uint8_t ref_idx) const
{
  return _bitValue(_refMask, ref_idx) ? _getCounter(ref_idx) + 1 : 0;
}

//Converts refCounters mask to its string representation form.
//Returns number of chars.
unsigned RefCountersMaskAC::toString(char * use_buf, unsigned buf_len) const
{
  if (!buf_len || (buf_len-- < 2))
    return 0;

  unsigned chIdx = 0;

  for (uint8_t i = 0; buf_len && (i < _maxRefs); ++i) {
    char      tmpBuf[12];
    unsigned  numCh = 0;

    if (_bitValue(_refMask, i)) {
      unsigned cnt = _getCounter(i) + 1;

      if (!cnt) { //counter overloading
        tmpBuf[numCh++] = 'M';
        tmpBuf[numCh++] = 'A';
        tmpBuf[numCh++] = 'X';
      } else {
        numCh = snprintf(tmpBuf, sizeof(tmpBuf)-1, "%u", cnt);
      }
    } else {
      tmpBuf[numCh++] = '0';
    }
    tmpBuf[numCh++] = ',';
    if (numCh > buf_len) {
      numCh = buf_len;
      buf_len = 0;
    } else
      buf_len -= numCh;

    memcpy(use_buf + chIdx, tmpBuf, numCh);
    chIdx += numCh;
  }
  if (chIdx && (use_buf[chIdx] == ','))
    --chIdx;
  use_buf[chIdx] = 0;
  return chIdx;
}

} //util
} //smsc

