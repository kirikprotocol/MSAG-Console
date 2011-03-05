/* ************************************************************************* *
 * Lightweight Array: store elements on stack until its number doesn't
 * exceed specified limit.
 * NOTE: array reallocation occurs only in case of explicit demand!
 * ************************************************************************* */
#ifndef __UTIL_LWARRAY_DEFS__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __UTIL_LWARRAY_DEFS__

#include <algorithm>

#include "util/Exception.hpp"
#include "eyeline/util/IntConv.hpp"
#include "eyeline/util/MaxSizeof.hpp"
#include "eyeline/util/LWArrayTraits.hpp"

namespace eyeline {
namespace util {

template <
  typename _SizeTypeArg //must be an unsigned integer type, implicitly
                        //restricts maximum number of elements in array!
>
struct LWArrayResizerDflt {
  //always calculate a new size as a multiple of original size
  static _SizeTypeArg resize(_SizeTypeArg req_sz, _SizeTypeArg org_sz)
  {
    _SizeTypeArg maxSize = (_SizeTypeArg)(-1);
    _SizeTypeArg factor = req_sz/org_sz + (req_sz % org_sz  ? 1 : 0);
    return (factor > maxSize/org_sz) ? maxSize : org_sz*factor;
  }
};

/* ************************************************************************* *
 * Lightweight Extension of array: store elements into referenced array 
 * until its number doesn't exceed specified limit. Afterward allocates 
 * new array on heap. 
 * NOTE: Element type must have operator<() defined in order to maintain 
 *       sorted array.
 * ************************************************************************* */
template <
    class _TArg           //Element of array, must have default & copying
                          //constructors (operator<() in case of sorted array)
  , typename _SizeTypeArg //must be an unsigned integer type, implicitly
                          //restricts maximum number of elements in array!
  , class _ResizerArg = LWArrayResizerDflt<_SizeTypeArg>
>
class LWArrayExtension_T {
private:
  _SizeTypeArg  _orgSz;         //number of max elements in original array
  _TArg *       _buf;           //pointer to data elements buffer (either extension heap or original buf)
  _SizeTypeArg  _heapBufSz;     //size of heap buffer allocated
  _SizeTypeArg  _numElem;       //number of initilized/assigned elements

protected:
  void denyIndex(_SizeTypeArg use_idx) const //throw(std::exception)
  {
    UIntToA_T<_SizeTypeArg> v0((_SizeTypeArg)sizeof(_SizeTypeArg));
    UIntToA_T<_SizeTypeArg> v1(_orgSz);
    UIntToA_T<_SizeTypeArg> v2(use_idx);
    UIntToA_T<_SizeTypeArg> v3(_numElem ? _numElem-1 : 0);
    throw smsc::util::Exception("LWArray_T<%s,%s>: index=%s is out of range=%s",
                                v0.get(), v1.get(), v2.get(), v3.get());
  }

  void denyAddition(_SizeTypeArg num_2add) const //throw(std::exception)
  {
    UIntToA_T<_SizeTypeArg> v0((_SizeTypeArg)sizeof(_SizeTypeArg));
    UIntToA_T<_SizeTypeArg> v1(_orgSz);
    UIntToA_T<_SizeTypeArg> v2(num_2add);
    UIntToA_T<_SizeTypeArg> v3(_MAX_SIZE() - _numElem);

    throw smsc::util::Exception("LWArray_T<%s,%s>: number of elements are to"
                                " insert (%s) exceeds available limit (%s)",
                                v0.get(), v1.get(), v2.get(), v3.get());
  }

  //Reallocates heap buffer.
  void reallocBuf(_SizeTypeArg req_sz) //throw()
  {
    _SizeTypeArg new_sz =_ResizerArg::resize(req_sz, _orgSz);

    _TArg * hbuf = (_TArg *)(new uint8_t[sizeof(_TArg) * new_sz]);
    //copy initialized elements
    LWArrayTraits<_TArg>::construct(hbuf, _buf, _numElem);
    LWArrayTraits<_TArg>::destroy(_buf, _numElem);
    if (_heapBufSz)
      delete [] (uint8_t*)_buf;

    _buf = hbuf;
    _heapBufSz = new_sz;
  }

