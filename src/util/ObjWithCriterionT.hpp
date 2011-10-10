/* ************************************************************************** *
 * Template that heps to organize(sort) objects according to some criterion.
 * ************************************************************************** */
#ifndef __UTIL_OBJ_WITH_CRITERION_T
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __UTIL_OBJ_WITH_CRITERION_T

#include <functional>

namespace smsc {
namespace util {

template <
  typename _CriterionArg //Must have operator<() defined!
>
class ObjWithCriterion_T {
public:
  typedef _CriterionArg criterion_type;

  ~ObjWithCriterion_T()
  { }

  void setCriterion(const criterion_type & use_crit) { mCriterion = use_crit; }
  const criterion_type & getCriterion(void) const { return mCriterion; }

  bool operator< (const criterion_type & crit_val) const
  {
    return (getCriterion() < crit_val);
  }
  bool operator< (const ObjWithCriterion_T & cmp_obj) const
  {
    return (getCriterion() < cmp_obj.getCriterion());
  }

  //Performs a '3way-compare'
  short compare3way(const criterion_type & cmp_crit) const
  {
    return (mCriterion < cmp_crit) ? -1 : ((cmp_crit < mCriterion) ? 1 : 0);
  }
  short compare3way(const ObjWithCriterion_T & cmp_val) const
  {
    return compare3way(cmp_val.getCriterion());
  }

private:
  criterion_type mCriterion;

protected:
  ObjWithCriterion_T()
  { }
  explicit ObjWithCriterion_T(const criterion_type & use_crit) : mCriterion(use_crit)
  { }
};


template <class _TArg /* : public ObjWithCriterion_T<> */>
struct less_crit : public std::binary_function<_TArg, _TArg, bool> {
  bool operator()(const _TArg & obj_1, const _TArg & obj_2) const
  {
    return (obj_1.getCriterion() < obj_2.getCriterion());
  }
  bool operator()(const _TArg & obj_1, const typename _TArg::criterion_type & crit_val) const
  {
    return (obj_1.getCriterion() < crit_val);
  }
  bool operator()(const typename _TArg::criterion_type & crit_val, const _TArg & obj_1) const
  {
    return (crit_val < obj_1.getCriterion());
  }
};

} //util
} //smsc
#endif /* __UTIL_OBJ_WITH_CRITERION_T */

