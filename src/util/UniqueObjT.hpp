/* ************************************************************************** *
 * Helper template that transforms arbitrary class to UniqueObj: objects have 
 * unique index and objects cann't be copied.
 * ************************************************************************** */
#ifndef __UTIL_UNIQUE_OBJ_T
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __UTIL_UNIQUE_OBJ_T

namespace smsc {
namespace util {

// --------------------------------------------------------------------------
// Transforms object of arbitrary class to UniqueObj (cann't be copied)
// --------------------------------------------------------------------------
template <
  class _TArg       //only default constructor is required !!
, typename _IndexTypeArg = unsigned //MUST be an unsigned integer type
>
class UniqueObj_T : public _TArg {
private:
  const _IndexTypeArg mUnqIdx; //unique index of this object

  UniqueObj_T(const UniqueObj_T & cp_obj);
  UniqueObj_T & operator=(const UniqueObj_T & cp_obj);

protected:
  explicit UniqueObj_T(_IndexTypeArg use_idx) : _TArg(), mUnqIdx(use_idx)
  { }
  ~UniqueObj_T()
  { }

public:
  _IndexTypeArg getUIdx(void) const { return mUnqIdx; }
};

} //util
} //smsc

#endif /* __UTIL_UNIQUE_OBJ_T */

