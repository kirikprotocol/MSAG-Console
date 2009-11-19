/* ************************************************************************* *
 * Lightweight Bits Array: store elements on stack until its number doesn't
 * exceed specified limit.
 * ************************************************************************* */
#ifndef __BITARRAY_DEFS__
#ident "@(#)$Id$"
#define __BITARRAY_DEFS__

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

template <
    typename _SizeTypeArg         //must be an unsigned integer type!
  , _SizeTypeArg _max_STACK_BITS  //maximum number of bits are to store on stack
>
class BITArray_T {
protected:
  _SizeTypeArg  _heapBufSz;        //size of heap buffer allocated
  _SizeTypeArg  _numBits;          //number of used bits
  uint8_t       _sbuf[(_max_STACK_BITS+7)/8];   //
  uint8_t *     _buf;              //pointer to data buffer (either heap or stack)

  _SizeTypeArg _MAX_FACTOR(void) const { return _MAX_BITS()/_max_STACK_BITS; }

  void checkIndex(_SizeTypeArg bit_idx) const //throw(std::exception)
  {
    max_index = _numBits ? _numBits - 1 : 0;
    if (bit_idx > max_index) {
      UIntToA_T<_SizeTypeArg> v0((_SizeTypeArg)sizeof(_SizeTypeArg));
      UIntToA_T<_SizeTypeArg> v1(_max_STACK_BITS);
      UIntToA_T<_SizeTypeArg> v2(bit_idx);
      UIntToA_T<_SizeTypeArg> v3(max_index);
      throw smsc::util::Exception("BITArray_T<%s,%s>: index=%s is out of range=%c0:%s%c",
                                  v0.get(), v1.get(), v2.get(),
                                  _numBits ? '[' : '(', v3.get(), _numBits ? ']' : ')');
    }
  }

  void denyAddition(_SizeTypeArg bits_2add) const //throw(std::exception)
  {
    UIntToA_T<_SizeTypeArg> v0((_SizeTypeArg)sizeof(_SizeTypeArg));
    UIntToA_T<_SizeTypeArg> v1(_max_STACK_BITS);
    UIntToA_T<_SizeTypeArg> v2(bits_2add);
    UIntToA_T<_SizeTypeArg> v3(_MAX_BITS() - size());

    throw smsc::util::Exception("BITArray_T<%s,%s>: number of elements are to"
                                " insert (%s) exceeds available limit (%s)",
                                v0.get(), v1.get(), v2.get(), v3.get());
  }

  void clearUnused(void)
  {
    _buf[numOcts()-1] &= ((uint8_t)0xFF << (8 - (uint8_t)(_numBits%8))); //clear unused bits
  }
  //Appends given octet to octet aligned bit array
  void appendOctetAligned(uint8_t use_val)
  {
    _buf[numOcts()] = use_val;
    _numBits += 8;
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
      appendBitsAligned(use_val <<= restUnused, msb_used - restUnused)
    }
  }


