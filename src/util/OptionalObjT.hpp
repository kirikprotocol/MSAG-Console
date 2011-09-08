/* ************************************************************************* *
 * Helper template class for creation of arbitrary object in preallocated memory.
 * ************************************************************************* */
#ifndef __SMSC_UTIL_OPTIONAL_OBJECT_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_UTIL_OPTIONAL_OBJECT_HPP

namespace smsc {
namespace util {

template <
  class _TArg  //must have public default constructor and destructor
> 
class OptionalObj_T {
private:
  union {
    void *        mAligner;
    unsigned char mBuf[sizeof(_TArg)];
  }     mMem;

protected:
  bool  mHasObj;

  _TArg * pObj(void) { return (_TArg *)(mMem.mBuf); }
  const _TArg * pObj(void) const { return (const _TArg *)(mMem.mBuf); }

public:
  OptionalObj_T() : mHasObj(false)
  { }
  OptionalObj_T(const OptionalObj_T & use_obj) : mHasObj(use_obj.mHasObj)
  {
    if (use_obj.mHasObj)
      new (mMem.mBuf)_TArg(*use_obj.pObj());
  }
  //
  ~OptionalObj_T()
  {
    clear();
  }

  bool empty(void) const { return mHasObj; }
  //
  void clear(void)
  {
    if (mHasObj) {
      pObj()->~_TArg();
      mHasObj = false;
    }
  }
  //
  _TArg  & init(void)
  {
    clear();
    new (mMem.mBuf)_TArg();
    mHasObj = true;
    return *pObj();
  }
  //
  _TArg  & init(const _TArg & use_obj)
  {
    clear();
    new (mMem.mBuf)_TArg(use_obj);
    mHasObj = true;
    return *pObj();
  }

  //
  _TArg  * get(void) { return mHasObj ? pObj() : 0; }
  //
  const _TArg  * get(void) const { return mHasObj ? pObj() : 0; }
  //
  _TArg * operator->() { return get(); }
  const _TArg * operator->() const { return get(); }

  //
  OptionalObj_T & operator=(const OptionalObj_T & cp_obj)
  {
    if (this != &cp_obj) {
      if (!cp_obj.empty())
        init(*cp_obj.pObj());
      else
        clear();
    }
    return *this;
  }
};

} //util
} //smsc

#endif /* __SMSC_UTIL_OPTIONAL_OBJECT_HPP */

