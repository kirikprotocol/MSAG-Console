/* ************************************************************************* *
 * Helper implementing bitmask of reference counters.
 * ************************************************************************* */
#ifndef __SMSC_UTIL_REFCOUNTS_MASK_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_UTIL_REFCOUNTS_MASK_HPP

#include <inttypes.h>
#include "core/buffers/FixedLengthString.hpp"

namespace smsc {
namespace util {

class RefCountersMaskAC {
private:
  const uint8_t   _maxRefs;
  const uint8_t   _bitsPerCnt;
  bool            _isToKill;
  uint8_t *       _refMask;
  uint8_t *       _refCnt; //ref counters, _bitsPerCnt bits per counter

  //Returns true if all _refMask bits have given value
  bool _isMaskBits(bool bit_value) const;
  //Returns true if all counter bits have given value
  bool _isCounterBits(uint8_t ref_idx, bool bit_value) const;
  //returns true if recursive counter isn't zero
  bool _hasCounter(uint8_t ref_idx) const
  {
    return !_isCounterBits(ref_idx, false);
  }
  //increases counter value,
  //returns false if number of recursive refs is exceeded the maximum
  bool _incCounter(uint8_t ref_idx);
  //decreases counter value,
  //NOTE: counter MUST BE checked fo zero prior to this call
  void _decCounter(uint8_t ref_idx);
  //returns number of references stored in counter
  uint16_t _getCounter(uint8_t ref_idx) const;

protected:
  static inline uint8_t _octMask(unsigned bit_idx)
  {
    return (uint8_t)(0x01 << (7 - (bit_idx % 8)));
  }
  static inline bool _bitValue(const uint8_t * p_arr, unsigned bit_idx)
  {
    return ((p_arr[bit_idx/8] & _octMask(bit_idx)) != 0);
  }
  static inline void _bitSet(uint8_t * p_arr, unsigned bit_idx)
  {
    p_arr[bit_idx/8] |= _octMask(bit_idx);
  }
  static inline void _bitUnset(uint8_t * p_arr, unsigned bit_idx)
  {
    p_arr[bit_idx/8] &= ~_octMask(bit_idx);
  }

  void setMask(uint8_t * use_ref_mask, uint8_t * use_cnt_arr)
  {
    _refMask = use_ref_mask; _refCnt = use_cnt_arr;
  }

  RefCountersMaskAC(uint8_t max_refs, uint8_t bits_per_cnt,
                    uint8_t * use_ref_mask, uint8_t * use_cnt_arr)
    : _maxRefs(max_refs), _bitsPerCnt(bits_per_cnt), _isToKill(false)
    , _refMask(use_ref_mask), _refCnt(use_cnt_arr)
  { }
  //NOTE: successor copying constructor MUST properly set arrays via setMask() call!
  RefCountersMaskAC(const RefCountersMaskAC & cp_obj)
    : _maxRefs(cp_obj._maxRefs), _bitsPerCnt(cp_obj._bitsPerCnt), _isToKill(cp_obj._isToKill)
    , _refMask(0), _refCnt(0)
  { }

public:
  ~RefCountersMaskAC()
  { }

  //returns true if at least one reference is present,
  //false - means referenced object may be safely deleted.
  bool hasRefs(void) const { return !_isMaskBits(false); }
  //returns true if reference with specified id is present
  bool hasRefs(uint8_t ref_idx) const { return _bitValue(_refMask, ref_idx); }
  //returns false if counter of reference with specified id
  //is exceeded the maximum, or referenced object is already
  //marked for deletion.
  bool addRef(uint8_t ref_idx);
  //returns true if no more reference with specified id is present.
  bool unRef(uint8_t ref_idx);
  //Returns counter of references with specified id.
  //NOTE: may return 0 if maximum number of refs is reached, so call
  //      hasRefs() to distinguish NULL and MAX counters.
  uint16_t getRefsCount(uint8_t ref_idx) const;
  //Converts refCounters mask to its string representation form.
  //Returns number of chars outputted.
  unsigned toString(char * use_buf, unsigned buf_len) const;
};


template <
    uint8_t _NumOfRefsArg       //restricts maximum number of references [1 .. 255]
  , uint8_t _BitsPerCntArg = 4  //bits per refs counter [1 .. 16],
>
class RefCountersMask_T : public RefCountersMaskAC {
public:
  enum Constants_e {
      refIdMaximum = (_NumOfRefsArg - 1)
    , refCntMaximum = (1 << _BitsPerCntArg) //maximum counter value is 1 + (2**_BitsPerCntArg - 1)
    , szoRefMask = ((_NumOfRefsArg + 7) >> 3)
    , szoCntArray =  (((_NumOfRefsArg*_BitsPerCntArg) + 7) >> 3)
    , charsPerCounter = (1 + (_BitsPerCntArg < 7) ? 2 : ((_BitsPerCntArg + 7) >> 3) * 3)
    , szoStringForm = (_NumOfRefsArg * charsPerCounter)
  };

  typedef smsc::core::buffers::FixedLengthString<szoStringForm + 1> StringForm_t;

  RefCountersMask_T()
    : RefCountersMaskAC(_NumOfRefsArg, _BitsPerCntArg, _refMaskArr, _refCntArr)
  {
    for (unsigned i = 0; i < szoRefMask; ++i)
      _refMaskArr[i] = 0;
    for (unsigned i = 0; i < szoCntArray; ++i)
      _refCntArr[i] = 0;
  }
  //
  RefCountersMask_T(const RefCountersMask_T & cp_obj)
    : RefCountersMaskAC(cp_obj)
  {
    setMask(_refMaskArr, _refCntArr);
    for (unsigned i = 0; i < szoRefMask; ++i)
      _refMaskArr[i] = cp_obj._refMaskArr[i];
    for (unsigned i = 0; i < szoCntArray; ++i)
      _refCntArr[i] = cp_obj._refCntArr[i];
  }
  //
  ~RefCountersMask_T()
  { }


  StringForm_t toString(void) const
  {
    StringForm_t rval;
    RefCountersMaskAC::toString(rval.str, (unsigned)rval.MAX_SZ - 1);
    return rval;
  }

private:
  uint8_t   _refMaskArr[szoRefMask];
  uint8_t   _refCntArr[szoCntArray]; //ref counters, _BitsPerCntArg per counter
};

} //util
} //smsc
#endif /* __SMSC_UTIL_REFCOUNTS_MASK_HPP */

