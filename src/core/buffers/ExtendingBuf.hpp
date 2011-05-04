#ifndef __SMSC_CORE_EXTENDING_BUFFER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_CORE_EXTENDING_BUFFER_HPP

#include <algorithm>

namespace smsc {
namespace core {
namespace buffers {

//Similar to smsc::core::buffers::TmpBuf (that is a pure serial buffer).
//Difference is the possibility to not just append but also write data at any
//position within buffer. Reading from buffer always performed with check for
//ABR and effective data size.
//May be allocated both on stack and heap. 
//
//    0                               dataSz
//    |                                  |
//    |-------- pos --------------------->
//               |
//             Write()/Append()
// 

template <
  class _TArg                       //must have default constructor & operator=() defined
, typename _SizeTypeArg = unsigned  //must be an unsigned integer type
>
class BufferExtension_T {
protected:
  const _SizeTypeArg _orgSz;
  //
  _TArg *       _dataBuf;
  _TArg *       _heapBuf;
  _SizeTypeArg  _heapBufSz;  //actual size of allocated heap buffer
  _SizeTypeArg  _pos;        //current position within buffer for reading/writing data
  _SizeTypeArg  _dataSz;     //effective size of data in buffer (stack or heap), starting from pos = 0

  BufferExtension_T(_TArg * org_buf, _SizeTypeArg org_max_sz,
                    _SizeTypeArg org_data_sz, _SizeTypeArg org_pos)
    : _orgSz(org_max_sz), _dataBuf(org_buf)
    , _heapBuf(0), _heapBufSz(0), _pos(org_pos), _dataSz(org_data_sz)
  { }

public:
  typedef _SizeTypeArg  size_type;

  virtual ~BufferExtension_T()
  {
    if (_heapBuf)
      delete [] _heapBuf;
    //else keep original buffer intact, it's a successor responsibility to cleanup it.
  }

  static const size_type MAX_POS = (size_type)(-1);

  _TArg *   get(void)         const { return _dataBuf; }
  _TArg *   getCurPtr(void)   const { return _dataBuf + _pos; }

  size_type  getPos(void)      const { return _pos; }
  size_type  getDataSize(void) const { return _dataSz; }
  size_type  getMaxSize(void)  const { return _heapBufSz ? _heapBufSz : _orgSz; }

  //resets buffer, destroys all previously set data
  _TArg * reset(void)
  {
    _pos = _dataSz = 0;
    return _dataBuf;
  }
  //resets buffer, resizing it if needed, destroys all previously set data
  _TArg * reset(size_type size)
  {
    if (_heapBuf) {
      if (size > _heapBufSz) { //reallocate heap buffer
        delete [] _heapBuf;
        _dataBuf = _heapBuf = new _TArg[_heapBufSz = size];
      }
    } else if (size > _orgSz) { //switch to heap buffer
      _dataBuf = _heapBuf = new _TArg[_heapBufSz = size];
    }
    _pos = _dataSz = 0;
    return _dataBuf;
  }

  //Extends buffer size to hold at least the given number of elements
  //Returns true on buffer resizing
  bool extend(size_type new_size)
  {
    if (_heapBuf) {
      if (new_size > _heapBufSz) { //reallocate heap buffer
        _heapBuf = new _TArg[_heapBufSz = new_size];
        std::copy(_dataBuf, _dataBuf + _dataSz, _heapBuf);
        delete [] _dataBuf;
        _dataBuf = _heapBuf;
        return true;
      }
    } else if (new_size > _orgSz) { //switch to heap buffer
      _heapBuf = new _TArg[_heapBufSz = new_size];
      std::copy(_dataBuf, _dataBuf + _dataSz, _heapBuf);
      _dataBuf = _heapBuf;
      return true;
    }
    return false;
  }

  //resizes buffer preserving current data, truncates data in buffer if needed
  _TArg * resize(size_type new_size)
  {
    if (_dataSz > new_size) { //truncate effective data size and position
      _dataSz = new_size;
      if (_pos > new_size)
        _pos = new_size;
    } else 
      extend(new_size);
    return _dataBuf;
  }

  //Sets current position for Read/Append operations, extending buffer
  //and effective data size if necessary
  void setPos(size_type new_pos)
  { 
    if (new_pos > getMaxSize())
      extend(new_pos + 1);
    if (_dataSz < new_pos)
      _dataSz = new_pos;
    _pos = new_pos;
  }
  //Sets effective buffer data size, extending buffer if necessary
  void setDataSize(size_type new_dsz)
  { 
    if (new_dsz > getMaxSize())
      extend(new_dsz);
    _dataSz = new_dsz;
  }

  //Reads requested number of elements at current position
  //and adjusts it if requested.
  //Checks for ABR, returns number of objects have been read.
  size_type Read(_TArg * dst_buf, size_type use_count, bool adjust_pos = true)
  {
    if ((_pos + use_count) >= _dataSz)
      use_count = _dataSz - _pos;
    if (use_count) {
      std::copy(_dataBuf + _pos, _dataBuf + _pos + use_count, dst_buf);
      if (adjust_pos)
        _pos += use_count;
    }
    return use_count;
  }