public:
  class BIT {
  private:
    uint8_t * _addr;
    uint8_t   _offs : 3;

  protected:
    friend BITArray_T;
    BIT(uint8_t * use_adr, uint8_t use_offs)
      : _addr(use_addr), _offs(use_offs)
    { }

  public:
    BIT(const BIT & use_bit)
      : _addr(use_bit._addr), _offs(use_bit._offs)
    { }

    bool isSet(void) const { return ((*_addr) & (1 << _offs)) != 0; }

    BIT & operator=(bool use_val)
    {
      if (use_val)
        *_addr |= (1 << _offs);
      else
        *_addr &= ~(1 << _offs);
      return *this;
    }
    BIT & operator=(uint8_t use_val)
    {
      return operator=(use_val != 0);
    }

    bool operator==(const BIT & use_bit) const
    {
      return value() == use_bit.value();
    }
    bool operator!=(const BIT & use_bit) const
    {
      return value() != use_bit.value();
    }
  };

  typedef _SizeTypeArg  size_type;

  _SizeTypeArg _MAX_BITS(void) const { return (_SizeTypeArg)(-1); }


  BITArray_T(_SizeTypeArg num_bits = 0) //throw()
    : _heapBufSz(0), _numBits(0), _buf(_sbuf)
  {
    enlarge(num_bits);
    if (!_heapBufSz) { //reset stack elements
      for (_SizeTypeArg i = 0; i < numOcts(); ++i)
        _buf[i] = 0; //TODO: use memset() ?
    }
  }
  BITArray_T(const BITArray_T & use_arr) //throw()
    : _heapBufSz(0), _numBits(0), _buf(_sbuf)
  {
    append(use_arr); //NOTE: here append() cann't fail
  }
  ~BITArray_T()
  {
    if (_heapBufSz)
      delete [] _buf;
  }

  //Returns address of allocated array (single memory block)
  const uint8_t * getOcts(void) const { return _numOcts ? _buf : 0; }

  //Returns number of octets occupied by used bits
  _SizeTypeArg numOcts(void) const { return bitsNum2Octs(_numBits); }


  //Returns number of used bits
  _SizeTypeArg size(void) const { return _numBits; }

  //Returns available capacity in bits of allocated array
  _SizeTypeArg capacity(void) const { return _heapBufSz ? _heapBufSz<<3 : _max_STACK_BITS; }

  bool getBit(_SizeTypeArg bit_idx) const //throw(std::exception)
  {
    checkIndex(bit_idx);
    return _buf[bit_idx/8] & (1<<bit_idx%8);
  }

  void setBit(_SizeTypeArg bit_idx, bool use_val) //throw(std::exception)
  {
    checkIndex(bit_idx);
    if (use_val)
      _buf[bit_idx/8] |= (1 << (bit_idx%8));
    else
      _buf[bit_idx/8] &= ~(1 << (bit_idx%8));
  }

  BIT & operator[](_SizeTypeArg bit_idx) //throw(std::exception)
  {
    return BIT(&_buf[bit_idx/8], bit_idx%8);
  }

  const BIT & operator[](_SizeTypeArg bit_idx) const //throw(std::exception)
  {
    return BIT(&_buf[bit_idx/8], bit_idx%8);
  }

  void clear(void)
  {
    for (_SizeTypeArg i = 0; i < numOcts(); ++i)
      _buf[i] = 0; //TODO: use memset() ?
    _numBits = 0;
  }

  //Attempts to enlarge buffer for specified number of elements,
  //performin check for _SizeTypeArg overloading.
  //Return false if resulting size exceeds _MAX_BITS()
  bool enlarge(_SizeTypeArg add_bits) //throw()
  {
    _SizeTypeArg req_bits = size() + add_bits;
    if (req_bits < size()) //check for overloading;
      return false;

    if (req_bits > _max_STACK_BITS) {
      reqOcts = bitsNum2Octs(req_bits);
      maxStackOcts = bitsNum2Octs(_max_STACK_BITS);
      if (reqOcts > _heapBufSz) { //reallocation is required
        _SizeTypeArg factor = (reqOcts + maxStackOcts - 1)/maxStackOcts;
        factor += (factor+3)>>2;
        _SizeTypeArg newSz = maxStackOcts*((factor > _MAX_FACTOR()) ? _MAX_FACTOR() : factor);

        uint8_t * hbuf = new uint8_t[newSz];
        for (_SizeTypeArg i = 0; i < numOcts(); ++i)
          hbuf[i] = _buf[i]; //TODO: use memcpy() ?

        if (_heapBufSz)
          delete [] _buf;
        _buf = hbuf;
        _heapBufSz = newSz;
      } //else heap buffer has enough free space
    }
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
      _SizeTypeArg i = 0
      if (use_arr.size() > 8) {
        if (!bitsUnused) {
          for (; i < bits_num/8; ++i)
            appendOctetAligned(use_val[i]);
        } else {
          for (; i < bits_num/8; ++i)
            appendOctetUnaligned(use_val[i]);
        }
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

  //Appends another BITArray_T.
  //Returns false if resulting number of tags exceeds limit of 255.
  bool append(const BITArray_T & use_arr) //throw()
  {
    return append(use_arr.getOcts(), use_arr.size());
  }

  //NOTE: throws if resulting number of elements would exceed limit 
  BITArray_T & operator+= (bool use_val) //throw(std::exception)
  {
    if (!append(use_val))
      denyAddition(1);
    return *this;
  }

  //NOTE: throws if resulting number of elements would exceed limit
  BITArray_T & operator+= (const BITArray_T & use_arr) //throw(std::exception)
  {
    if (!append(use_arr))
      denyAddition(use_arr.size());
    return *this;
  }

  BITArray_T & operator= (const BITArray_T & use_arr) //throw()
  {
    clear();
    append(use_arr); //cann't fail here
    return *this;
  }
};

} //util
} //eyeline

#endif /* __BITARRAY_DEFS__ */