  //shifts array right, keeping moved elements intact
  //NOTE: at_pos must be <= _numElem
  bool shiftRightOnly(const _SizeTypeArg shift_sz, _SizeTypeArg at_pos = 0) //throw()
  {
    if (!enlarge(shift_sz))
      return false;

    LWArrayTraits<_TArg>::shift_right(_buf + at_pos, (_SizeTypeArg)(_numElem - at_pos), shift_sz);
    _numElem -= at_pos;
    _numElem += shift_sz;
    return true;
  }

  //NOTE: array should not be empty!
  _SizeTypeArg lower_bound_pos(const _TArg & use_val) const //throw()
  {
    const _TArg * pNext = std::lower_bound(get(), get() + size(), use_val);
    return (pNext - get());
  }

  //NOTE: it's a responsibility of a successor copying constructor
  //to properly copy referenced (original) buffer if necessary
  LWArrayExtension_T(const LWArrayExtension_T & use_arr) //throw()
    : _orgSz(use_arr._orgSz), _buf(0), _heapBufSz(0), _numElem(0)
  {
    if (use_arr._heapBufSz) {
      append(use_arr);        //NOTE: here append() cann't fail
    } else {                  //just a reference to original buffer
      _buf = use_arr._buf;
      _numElem = use_arr._numElem;
    }
  }

public:
  typedef _TArg         value_type;
  typedef _SizeTypeArg  size_type;

  _SizeTypeArg _MAX_SIZE(void) const { return (_SizeTypeArg)(-1); }

  //Special Not-a-Position marker
  _SizeTypeArg npos(void) const { return (_SizeTypeArg)(-1); }

  explicit LWArrayExtension_T() //throw()
    : _orgSz(0), _buf(0), _heapBufSz(0), _numElem(0)
  { }

  //Constructor for array, that extends given buffer
  LWArrayExtension_T(_SizeTypeArg org_max_sz, _TArg * org_buf,
                     _SizeTypeArg org_num_elem = 0) //throw()
    : _orgSz(org_max_sz), _buf(org_buf), _heapBufSz(0), _numElem(org_num_elem)
  { }
  //
  ~LWArrayExtension_T()
  {
    assign(0, 0, 0);
  }

  bool empty(void) const { return _numElem == 0; }

  bool isHeapBuf(void) const { return _heapBufSz != 0; }

  //Assigns buffer to extend
  void assign(_SizeTypeArg org_max_sz, _TArg * org_buf,
            _SizeTypeArg org_num_elem = 0) //throw()
  {
    if (_heapBufSz) {
      clear();
      delete [] (uint8_t*)_buf;
      _heapBufSz = 0;
    }
    _orgSz = org_max_sz; _buf = org_buf;
    _numElem = org_num_elem;
  }

  //Returns number of initilized/assigned array elements
  _SizeTypeArg size(void) const { return _numElem; }

  //Returns size of allocated array
  _SizeTypeArg capacity(void) const { return _heapBufSz ? _heapBufSz : _orgSz; }

  //Returns address of elements array (single memory block)
  const _TArg * get(void) const { return _numElem ? _buf : 0; }

  //Returns initialized/assigned element of array at specified index.
  //Throws if specified index exceeds maximal allowed array capacity.
  //if specified index is beyond of the space of initialized elemens
  //but within capacity, all elements up to specified one are initialized.
  _TArg & at(_SizeTypeArg use_idx) //throw(std::exception)
  {
    if (use_idx < npos()) {
      if (use_idx >= capacity())
        reallocBuf(use_idx + 1);
    } else
      denyIndex(use_idx);

    if (use_idx >= _numElem) {
      LWArrayTraits<_TArg>::construct(_buf + _numElem, (_SizeTypeArg)(use_idx - _numElem + 1));
      _numElem = use_idx + 1;
    }
    return _buf[use_idx];
  }

  //Returns initialized/assigned element of array.
  //Throws if specified index is beyond of the space of initialized elemens.
  const _TArg & at(_SizeTypeArg use_idx) const //throw(std::exception)
  {
    if (use_idx >= _numElem)
      denyIndex(use_idx);
    return ((const _TArg *)_buf)[use_idx];
  }

