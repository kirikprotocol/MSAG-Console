/* ************************************************************************* *
 * Helper templates for union classes.
 * ************************************************************************* */
#ifndef __EYELINE_UTIL_UNION_HPP
#ident "@(#)$Id$"
#define __EYELINE_UTIL_UNION_HPP

#include <inttypes.h>
#include "eyeline/util/MaxSizeof.hpp"

namespace eyeline {
namespace util {

class UnionStorageAC {
protected:
  uint16_t   _altIdx;

  UnionStorageAC(const UnionStorageAC & cp_obj)
    : _altIdx(cp_obj._altIdx)
  { }

public:
  UnionStorageAC() : _altIdx((uint16_t)-1)
  { }
  virtual ~UnionStorageAC()
  { }

  bool empty(void) const { return (_altIdx == (uint16_t)-1); }
  //
  bool isIdx(uint16_t use_val) const { return (_altIdx == use_val); }
  //
  uint16_t getIdx(void) const { return _altIdx; }
  //
  void setIdx(uint16_t use_val) { _altIdx = use_val; }

  //
  virtual void clear(void) = 0;
  virtual void * get(void) = 0;
  virtual const void * get(void) const = 0;
};


//
template <
  class _TArg, uint16_t _altIdxArg
> 
class UnionAlternative_T {
protected:
  UnionStorageAC & _store;

public:
  UnionAlternative_T(UnionStorageAC & use_store)
    : _store(use_store)
  { }
  ~UnionAlternative_T()
  { }

  //
  _TArg & init(void)
  {
    _store.clear();
    _store.setIdx(_altIdxArg);
    return *(new (_store.get())_TArg());
  }
  //
  _TArg * get(void)
  {
    return static_cast<_TArg*>(_store.isIdx(_altIdxArg) ? _store.get() : 0);
  }
  //
  const _TArg * get(void) const
  {
    return static_cast<const _TArg*>(_store.isIdx(_altIdxArg) ? _store.get() : 0);
  }
  //
  void clear(void)
  {
    if (_store.isIdx(_altIdxArg)) {
      _store.clear();
      //_store.setIdx((uint16_t)-1);
    }
  }
};


//
template <
  class _TArg, uint16_t _altIdxArg
> 
class UnionConstAlternative_T {
protected:
  const UnionStorageAC & _store;

public:
  UnionConstAlternative_T(const UnionStorageAC & use_store)
    : _store(use_store)
  { }
  ~UnionConstAlternative_T()
  { }

  //
  const _TArg * get(void) const
  {
    return static_cast<const _TArg*>(_store.isIdx(_altIdxArg) ? _store.get() : 0);
  }
};

/* ************************************************************************* *
 * Helper template for union of two types ...
 * ************************************************************************* */
//
template <
  class _TArg1, class _TArg2
>
class UnionStorageOf2_T : public UnionStorageAC {
protected:
  union {
    void *  _aligner;
    uint8_t _buf[util::MaxSizeOf2_T<_TArg1, _TArg2>::VALUE];
  } _mem;

public:
  UnionStorageOf2_T() : UnionStorageAC()
  {
    _mem._aligner = 0;
  }
  UnionStorageOf2_T(const UnionStorageOf2_T & cp_obj) : UnionStorageAC(cp_obj)
  {
    if (!_altIdx) {
      new (_mem._buf)_TArg1(*(const _TArg1 *)cp_obj.get());
    } else if (_altIdx == 1) {
      new (_mem._buf)_TArg2(*(const _TArg2 *)cp_obj.get());
    } else
      _mem._aligner = 0;
  }
  virtual ~UnionStorageOf2_T()
  {
    clear();
  }

  virtual void clear(void)
  {
    if (!_altIdx) {
      ((_TArg1*)_mem._buf)->~_TArg1();
    } else if (_altIdx == 1) {
      ((_TArg2*)_mem._buf)->~_TArg2();
    }
    _altIdx = (uint16_t)(-1);
  }
  virtual void * get(void) { return _mem._buf; }
  virtual const void * get(void) const { return _mem._buf; }
};


/* ************************************************************************* *
 * Helper template class for union of three types ...
 * ************************************************************************* */
//
template <
  class _TArg1, class _TArg2, class _TArg3
>
class UnionStorageOf3_T : public UnionStorageAC {
protected:
  union {
    void *  _aligner;
    uint8_t _buf[util::MaxSizeOf3_T<_TArg1, _TArg2, _TArg3>::VALUE];
  } _mem;

public:
  UnionStorageOf3_T() : UnionStorageAC()
  {
    _mem._aligner = 0;
  }
  UnionStorageOf3_T(const UnionStorageOf3_T & cp_obj) : UnionStorageAC(cp_obj)
  {
    if (!_altIdx) {
      new (_mem._buf)_TArg1(*(const _TArg1 *)cp_obj.get());
    } else if (_altIdx == 1) {
      new (_mem._buf)_TArg2(*(const _TArg2 *)cp_obj.get());
    } else if (_altIdx == 2) {
      new (_mem._buf)_TArg3(*(const _TArg3 *)cp_obj.get());
    } else
      _mem._aligner = 0;
  }
  virtual ~UnionStorageOf3_T()
  {
    clear();
  }

