/* ************************************************************************* *
 * Sorted Lightweight Array: enhances a Lightweight Array by ability to 
 * sort elements according to some criterion, that each objects possess.
 *  
 * NOTE: Element type must be hte successor of ObjWithCriterion_T<>.
 * ************************************************************************* */
#ifndef __CORE_BUFFERS_LWARRAY_SORTED_DEFS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __CORE_BUFFERS_LWARRAY_SORTED_DEFS

#include "util/ObjWithCriterionT.hpp"
#include "core/buffers/LWArrayT.hpp"

namespace smsc {
namespace core {
namespace buffers {

template <
  class _TArg // : public smsc::util::ObjWithCriterion_T<>
              //Element of array, must have default & copying constructors.

, typename _SizeTypeArg   //must be an unsigned integer type, implicitly
                          //restricts maximum number of elements in array!
, template <class _TypArg>
    class _TraitsArg = LWArrayTraits_T //assume non-POD objects by default
, template <typename _SzArg>
    class _ResizerArg = LWArrayResizerDflt
>
class LWArraySortedExtension_T : public LWArrayExtension_T<_TArg, _SizeTypeArg, _TraitsArg, _ResizerArg> {
public:
  typedef _TArg         value_type;
  typedef _SizeTypeArg  size_type;

  typedef typename _TArg::criterion_type        criterion_type;
  typedef typename smsc::util::less_crit<_TArg> comparator_type;

  explicit LWArraySortedExtension_T() : base_type() //throw()
  { }
  //Constructor for array, that extends given buffer
  LWArraySortedExtension_T(size_type org_max_sz, value_type * org_buf,
                           size_type org_num_elem = 0) //throw()
    : base_type(org_max_sz, org_buf, org_num_elem)
  { }
  //
  ~LWArraySortedExtension_T()
  { }

  // -------------------------------------------------------------------
  // NOTE: following methods are applicable only to sorted array, i.e.
  // the array that either was explicitly sorted by sort() call, or was
  // filled in by insert() method only.
  // -------------------------------------------------------------------

  //Returns the position of first element having criterion that is grater or equal to given value.
  //If no such element exists, Not-a-Position is returned.
  size_type lower_bound_crit(const criterion_type & use_val) const //throw()
  {
    if (this->empty())
      return this->npos();

    size_type atPos = lower_bound_crit_pos(use_val);
    return  (atPos < this->size()) ? atPos : this->npos();
  }

  //Returns the position of first element having criterion that is equal to given value.
  //If no such element exists, Not-A-Position is returned.
  size_type find_crit(const criterion_type & use_crit) const //throw()
  {
    size_type atPos = lower_bound_crit(use_crit);
    comparator_type oComp;
    return ((atPos < this->size())
            && !oComp((this->get())[atPos], use_crit)
            && !oComp(use_crit, (this->get())[atPos])) ? atPos : this->npos();
  }

protected:
  typedef LWArrayExtension_T<_TArg, _SizeTypeArg, _TraitsArg, _ResizerArg> base_type;

  //NOTE: it's a responsibility of a successor copying constructor
  //to properly copy referenced (original) buffer if necessary
  explicit LWArraySortedExtension_T(const LWArraySortedExtension_T & use_arr) //throw()
    : base_type(use_arr)
  { }

  //Returns the offset of first element having criterion that is grater or equal to given value.
  //If no such element exists, zero is returned.
  //NOTE: array should not be empty!
  _SizeTypeArg lower_bound_crit_pos(const criterion_type & use_val) const //throw()
  {
    const _TArg * pNext = std::lower_bound(this->get(), this->get() + this->size(), use_val, comparator_type());
    return (pNext - this->get());
  }
};

/* ************************************************************************* *
 * Lightweight Array: 
 * ************************************************************************* */
template <
  class _TArg // : public smsc::util::ObjWithCriterion_T<>
              //Element of array, must have default & copying constructors.

, typename _SizeTypeArg       //must be an unsigned integer type, implicitly
                              //restricts maximum number of elements in array!
, _SizeTypeArg _max_STACK_SZ  //maximum number of elements are to store on stack
, template <class _TypArg>
    class _TraitsArg = LWArrayTraits_T  //assume non-POD objects by default
, template <typename _SzArg>
    class _ResizerArg = LWArrayResizerDflt
>
class LWArraySorted_T : public LWArraySortedExtension_T<_TArg, _SizeTypeArg, _TraitsArg, _ResizerArg> {
private:
  union {
    uint8_t  _buf[_max_STACK_SZ * sizeof(_TArg)];
    void *   _alignedPtr;
  } _stack;

public:
  static const _SizeTypeArg k_dfltSize = _max_STACK_SZ;

  typedef LWArraySortedExtension_T<_TArg, _SizeTypeArg, _TraitsArg, _ResizerArg> base_type;
  typedef typename base_type::size_type size_type;

  explicit LWArraySorted_T(_SizeTypeArg num_to_reserve = 0) //throw()
    : base_type(_max_STACK_SZ, (_TArg *)_stack._buf, 0)
  {
    _stack._alignedPtr = 0;
    if (num_to_reserve)
      this->reserve(num_to_reserve);
  }
  LWArraySorted_T(const _TArg * use_arr, _SizeTypeArg num_elem) //throw()
    : base_type(_max_STACK_SZ, (_TArg *)_stack._buf, 0)
  {
    _stack._alignedPtr = 0;
    this->append(use_arr, num_elem); //NOTE: here append() cann't fail
  }
  //
  LWArraySorted_T(const LWArraySorted_T & use_arr) //throw()
    : base_type(_max_STACK_SZ, (_TArg *)_stack._buf, 0)
  {
    _stack._alignedPtr = 0;
    this->append(use_arr); //NOTE: here append() cann't fail
  }
  template <_SizeTypeArg _SZArg>
  LWArraySorted_T(const LWArraySorted_T<_TArg, _SizeTypeArg, _SZArg, _TraitsArg, _ResizerArg>& use_arr) //throw()
    : base_type(_max_STACK_SZ, (_TArg *)_stack._buf, 0)
  {
    _stack._alignedPtr = 0;
    this->append(use_arr); //NOTE: here append() cann't fail
  }
  //
  ~LWArraySorted_T()
  {
    if (!this->isHeapBuf()) //elements on private stack were never moved, so destroy them
      this->clear();
  }

  LWArraySorted_T & operator= (const LWArraySorted_T& use_arr) //throw()
  {
    if (this == &use_arr)
      return *this;
    this->clear();
    this->append(use_arr.get(), use_arr.size()); //cann't fail here
    return *this;
  }

  template <_SizeTypeArg _SZArg>
  LWArraySorted_T & operator= (const LWArraySorted_T<_TArg, _SizeTypeArg, _SZArg, _TraitsArg, _ResizerArg> & use_arr) //throw()
  {
    this->clear();
    this->append(use_arr.get(), use_arr.size()); //cann't fail here
    return *this;
  }
};

} //buffers
} //core
} //smsc

#endif /* __CORE_BUFFERS_LWARRAY_SORTED_DEFS */

