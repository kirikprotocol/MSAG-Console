/* ************************************************************************* *
 * Lightweight Bits Array: store bits on stack until its number doesn't
 * exceed specified limit.
 * ************************************************************************* */
#ifndef __BITARRAY_DEFS__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __BITARRAY_DEFS__

#include <memory.h>
#include "util/Exception.hpp"
#include "eyeline/util/IntConv.hpp"

namespace eyeline {
namespace util {

template <
  typename _SizeTypeArg //must be an unsigned integer type!
> 
_SizeTypeArg bitsNum2Octs(_SizeTypeArg num_bits)
{
  return num_bits/8 + (num_bits%8 ? 1 : 0);
}

/* ************************************************************************* *
 * Lightweight Extension of bit array: store bits into referenced array 
 * until its number doesn't exceed specified limit.
 * Afterward allocates new array on heap.
 * ************************************************************************* */
template <
  typename _SizeTypeArg         //must be an unsigned integer type!
>
class BITArrayExtension_T {
private:
  _SizeTypeArg  _orgBits;   //number of max bits in original array
  uint8_t *     _buf;       //pointer to data buffer (either heap or stack)
  _SizeTypeArg  _heapBufSz; //size of heap buffer allocated
  _SizeTypeArg  _numBits;   //number of initialized/assigned bits

protected:
  _SizeTypeArg _MAX_FACTOR(void) const { return _MAX_SIZE()/_orgBits; }

  void denyIndex(_SizeTypeArg bit_idx) const //throw(std::exception)
  {
    UIntToA_T<_SizeTypeArg> v0((_SizeTypeArg)sizeof(_SizeTypeArg));
    UIntToA_T<_SizeTypeArg> v1(_orgBits);
    UIntToA_T<_SizeTypeArg> v2(bit_idx);
    UIntToA_T<_SizeTypeArg> v3(_numBits ? _numBits - 1 : 0);
    throw smsc::util::Exception("BITArray_T<%s,%s>: index=%s is out of range=%c0:%s%c",
                                v0.get(), v1.get(), v2.get(),
                                _numBits ? '[' : '(', v3.get(), _numBits ? ']' : ')');
  }
  void denyAddition(_SizeTypeArg bits_2add) const //throw(std::exception)
  {
    UIntToA_T<_SizeTypeArg> v0((_SizeTypeArg)sizeof(_SizeTypeArg));
    UIntToA_T<_SizeTypeArg> v1(_orgBits);
    UIntToA_T<_SizeTypeArg> v2(bits_2add);
    UIntToA_T<_SizeTypeArg> v3(_MAX_SIZE() - size());

    throw smsc::util::Exception("BITArray_T<%s,%s>: number of elements are to"
                                " insert (%s) exceeds available limit (%s)",
                                v0.get(), v1.get(), v2.get(), v3.get());
  }

  //Returns number of unused bits in last assigned octet
  uint8_t unusedBits(void) const
  {
    return ((uint8_t)8 - (uint8_t)(_numBits%8));
  }
  //Clears unused bits in last assigned octet
  void clearUnused(void)
  {
    _buf[numOcts()-1] &= ((uint8_t)0xFF << unusedBits());
  }
  //Appends given octet to octet aligned bit array
  void appendOctetAligned(uint8_t use_val)
  {
    _buf[numOcts()] = use_val;
    _numBits += 8;
  }
  //Appends given number of octets to octet aligned bit array
  void appendOctetsAligned(const uint8_t * use_val, _SizeTypeArg num_octs)
  {
    if (num_octs < 8) {
      for (_SizeTypeArg i = 0; i < num_octs; ++i)
        appendOctetAligned(use_val[i]);
    } else {
      memcpy(_buf + numOcts(), use_val, num_octs);
      _numBits += num_octs<<3;
    }
  }
  //Appends given octet to bit array that isn't octet aligned
  void appendOctetUnaligned(uint8_t use_val)
  {
    uint8_t bitsUsed = (uint8_t)(_numBits%8);
    _buf[numOcts() - 1] &= ((uint8_t)0xFF << (8 - bitsUsed)); //clear unused bits
    _buf[numOcts() - 1] |= (use_val >> (8 - bitsUsed));

    _buf[numOcts()] = (use_val & ((uint8_t)0xFF >> bitsUsed)) << (8 - bitsUsed);
//    _buf[numOcts()] &= ((uint8_t)0xFF << (8 - bitsUsed)); //clear unused bits
    _numBits += 8;
  }
  //Appends MSBs of given octet to octet aligned bit array
  void appendBitsAligned(uint8_t use_val, uint8_t msb_used)
  {
    _buf[numOcts()] = (use_val &= ((uint8_t)0xFF << (8 - msb_used)));
    _numBits += msb_used;
  }
  //Appends MSBs of given octet to bit array that isn't octet aligned
  void appendBitsUnaligned(uint8_t use_val, uint8_t msb_used)
  {
    uint8_t bitsUsed = (uint8_t)(_numBits%8);
    uint8_t restUnused = (8 - bitsUsed);
    use_val &= ((uint8_t)0xFF << (8 - msb_used)); //clear unused bits

    _buf[numOcts() - 1] |= (use_val >> bitsUsed);
    if (msb_used <= restUnused) {
      _numBits += msb_used;
    } else { //split use_val 
      _numBits += restUnused;
      appendBitsAligned(use_val <<= restUnused, msb_used - restUnused);
    }
  }

