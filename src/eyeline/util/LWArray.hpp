/* ************************************************************************* *
 * Lightweight Array: store elements on stack until its number doesn't
 * exceed specified limit.
 * ************************************************************************* */
#ifndef __LWARRAY_DEFS__
#ident "@(#)$Id$"
#define __LWARRAY_DEFS__

#include "util/Exception.hpp"
#include "eyeline/util/IntConv.hpp"

namespace eyeline {
namespace util {

template <
    class _TArg                 //Element of array, must have a default constructor!
  , typename _SizeTypeArg       //must be an unsigned integer type, implicitly
                                //restricts maximum number of elemnets in array!
  , _SizeTypeArg _max_STACK_SZ  //maximum number of elements are to store on stack
>
class LWArray_T {
protected:
  _SizeTypeArg _heapBufSz;        //size of heap buffer allocated
  _SizeTypeArg _numElem;          //number of data elements
  _TArg   _sbuf[_max_STACK_SZ];   //
  _TArg * _buf;                   //pointer to data elements buffer (either heap or stck)

  _SizeTypeArg _MAX_FACTOR(void) const { return _MAX_SIZE()/_max_STACK_SZ; }

  //Resetes unused stack elements
  void resetUnusedStack(void)
  {
    if (!_heapBufSz) {
      for (_SizeTypeArg i = _numElem; i < capacity(); ++i)
        new (_buf + i)_TArg();
    }
  }

  void checkIndex(_SizeTypeArg use_idx) const //throw(std::exception)
  {
    if (use_idx >= _numElem) {
      UIntToA_T<_SizeTypeArg> v0((_SizeTypeArg)sizeof(_SizeTypeArg));
      UIntToA_T<_SizeTypeArg> v1(_max_STACK_SZ);
      UIntToA_T<_SizeTypeArg> v2(use_idx);
      UIntToA_T<_SizeTypeArg> v3(_numElem ? _numElem-1 : 0);
      throw smsc::util::Exception("LWArray_T<%s,%s>: index=%s is out of range=%s",
                                  v0.get(), v1.get(), v2.get(), v3.get());
    }
  }

  void denyAddition(_SizeTypeArg num_2add) const //throw(std::exception)
  {
    UIntToA_T<_SizeTypeArg> v0((_SizeTypeArg)sizeof(_SizeTypeArg));
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

  LWArray_T(_SizeTypeArg num_to_reserve = 0) //throw()
    : _heapBufSz(0), _numElem(0), _buf(_sbuf)
  {
    enlarge(num_to_reserve);
    resetUnusedStack();
  }
  LWArray_T(const LWArray_T & use_arr) //throw()
    : _heapBufSz(0), _numElem(0), _buf(_sbuf)
  {
    append(use_arr); //NOTE: here append() cann't fail
  }
  ~LWArray_T()
  {
    if (_heapBufSz)
      delete [] _buf;
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

  //Reserves space for storing given number of element
  //Returns false if requested capacity exceeds _MAX_SIZE().
  bool reserve(_SizeTypeArg num_to_reserve)
  {
    if (num_to_reserve <= capacity())
      return true;
    if (!enlarge(num_to_reserve - capacity()))
      return false;
    resetUnusedStack();
    return true;
  }

  //Attempts to enlarge buffer for specified number of elements,
  //performing check for _SizeTypeArg overloading.
  //Return false if resulting size exceeds _MAX_SIZE()
  bool enlarge(_SizeTypeArg add_sz) //throw()
  {
    _SizeTypeArg req_sz = capacity() + add_sz;
    if (req_sz < capacity()) //check for overloading;
      return false;

    if (req_sz > _max_STACK_SZ) {
      if (req_sz > _heapBufSz) { //reallocation is required
        _SizeTypeArg factor = (req_sz + _max_STACK_SZ - 1)/_max_STACK_SZ;
        factor += (factor+3)>>2;
        _SizeTypeArg new_sz = _max_STACK_SZ*((factor > _MAX_FACTOR()) ? _MAX_FACTOR() : factor);

        _TArg * hbuf = new _TArg[new_sz];
        //copy initialized elements
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

  //Appends element to array
  //Returns false if resulting number of elemennts would exceed limit
  bool append(const _TArg & use_val) //throw()
  {
    bool rval = enlarge(1);
    if (rval)
      _buf[_numElem++] = use_val;
    return rval;
  }
  //Appends another array.
  //Returns false if resulting number of elements would exceed limit
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

