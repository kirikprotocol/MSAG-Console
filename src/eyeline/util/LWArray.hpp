/* ************************************************************************* *
 * Lightweight Array: store elements on stack until its number doesn't
 * exceed specified limit.
 * ************************************************************************* */
#ifndef __LWARRAY_DEFS__
#ident "@(#)$Id$"
#define __LWARRAY_DEFS__

#include "util/Exception.hpp"

namespace eyeline {
namespace util {

template <
  typename _SizeTypeArg /* must be an unsigned integer type! */ 
> 
class UIntToA_T { //converts unsigned integer type to its decimal string representation
protected:
  uint8_t _idx;
  char _buf[sizeof(_SizeTypeArg)*3+1];

public:
  UIntToA_T(_SizeTypeArg use_val)
    : _idx(sizeof(_SizeTypeArg)*3)
  {
    _buf[_idx] = 0;
    do {
      _buf[--_idx] = '0' + (char)(use_val % 10);
    } while ((use_val /= 10) != 0);
  }
  ~UIntToA_T()
  { }

  const char * get(void) const { return _buf + _idx; }
};


template <
    class _TArg                 //Element of array, must have a default constructor!
  , typename _SizeTypeArg       //must be an unsigned integer type!
  , _SizeTypeArg _max_STACK_SZ  //maximum number of elements are to store on stack
>
class LWArray_T {
protected:
  _SizeTypeArg _heapBufSz;        //size of heap buffer allocated
  _SizeTypeArg _numElem;          //number of data elements
  _TArg   _sbuf[_max_STACK_SZ];   //
  _TArg * _buf;                   //pointer to data elements buffer (either heap or stck)

  _SizeTypeArg _MAX_FACTOR(void) const { return _MAX_SIZE()/_max_STACK_SZ; }

  void checkIndex(_SizeTypeArg use_idx) const //throw(std::exception)
  {
    if (use_idx >= _numElem) {
      UIntToA_T<_SizeTypeArg> v0(sizeof(_SizeTypeArg));
      UIntToA_T<_SizeTypeArg> v1(_max_STACK_SZ);
      UIntToA_T<_SizeTypeArg> v2(use_idx);
      UIntToA_T<_SizeTypeArg> v3(_numElem ? _numElem-1 : 0);
      throw smsc::util::Exception("LWArray_T<%s,%s>: index=%s is out of range=%s",
                                  v0.get(), v1.get(), v2.get(), v3.get());
    }
  }

  void denyAddition(_SizeTypeArg num_2add) const //throw(std::exception)
  {
    UIntToA_T<_SizeTypeArg> v0(sizeof(_SizeTypeArg));
    UIntToA_T<_SizeTypeArg> v1(_max_STACK_SZ);
    UIntToA_T<_SizeTypeArg> v2(num_2add);
    UIntToA_T<_SizeTypeArg> v3(_MAX_SIZE() - _numElem);

    throw smsc::util::Exception("LWArray_T<%s,%s>: number of elements are to"
                                " insert (%s) exceeds available limit (%s)",
                                v0.get(), v1.get(), v2.get(), v3.get());
  }

public:
  typedef _SizeTypeArg  size_type;

  _SizeTypeArg _MAX_SIZE(void) const { return (_SizeTypeArg)(-1); }

  LWArray_T(_SizeTypeArg use_sz = 0) //throw()
    : _heapBufSz(0), _numElem(0), _buf(_sbuf)
  {
    enlarge(use_sz);
    if (!_heapBufSz) { //reset stack elements
      for (_SizeTypeArg i = 0; i < _numElem; ++i)
        new (_buf + i)_TArg();
    }
  }
  LWArray_T(const LWArray_T & use_arr) //throw()
    : _heapBufSz(0), _numElem(0), _buf(_sbuf)
  {
    append(use_arr); //NOTE: here append() cann't fail
  }

  //Returns number of array elements
  _SizeTypeArg size(void) const { return _numElem; }

  //Returns size of allocated array
  _SizeTypeArg capacity(void) const { return _heapBufSz ? _heapBufSz : _max_STACK_SZ; }


  //Returns address of elements array (single memory block)
  const _TArg * get(void) const { return _numElem ? _buf : 0; }

  _TArg & at(_SizeTypeArg use_idx) //throw(std::exception)
  {
    checkIndex(use_idx);
    return _buf[use_idx];
  }

  const _TArg & at(_SizeTypeArg use_idx) const //throw(std::exception)
  {
    checkIndex(use_idx);
    return ((const _TArg *)_buf)[use_idx];
  }

  _TArg & operator[](_SizeTypeArg use_idx) //throw(std::exception)
  {
    return at(use_idx);
  }

  const _TArg & operator[](_SizeTypeArg use_idx) const //throw(std::exception)
  {
    return at(use_idx);
  }

  void clear(void)
  {
    for (_SizeTypeArg i = 0; i < _numElem; ++i)
      _buf[i].~_TArg();
    _numElem = 0;
  }

  //Attempts to enlarge buffer for specified number of elements,
  //performin check for _SizeTypeArg overloading.
  //Return false if resulting size exceeds _MAX_SIZE()
  bool enlarge(_SizeTypeArg add_sz) //throw()
  {
    _SizeTypeArg req_sz = _numElem + add_sz;
    if (req_sz < _numElem) //check for overloading;
      return false;

    if (req_sz > _max_STACK_SZ) {
      if (req_sz > _heapBufSz) { //reallocation is required
        _SizeTypeArg factor = (req_sz + _max_STACK_SZ - 1)/_max_STACK_SZ;
        factor += (factor+3)>>2;
        _SizeTypeArg new_sz = _max_STACK_SZ*((factor > _MAX_FACTOR()) ? _MAX_FACTOR() : factor);

        _TArg * hbuf = new _TArg[new_sz];
        for (_SizeTypeArg i = 0; i < _numElem; ++i)
          hbuf[i] = _buf[i];

        if (_heapBufSz)
          delete [] _buf;
        _buf = hbuf;
        _heapBufSz = new_sz;
      } //else heap buffer has enough free space
    }
    return true;
  }

  //Appends tag to tagging.
  //Returns false if resulting number of tags exceeds limit of 255.
  bool append(const _TArg & use_val) //throw()
  {
    bool rval = enlarge(1);
    if (rval)
      _buf[_numElem++] = use_val;
    return rval;
  }
  //Appends another tagging.
  //Returns false if resulting number of tags exceeds limit of 255.
  bool append(const LWArray_T & use_arr) //throw()
  {
    bool rval = enlarge(use_arr.size());
    if (rval) { //copy elements
      for (_SizeTypeArg i = 0; i < use_arr.size(); ++i)
        _buf[_numElem - 1 + i] = use_arr._buf[i];
      _numElem += use_arr.size();
    }
    return rval;
  }

  //NOTE: throws if resulting number of elements would exceed limit 
  LWArray_T & operator+= (const _TArg & use_val) //throw(std::exception)
  {
    if (!append(use_val))
      denyAddition(1);
    return *this;
  }

  //NOTE: throws if resulting number of elements would exceed limit
  LWArray_T & operator+= (const LWArray_T & use_arr) //throw(std::exception)
  {
    if (!append(use_arr))
      denyAddition(use_arr.size());
    return *this;
  }

  LWArray_T & operator= (const LWArray_T & use_arr) //throw()
  {
    clear();
    append(use_arr); //cann't fail here
    return *this;
  }
};

} //util
} //eyeline

#endif /* __LWARRAY_DEFS__ */

