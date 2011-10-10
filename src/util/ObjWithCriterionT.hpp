/* ************************************************************************** *
 * Template that heps to organize(sort) objects according to some criterion.
 * ************************************************************************** */
#ifndef __UTIL_OBJ_WITH_CRITERION_T
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __UTIL_OBJ_WITH_CRITERION_T

#include <algorithm>

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
  bool operator()(const _TArg & obj_1, const _TArg & obj_2)
  {
    return (obj_1.getCriterion() < obj_2.getCriterion());
  }
  bool operator()(const _TArg & obj_1, const typename _TArg::criterion_type & crit_val)
  {
    return (obj_1.getCriterion() < crit_val);
  }
};

template <
  class _ForwardIter
, class _TArg /* : public ObjWithCriterion_T<> */
>
_ForwardIter lower_bound_crit(_ForwardIter it_first, 
                              const typename std::iterator_traits<_ForwardIter>::difference_type it_range,
                              const typename _TArg::criterion_type & crit_val)
{
  typedef typename std::iterator_traits<_ForwardIter>::difference_type distance_type;
  typedef less_crit<_TArg> compare_type;

  distance_type curRange = it_range;
  distance_type curHalf;
  _ForwardIter itMiddle;

  while (curRange > 0) {
    curHalf = curRange >> 1;
    itMiddle = it_first;
    std::advance(itMiddle, curHalf);
    if (compare_type(*itMiddle, crit_val)) {
      it_first = itMiddle;
      ++it_first;
      curRange = curRange - curHalf - 1;
    } else
      curRange = curHalf;
  }
  return it_first;
}

template <
  class _ForwardIter
, class _TArg /* : public ObjWithCriterion_T<> */
>
_ForwardIter lower_bound_crit(_ForwardIter it_first, _ForwardIter it_last,
                              const typename _TArg::criterion_type & crit_val)
{
  typedef typename std::iterator_traits<_ForwardIter>::difference_type distance_type;
  distance_type curRange = std::distance(it_first, it_last);
  return lower_bound_crit<_ForwardIter, _TArg>(it_first, curRange, crit_val);
}

} //util
} //smsc
#endif /* __UTIL_OBJ_WITH_CRITERION_T */