  virtual void clear(void)
  {
    if (!_altIdx) {
      ((_TArg1*)_mem._buf)->~_TArg1();
    } else if (_altIdx == 1) {
      ((_TArg2*)_mem._buf)->~_TArg2();
    } else if (_altIdx == 2) {
      ((_TArg3*)_mem._buf)->~_TArg3();
    }
    _altIdx = (uint16_t)(-1);
  }
  virtual void * get(void) { return _mem._buf; }
  virtual const void * get(void) const { return _mem._buf; }
};

/* ************************************************************************* *
 * Helper template class for union of four types ...
 * ************************************************************************* */
//
template <
  class _TArg1, class _TArg2, class _TArg3, class _TArg4
>
class UnionStorageOf4_T : public UnionStorageAC {
protected:
  union {
    void *  _aligner;
    uint8_t _buf[util::MaxSizeOf4_T<_TArg1, _TArg2, _TArg3, _TArg4>::VALUE];
  } _mem;

public:
  UnionStorageOf4_T() : UnionStorageAC()
  {
    _mem._aligner = 0;
  }
  UnionStorageOf4_T(const UnionStorageOf4_T & cp_obj) : UnionStorageAC(cp_obj)
  {
    if (!_altIdx) {
      new (_mem._buf)_TArg1(*(const _TArg1 *)cp_obj.get());
    } else if (_altIdx == 1) {
      new (_mem._buf)_TArg2(*(const _TArg2 *)cp_obj.get());
    } else if (_altIdx == 2) {
      new (_mem._buf)_TArg3(*(const _TArg3 *)cp_obj.get());
    } else if (_altIdx == 3) {
      new (_mem._buf)_TArg4(*(const _TArg4 *)cp_obj.get());
    } else
      _mem._aligner = 0;
  }
  virtual ~UnionStorageOf4_T()
  {
    clear();
  }

  virtual void clear(void)
  {
    if (!_altIdx) {
      ((_TArg1*)_mem._buf)->~_TArg1();
    } else if (_altIdx == 1) {
      ((_TArg2*)_mem._buf)->~_TArg2();
    } else if (_altIdx == 2) {
      ((_TArg3*)_mem._buf)->~_TArg3();
    } else if (_altIdx == 3) {
      ((_TArg4*)_mem._buf)->~_TArg4();
    }
    _altIdx = (uint16_t)(-1);
  }
  virtual void * get(void) { return _mem._buf; }
  virtual const void * get(void) const { return _mem._buf; }
};

/* ************************************************************************* *
 * Helper template class for union of five types ...
 * ************************************************************************* */
//
template <
  class _TArg1, class _TArg2, class _TArg3, class _TArg4, class _TArg5
>
class UnionStorageOf5_T : public UnionStorageAC {
protected:
  union {
    void *  _aligner;
    uint8_t _buf[util::MaxSizeOf5_T<_TArg1, _TArg2, _TArg3, _TArg4, _TArg5>::VALUE];
  } _mem;

public:
  UnionStorageOf5_T() : UnionStorageAC()
  {
    _mem._aligner = 0;
  }
  UnionStorageOf5_T(const UnionStorageOf5_T & cp_obj) : UnionStorageAC(cp_obj)
  {
    if (!_altIdx) {
      new (_mem._buf)_TArg1(*(const _TArg1 *)cp_obj.get());
    } else if (_altIdx == 1) {
      new (_mem._buf)_TArg2(*(const _TArg2 *)cp_obj.get());
    } else if (_altIdx == 2) {
      new (_mem._buf)_TArg3(*(const _TArg3 *)cp_obj.get());
    } else if (_altIdx == 3) {
      new (_mem._buf)_TArg4(*(const _TArg4 *)cp_obj.get());
    } else if (_altIdx == 4) {
      new (_mem._buf)_TArg5(*(const _TArg5 *)cp_obj.get());
    } else
      _mem._aligner = 0;
  }
  virtual ~UnionStorageOf5_T()
  {
    clear();
  }

  virtual void clear(void)
  {
    if (!_altIdx) {
      ((_TArg1*)_mem._buf)->~_TArg1();
    } else if (_altIdx == 1) {
      ((_TArg2*)_mem._buf)->~_TArg2();
    } else if (_altIdx == 2) {
      ((_TArg3*)_mem._buf)->~_TArg3();
    } else if (_altIdx == 3) {
      ((_TArg4*)_mem._buf)->~_TArg4();
    } else if (_altIdx == 4) {
      ((_TArg5*)_mem._buf)->~_TArg5();
    }
    _altIdx = (uint16_t)(-1);
  }
  virtual void * get(void) { return _mem._buf; }
  virtual const void * get(void) const { return _mem._buf; }
};

} //util
} //eyeline

#endif /* __EYELINE_UTIL_UNION_HPP */