  //Destroys number of elements at given position within array.
  //Throws if specified index is beyond of the space of initialized elemens.
  void erase(_SizeTypeArg at_pos, _SizeTypeArg num_to_erase = 1) //throw(std::exception)
  {
    if (at_pos >= _numElem)
      denyIndex(at_pos);
    if ((at_pos + num_to_erase) > _numElem)
      num_to_erase = _numElem - at_pos;

    //copy elements starting from first to last
    LWArrayTraits<_TArg>::shift_left(_buf + at_pos, (_SizeTypeArg)(_numElem - at_pos), num_to_erase);
    //destroy excessive elements
    LWArrayTraits<_TArg>::destroy(_buf + _numElem - num_to_erase, num_to_erase);
    _numElem -= num_to_erase;
  }

  //Returns last initialized/assigned element of array.
  //If array is empty the first element is initialized.
  _TArg & atLast(void) //throw()
  {
    if (!_numElem) {
      new (_buf)_TArg();
      _numElem = 1;
    }
    return _buf[_numElem - 1];
  }

  //Returns last initialized/assigned element of array.
  //Throws if array is empty.
  const _TArg & atLast(void) const //throw(std::exception)
  {
    if (!_numElem)
      denyIndex(0);
    return ((const _TArg *)_buf)[_numElem - 1];
  }
  //
  _TArg & operator[](_SizeTypeArg use_idx) //throw(std::exception)
  {
    return at(use_idx);
  }
  //Returns initialized/assigned element of array at pos 'use_idx'
  //Throws if specified index is beyond of the space of initialized elemens.
  const _TArg & operator[](_SizeTypeArg use_idx) const //throw(std::exception)
  {
    return at(use_idx);
  }
  //
  void clear(void)
  {
    LWArrayTraits<_TArg>::destroy(_buf, _numElem);
    _numElem = 0;
  }

  //Enlarges buffer capacity if current one unable to hold additional number
  //of elements (performs check for _SizeTypeArg overloading).
  //Returns false if resulting size exceeds _MAX_SIZE()
  bool enlarge(_SizeTypeArg add_sz) //throw()
  {
    _SizeTypeArg reqSz = _numElem + add_sz;
    if (reqSz < _numElem) //check for _SizeTypeArg overloading;
      return false;

    if (reqSz > capacity()) //heap buffer reallocation is required
      reallocBuf(reqSz);
    return true;
  }

  //Reserves space for storing given number of elements
  //Returns false if requested capacity exceeds _MAX_SIZE().
  void reserve(_SizeTypeArg num_to_reserve) //throw()
  {
    if (num_to_reserve > capacity())
      reallocBuf(num_to_reserve);
  }

  //Alters the array size.  If the new size is greater than the current
  //one, then (req_sz - size()) instances of the default value of type
  //_TArg are inserted at the end of the array.
  //If the new size is smaller than the current size(), then the array
  //is truncated by erasing *size() - req_sz) elements off the end.
  //Returns true on success.
  bool resize(_SizeTypeArg req_sz) //throw()
  {
    if (req_sz == _numElem)
      return true;
    if (req_sz < _numElem) { //shrink
      LWArrayTraits<_TArg>::destroy(_buf + req_sz, (_SizeTypeArg)(_numElem - req_sz));
      _numElem = req_sz;
      return true;
    }
    if (req_sz > _numElem) {
      reserve(req_sz);
      LWArrayTraits<_TArg>::construct(_buf + _numElem, (_SizeTypeArg)(req_sz - _numElem));
      _numElem = req_sz;
      return true;
    }
    return false;
  }

  //Appends element to array
  //Returns false if resulting number of elements would exceed limit
  bool append(const _TArg & use_val) //throw()
  {
    bool rval = enlarge(1);
    if (rval)
      new (_buf + _numElem++)_TArg(use_val);
    return rval;
  }
  //Appends number of elements
  //Returns false if resulting number of elements would exceed limit
  bool append(const _TArg * use_arr, _SizeTypeArg use_num) //throw()
  {
    bool rval = enlarge(use_num);
    if (rval) { //copy elements
      LWArrayTraits<_TArg>::construct(_buf + _numElem, use_arr, use_num);
      _numElem += use_num;
    }
    return rval;
  }