  void reallocBuf(_SizeTypeArg req_octs)
  {
    _SizeTypeArg maxStackOcts = bitsNum2Octs(_orgBits);
    _SizeTypeArg factor = (req_octs + maxStackOcts - 1)/maxStackOcts;
    factor += (factor+3)>>2;
    _SizeTypeArg newOctSz = (factor > _MAX_FACTOR()) ? _MAX_SIZE()/8 : maxStackOcts * factor;

    uint8_t * hbuf = new uint8_t[newOctSz];
    _SizeTypeArg nb = numOcts();
    if (nb <= sizeof(void*)) {
      for (_SizeTypeArg i = 0; i < nb; ++i)
        hbuf[i] = _buf[i];
    } else
      memcpy(hbuf, _buf, nb);

    if (_heapBufSz)
      delete [] _buf;
    _buf = hbuf;
    _heapBufSz = newOctSz;
  }

  bool getBitValue(_SizeTypeArg bit_idx) const //throw(std::exception)
  {
    return _buf[bit_idx/8] & (1<<bit_idx%8);
  }

  void setBitValue(_SizeTypeArg bit_idx, bool use_val) //throw(std::exception)
  {
    if (use_val)
      _buf[bit_idx/8] |= (1 << (bit_idx%8));
    else
      _buf[bit_idx/8] &= ~(1 << (bit_idx%8));
  }
  //Initializes bits starting from last assigned and up to specified one
  //NOTE: must be called only for (bit_idx >= _numBits) && (bit_idx < capacity())
  void initBits(_SizeTypeArg bit_idx)
  {
    clearUnused();
    if (((bit_idx + 1) - _numBits) > unusedBits()) {
      _SizeTypeArg bit2set = (bit_idx + 1) - _numBits - unusedBits();
      _SizeTypeArg oct2set = bitsNum2Octs(bit2set) + ((bit2set%8) ? 1 : 0);
      if (oct2set <= sizeof(void*)) {
        for (_SizeTypeArg i = 0; i < oct2set; ++i)
          _buf[numOcts() + i] = 0;
      } else
        memset(_buf + numOcts(), 0, oct2set);
    }
  }

  //NOTE: it's a responsibility of a successor copying constructor
  //to properly copy referenced (original) buffer if necessary
  BITArrayExtension_T(const BITArrayExtension_T & use_arr) //throw()
    : _orgBits(use_arr._orgBits), _buf(0), _heapBufSz(0), _numBits(0)
  {
    if (use_arr._heapBufSz) {
      append(use_arr);        //NOTE: here append() cann't fail
    } else {                  //just a reference to original buffer
      _buf = use_arr._buf;
      _numBits = use_arr._numBits;
    }
  }

public:
  typedef _SizeTypeArg  size_type;

  _SizeTypeArg _MAX_SIZE(void) const { return (_SizeTypeArg)(-1); }

  BITArrayExtension_T() //throw()
    : _orgBits(0), _buf(0), _heapBufSz(0), _numBits(0)
  { }
  //Constructor for array, that extends given buffer
  BITArrayExtension_T(_SizeTypeArg org_max_bits, uint8_t * org_buf,
                      _SizeTypeArg org_num_bits = 0) //throw()
    : _orgBits(org_max_bits), _buf(org_buf), _heapBufSz(0), _numBits(org_num_bits)
  { }
  //
  ~BITArrayExtension_T()
  {
    assign(0, 0, 0);
  }

  bool empty(void) const { return _numBits == 0; }

  bool isHeapBuf(void) const { return _heapBufSz != 0; }

  //Assigns buffer to extend
  void assign(_SizeTypeArg org_max_bits, uint8_t * org_buf,
              _SizeTypeArg org_num_bits = 0) //throw()
  {
    if (_heapBufSz) {
      clear();
      delete [] _buf;
      _heapBufSz = 0;
    }
    _orgBits = org_max_bits; _buf = org_buf;
    _numBits = org_num_bits;
  }