  //Copies requested number of elements at specified position.
  //Checks for ABR, returns number of objects have been read.
  size_type Copy(_TArg * dst_buf, size_type pos_at, size_type use_count) const
  {
    if (pos_at >= _dataSz)
      return 0;

    unsigned pos_end = pos_at + use_count;
    if (pos_end >= _dataSz)
      use_count = _dataSz - pos_at;
    if (use_count)
      std::copy(_dataBuf + pos_at, _dataBuf + pos_at + use_count, dst_buf);
    return use_count;
  }

  //Writes data to buffer at given position, extending buffer and 
  //effective data size if necessary. Doesn't change current position!!!
  //Returns number of elements written (may be less than specified).
  size_type Write(size_type use_pos, const _TArg * data, size_type use_count)
  {
    size_type newSz = use_pos + use_count;
    if (newSz < use_pos) //check size_type overloading
      use_count = MAX_POS - use_pos;

    if (use_count) {
      if (newSz > getMaxSize()) {
        extend(newSz);
        _dataSz = newSz;
      } else if (newSz > _dataSz)
        _dataSz = newSz;
      std::copy(data, data + use_count, _dataBuf + use_pos);
    }
    return use_count;
  }

  //Writes/Appends data to buffer starting from current position and adjusts it.
  //NOTE: if position is set within existing data, it's overwritten.
  //Returns number of elements appended (may be less than specified).
  size_type Append(const _TArg * data, size_type use_count)
  {
    size_type n = Write(_pos, data, use_count);
    _pos += n;
    return n;
  }

  operator _TArg * () { return _dataBuf; }

  BufferExtension_T & operator=(const BufferExtension_T & _Right)
  {
    if (this != &_Right) {
      this->reset(_Right.getMaxSize()); //pos = 0
      this->Append(_Right.get(), _Right.getDataSize());
      this->setPos(_Right.getPos());
    }
    return (*this);
  }
  BufferExtension_T & operator+=(const BufferExtension_T & _Right)
  {
    this->Append(_Right.get(), _Right.getDataSize());
    return (*this);
  }
};

template <
  class _TArg            //must have default constructor & operator=() defined
, typename _SizeTypeArg  //must be an unsigned integer type
>
const _SizeTypeArg BufferExtension_T<_TArg, _SizeTypeArg>::MAX_POS;


template <
  class _TArg           //must have default & copying constructors
, typename _SizeTypeArg //must be an integer type
, _SizeTypeArg _STACK_SZ  //max stack buffer size
>
class ExtendingBuffer_T : public BufferExtension_T<_TArg, _SizeTypeArg> {
protected:
  union {
    _TArg     buf[_STACK_SZ];
//    uint64_t  alignerInt;
    void *    alignerPtr;
  } _stack;

public:
  typedef BufferExtension_T<_TArg, _SizeTypeArg>  BaseT;
  using BufferExtension_T<_TArg, _SizeTypeArg>::size_type;
  using BufferExtension_T<_TArg, _SizeTypeArg>::operator+=;

  explicit ExtendingBuffer_T(_SizeTypeArg initial_size = _STACK_SZ)
    : BufferExtension_T<_TArg, _SizeTypeArg>(_stack.buf, _STACK_SZ, 0, 0)
  {
    _stack.alignerPtr = 0;
    if (initial_size > _STACK_SZ)
      this->extend(initial_size);
  }
  ~ExtendingBuffer_T()
  { }

  ExtendingBuffer_T & operator=(const ExtendingBuffer_T & _Right)
  {
    if (this != &_Right)
      BufferExtension_T<_TArg, _SizeTypeArg>::operator=(_Right);
    return (*this);
  }

  ExtendingBuffer_T & operator+=(const ExtendingBuffer_T & _Right)
  {
    this->Append(_Right.get(), _Right.getDataSize());
    return (*this);
  }

  template <_SizeTypeArg _SZ_Arg>
  ExtendingBuffer_T & 
    operator=(const ExtendingBuffer_T<_TArg, _SizeTypeArg, _SZ_Arg> & _Right)
  {
    BufferExtension_T<_TArg, _SizeTypeArg>::operator=(_Right);
    return (*this);
  }
   
  template <_SizeTypeArg _SZ_Arg>
  ExtendingBuffer_T & 
    operator+=(const ExtendingBuffer_T<_TArg, _SizeTypeArg, _SZ_Arg> & _Right)
  {
    this->Append(_Right.get(), _Right.getDataSize());
    return (*this);
  }
};


}//namespace buffers
}//namespace core
}//namespace smsc

#endif /* __SMSC_CORE_EXTENDING_BUFFER_HPP */