  //shifts array elements to left (shrinks specified number of elements at start)
  void shiftLeft(_SizeTypeArg shift_sz, _SizeTypeArg at_pos = 0) //throw()
  {
    //copy elements starting from first to last
    LWArrayTraits<_TArg>::shift_left(_buf + at_pos, (_SizeTypeArg)(_numElem - at_pos), shift_sz);
    //reset excessive elements
    LWArrayTraits<_TArg>::reset(_buf + _numElem - shift_sz, shift_sz);
    _numElem -= shift_sz;
  }

  //shifts array right (inserts empty elements at start)
  bool shiftRight(_SizeTypeArg shift_sz, _SizeTypeArg at_pos = 0) //throw()
  {
    if (!shiftRightOnly(shift_sz, at_pos))
      return false;
    LWArrayTraits<_TArg>::reset(_buf, shift_sz);
    return true;
  }

  //inserts element at start of array shifting existing ones.
  void prepend(const _TArg & use_val)  //throw(std::exception)
  {
    if (!shiftRightOnly(1))
      denyAddition(1); //throws
    _buf[0] = use_val;
    return;
  }

  //inserts number of elements at start of array shifting existing ones.
  //Returns false if resulting number of elements would exceed limit
  void prepend(const _TArg * use_arr, _SizeTypeArg use_num) //throw(std::exception)
  {
    if (!shiftRightOnly(use_num))
      denyAddition(use_num); //throws

    //copy elements
    LWArrayTraits<_TArg>::copy(_buf, use_arr, use_num);
    return;
  }

  //inserts elements from given array at start of this array
  //shifting existing ones
  void prepend(const LWArrayExtension_T & use_arr)  //throw(std::exception)
  {
    if (!shiftRight(use_arr.size()))
      denyAddition(use_arr.size()); //throws

    //copy elements
    LWArrayTraits<_TArg>::copy(_buf, use_arr.get(), use_arr.size());
    _numElem += use_arr.size();
    return;
  }

  //Appends another lightweight array.
  //Returns false if resulting number of elements would exceed limit
  bool append(const LWArrayExtension_T & use_arr) //throw()
  {
    return append(use_arr.get(), use_arr.size());
  }


  //NOTE: throws if resulting number of elements would exceed limit 
  LWArrayExtension_T & operator+= (const _TArg & use_val) //throw(std::exception)
  {
    if (!append(use_val))
      denyAddition(1);
    return *this;
  }
  //NOTE: throws if resulting number of elements would exceed limit
  LWArrayExtension_T & operator+= (const LWArrayExtension_T & use_arr) //throw(std::exception)
  {
    if (!append(use_arr))
      denyAddition(use_arr.size());
    return *this;
  }

  LWArrayExtension_T & operator= (const LWArrayExtension_T & use_arr) //throw()
  {
    if (this == &use_arr)
      return *this;

    clear();
    append(use_arr); //cann't fail here
    return *this;
  }

  LWArrayExtension_T & operator>> (_SizeTypeArg shift_sz) //throw(std::exception)
  {
    if (!shiftRight(shift_sz))
      denyAddition(shift_sz); //throws
    return *this;
  }
  
  LWArrayExtension_T & operator<< (_SizeTypeArg shift_sz) //throw()
  {
    shiftLeft(shift_sz);
    return *this;
  }

  //Sorts the elements of array into ascending order
  void sort(void) { std::sort(get(), get() + size()); }

  // -------------------------------------------------------------------
  // NOTE: following methods are applicable only to sorted array, i.e.
  // the array that either was explicitly sorted by sort() call, or was
  // filled in by insert() method only.
  // -------------------------------------------------------------------

  //Inserts element into sorted array preserving ascending order.
  //Returns position at which element was successfully inserted or
  //Not-A-Position if insertion failed because of limit of number
  //of elements is already reached.
  _SizeTypeArg insert(const _TArg & use_val) //throw()
  {
    _SizeTypeArg atPos = _numElem ? lower_bound_pos(use_val) : 0;

    if (atPos >= _numElem) { //all elements are less then given one
      if (!append(use_val))
        return npos();
    } else {
      if (!shiftRightOnly(1, atPos))
        return npos();
      _buf[atPos] = use_val;
    }
    return atPos;
  }