  //Returns address of allocated array (single memory block)
  const uint8_t * getOcts(void) const { return _numBits ? _buf : 0; }

  //Returns number of octets occupied by used bits
  _SizeTypeArg numOcts(void) const { return bitsNum2Octs(_numBits); }

  //Returns number of used bits
  _SizeTypeArg size(void) const { return _numBits; }

  //Returns available capacity in bits of allocated array
  _SizeTypeArg capacity(void) const { return _heapBufSz ? _heapBufSz<<3 : _orgBits; }


  //Returns initialized/assigned element of array.
  //Throws if specified index is beyond of the space of initialized elemens.
  bool at(_SizeTypeArg bit_idx) const //throw(std::exception)
  {
    if (bit_idx >= _numBits)
      denyIndex(bit_idx);
    return getBitValue(bit_idx);
  }

  //Returns initialized/assigned element of array at specified index.
  //Throws if specified index exceeds array capacity. 
  //if specified index is beyond of the space of initialized elemens
  //but within capacity, all elements up to specified one are initialized.
  void setBit(_SizeTypeArg bit_idx, bool use_val = true) //throw(std::exception)
  {
    if (bit_idx >= capacity())
      denyIndex(bit_idx);
    initBits(bit_idx);
    setBitValue(bit_idx, use_val);
  }

  void clear(void)
  {
    _SizeTypeArg nb = numOcts();
    if (nb < sizeof(void*)) {
      for (_SizeTypeArg i = 0; i < nb; ++i)
        _buf[i] = 0;
    } else
      memset(_buf, 0, nb);
    _numBits = 0;
  }

  //Attempts to enlarge buffer for specified number of elements,
  //performing check for _SizeTypeArg overloading.
  //Returns false if resulting size exceeds _MAX_SIZE()
  bool enlarge(_SizeTypeArg add_bits) //throw()
  {
    _SizeTypeArg req_bits = size() + add_bits;
    if (req_bits < size()) //check for overloading;
      return false;

    if (req_bits > capacity())
      reallocBuf(bitsNum2Octs(req_bits));
    return true;
  }

  //Reserves space for storing given number of elements
  //Returns false if requested capacity exceeds _MAX_SIZE().
  bool reserve(_SizeTypeArg num_to_reserve)
  {
    if (num_to_reserve > capacity())
      reallocBuf(bitsNum2Octs(num_to_reserve));
    return true;
  }

  //Appends bit to array
  //Returns false if resulting number of bits exceeds available limit
  bool append(bool use_val) //throw()
  {
    bool rval = enlarge(1);
    if (rval)
      setBit(_numBits++, use_val);
    return rval;
  }

  //Appends MSBs of given octet
  //Returns false if resulting number of bits exceeds available limit
  bool append(uint8_t use_val, uint8_t msb_used) //throw()
  {
    bool rval = enlarge(msb_used);
    if (rval) {
      if (!_numBits%8) 
        appendBitsAligned(use_val, msb_used);
      else
        appendBitsUnaligned(use_val, msb_used);
      clearUnused();
    }
    return rval;
  }
  //Appends given number of bits from octet array
  //Returns false if resulting number of bits exceeds available limit
  bool append(const uint8_t * use_val, _SizeTypeArg bits_num) //throw()
  {
    bool rval = enlarge(bits_num);
    if (rval) {
      uint8_t bitsUnused = _numBits%8;
      _SizeTypeArg i = 0;
      if (bits_num > 8) {
        if (bitsUnused) {
          for (; i < bits_num/8; ++i)
            appendOctetUnaligned(use_val[i]);
        } else
          appendOctetsAligned(use_val, i = bits_num/8);
      }
      //append last bits
      if (!bitsUnused) 
        appendBitsAligned(use_val[i], (uint8_t)(bits_num%8));
      else
        appendBitsUnaligned(use_val[i], (uint8_t)(bits_num%8));
      clearUnused();
    }
    return rval;
  }

  //Appends another BITArrayExtension_T.
  //Returns false if resulting number of bits exceeds limit.
  bool append(const BITArrayExtension_T & use_arr) //throw()
  {
    return append(use_arr.getOcts(), use_arr.size());
  }

  //NOTE: throws if resulting number of elements would exceed limit 
  BITArrayExtension_T & operator+= (bool use_val) //throw(std::exception)
  {
    if (!append(use_val))
      denyAddition(1);
    return *this;
  }

  //NOTE: throws if resulting number of elements would exceed limit
  BITArrayExtension_T & operator+= (const BITArrayExtension_T & use_arr) //throw(std::exception)
  {
    if (!append(use_arr))
      denyAddition(use_arr.size());
    return *this;
  }

  BITArrayExtension_T & operator= (const BITArrayExtension_T & use_arr) //throw()
  {
    clear();
    append(use_arr); //cann't fail here
    return *this;
  }

  bool operator< (const BITArrayExtension_T & cmp_arr) const //throw()
  {
    return (size() == cmp_arr.size())
            ? (memcmp(getOcts(), cmp_arr.getOcts(), numOcts()) < 0)
            : (size() < cmp_arr.size());
  }

  bool operator== (const BITArrayExtension_T & cmp_arr) const //throw()
  {
    return (size() == cmp_arr.size())
            ? (memcmp(getOcts(), cmp_arr.getOcts(), numOcts()) == 0)
            : false;
  }

  bool operator[](_SizeTypeArg bit_idx) const //throw(std::exception)
  {
    return getBit(bit_idx);
  }

  class BIT {
  private:
    BITArrayExtension_T * _arr;
    _SizeTypeArg      _idx;

  protected:
    friend class BITArrayExtension_T;
    BIT(BITArrayExtension_T * use_arr, _SizeTypeArg use_idx)
      : _arr(use_arr), _idx(use_idx)
    { }

  public:
    BIT(const BIT & use_bit)
      : _arr(use_bit._arr), _idx(use_bit._idx)
    { }

    bool get(void) const { return _arr->getBit(_idx); } //throw(std::exception)

    BIT & operator=(bool use_val)
    {
      _arr->setBit(_idx, use_val);
      return *this;
    }

    bool operator==(bool use_val) const { return get() == use_val; }

    bool operator!=(bool use_val) const { return get() != use_val; }

    bool operator==(const BIT & use_bit) const
    {
      if (_arr == use_bit._arr)
        return _idx == use_bit._idx;
      return get() == use_bit.get();
    }
    bool operator!=(const BIT & use_bit) const
    {
      return !this->operator==(use_bit);
    }
  };

  BIT operator[](_SizeTypeArg bit_idx) //throw(std::exception)
  {
    if (bit_idx >= _numBits)
      setBit(bit_idx, false); //throws if capacity is exceeded!
    return BIT(this, bit_idx);
  }
};

/* ************************************************************************* *
 * Lightweight Bits Array: store bits on stack until its number doesn't
 * exceed specified limit.
 * ************************************************************************* */
template <
    typename _SizeTypeArg         //must be an unsigned integer type!
  , _SizeTypeArg _max_STACK_BITS  //maximum number of bits are to store on stack
>
class BITArray_T : public BITArrayExtension_T<_SizeTypeArg> {
private:
  union {
    uint8_t  buf[(_max_STACK_BITS+7)/8];
    void *   alignedPtr;
  } _stack;

public:
  typedef BITArrayExtension_T<_SizeTypeArg> base_type;
  typedef typename BITArrayExtension_T<_SizeTypeArg>::size_type size_type;

  explicit BITArray_T(_SizeTypeArg num_bits_to_reserve = 0) //throw()
    : BITArrayExtension_T<_SizeTypeArg>(_max_STACK_BITS, _stack.buf, 0)
  {
    _stack.alignedPtr = 0;
    if (num_bits_to_reserve)
      reserve(num_bits_to_reserve);
  }
  BITArray_T(const uint8_t * use_bits, _SizeTypeArg num_bits) //throw()
    : BITArrayExtension_T<_SizeTypeArg>(_max_STACK_BITS, _stack.buf, 0)
  {
    _stack.alignedPtr = 0;
    append(use_bits, num_bits); //NOTE: here append() cann't fail
  }
  BITArray_T(const BITArray_T & use_arr) //throw()
    : BITArrayExtension_T<_SizeTypeArg>(_max_STACK_BITS, _stack.buf, 0)
  {
    _stack.alignedPtr = 0;
    append(use_arr); //NOTE: here append() cann't fail
  }
  ~BITArray_T()
  {
    if (!this->isHeapBuf()) //elements on private stack were never moved, so destroy them
      this->clear();
  }

  template <_SizeTypeArg _SZArg>
  BITArray_T & operator= (const BITArray_T<_SizeTypeArg, _SZArg> & use_arr) //throw()
  {
    this->clear();
    append(use_arr.getOcts(), use_arr.size());
    return *this;
  }
};

} //util
} //eyeline

#endif /* __BITARRAY_DEFS__ */