  //Returns the position of first element that is grater or equal to given value.
  //If no such element exists, Not-a-Position is returned.
  _SizeTypeArg lower_bound(const _TArg & use_val) const //throw()
  {
    _SizeTypeArg atPos = _numElem ? lower_bound_pos(use_val) : 0;
    return  (atPos < _numElem) ? atPos : npos();
  }

  //Returns the position of first element that is equal to given value.
  //If no such element exists, Not-A-Position is returned.
  _SizeTypeArg find(const _TArg & use_val) const //throw()
  {
    _SizeTypeArg atPos = lower_bound_pos(use_val);
    return ((atPos < _numElem)
            && !(_buf[atPos] < use_val) && !(use_val < _buf[atPos])) ?
                atPos : npos();
  }
};


/* ************************************************************************* *
 * Lightweight Array: store elements on stack until its number doesn't
 * exceed specified limit.
 * NOTE: array reallocation occurs only in case of explicit demand! 
 * NOTE: Element type must have operator<() defined in order to maintain 
 *       sorted array.
 * ************************************************************************* */
template <
    class _TArg                 //Element of array, must have a default & copying constructors!
  , typename _SizeTypeArg       //must be an unsigned integer type, implicitly
                                //restricts maximum number of elements in array!
  , _SizeTypeArg _max_STACK_SZ  //maximum number of elements are to store on stack
  , class _ResizerArg = LWArrayResizerDflt<_SizeTypeArg>
>
class LWArray_T : public LWArrayExtension_T<_TArg, _SizeTypeArg, _ResizerArg> {
private:
  union {
    uint8_t  _buf[_max_STACK_SZ * sizeof(_TArg)];
    void *   _alignedPtr;
  } _stack;

public:
  typedef LWArrayExtension_T<_TArg, _SizeTypeArg, _ResizerArg> base_type;
  typedef typename LWArrayExtension_T<_TArg, _SizeTypeArg, _ResizerArg>::size_type size_type;

  explicit LWArray_T(_SizeTypeArg num_to_reserve = 0) //throw()
    : LWArrayExtension_T<_TArg, _SizeTypeArg, _ResizerArg>(_max_STACK_SZ, (_TArg *)_stack._buf, 0)
  {
    _stack._alignedPtr = 0;
    if (num_to_reserve)
      reserve(num_to_reserve);
  }
  LWArray_T(const _TArg * use_arr, _SizeTypeArg num_elem) //throw()
    : LWArrayExtension_T<_TArg, _SizeTypeArg, _ResizerArg>(_max_STACK_SZ, (_TArg *)_stack._buf, 0)
  {
    _stack._alignedPtr = 0;
    append(use_arr, num_elem); //NOTE: here append() cann't fail
  }
  //
  LWArray_T(const LWArray_T & use_arr) //throw()
    : LWArrayExtension_T<_TArg, _SizeTypeArg, _ResizerArg>(_max_STACK_SZ, (_TArg *)_stack._buf, 0)
  {
    _stack._alignedPtr = 0;
    append(use_arr); //NOTE: here append() cann't fail
  }
  template <_SizeTypeArg _SZArg>
  LWArray_T(const LWArray_T<_TArg, _SizeTypeArg, _SZArg, _ResizerArg>& use_arr) //throw()
    : LWArrayExtension_T<_TArg, _SizeTypeArg, _ResizerArg>(_max_STACK_SZ, (_TArg *)_stack._buf, 0)
  {
    _stack._alignedPtr = 0;
    append(use_arr); //NOTE: here append() cann't fail
  }

  ~LWArray_T()
  {
    if (!this->isHeapBuf()) //elements on private stack were never moved, so destroy them
      this->clear();
  }

  LWArray_T & operator= (const LWArray_T& use_arr) //throw()
  {
    if (this == &use_arr)
      return *this;
    this->clear();
    append(use_arr.get(), use_arr.size()); //cann't fail here
    return *this;
  }

  template <_SizeTypeArg _SZArg>
  LWArray_T & operator= (const LWArray_T<_TArg, _SizeTypeArg, _SZArg, _ResizerArg> & use_arr) //throw()
  {
    this->clear();
    append(use_arr.get(), use_arr.size()); //cann't fail here
    return *this;
  }
};

} //util
} //eyeline

#endif /* __UTIL_LWARRAY_DEFS__ */

