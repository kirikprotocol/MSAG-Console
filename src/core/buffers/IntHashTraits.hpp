/* ************************************************************************** *
 * 
 * ************************************************************************** */
#ifndef __CORE_BUFFERS_INTHASH__TRAITS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __CORE_BUFFERS_INTHASH__TRAITS

#include "util/IntTypes.hpp"

namespace smsc {
namespace core {
namespace buffers {

// -----------------------------------------------------------------
// Default integer hash key traits:
//   hash function, collisions limit, target collisions percentage.
// -----------------------------------------------------------------
template < 
  typename  _KeyArg                   //must be an integer type (signed or unsigned)
, uint8_t   _CollMaxNumArg
, uint8_t   _CollPercentagePwrArg = 3 //NOTE: must belong to range [1..6], target
                                      //percentage is calculated as := 100/(2**PWR)
                                      //is [50% 25% 12.5% 6.25% 3.125% 1.56%]
>
class IntHashKeyTraitsDflt_T {
public:
  typedef typename smsc::util::IntUnsigner_T<_KeyArg>::unsigned_type  hash_type;

  static const uint8_t  _MAX_COLL_NUM = _CollMaxNumArg;
  //Target overall collisions percentage: = 100/(2**PWR)
  static const uint8_t  _TGT_COLL_PERCENTAGE_PWR = _CollPercentagePwrArg;
  //
  static hash_type hashKey(_KeyArg use_key, uint16_t num_attempt)
  {
    return (hash_type)use_key + num_attempt;
  }
};

// -----------------------------------------------------------------
// 
// -----------------------------------------------------------------
template <
  typename  _KeyArg //must be an integer type (signed or unsigned)
, class     _TArg   //required default & copying constructors
>
struct IKeyValue_T {
  const _KeyArg   _key;
  mutable _TArg   _value;

  IKeyValue_T() : _key(0)
  { }
  //
  IKeyValue_T(const _KeyArg use_key, const _TArg & use_val)
    : _key(use_key), _value(use_val)
  { }
  //
  explicit IKeyValue_T(const IKeyValue_T & cp_obj)
    : _key(cp_obj._key), _value(cp_obj._value)
  { }
  //
  ~IKeyValue_T()
  { }
};


class IHashSlotHeader {
protected:
  uint16_t    _refs;  //slot refCounts, Note: MSB - is a initialized value flag

public:
  IHashSlotHeader() : _refs(0)
  { }
  IHashSlotHeader(const IHashSlotHeader & cp_obj) : _refs(cp_obj._refs)
  { }
  ~IHashSlotHeader()
  { }

  //
  bool hasValue(void) const { return (_refs & 0x8000); }
  //
  bool hasRefs(void) const { return _refs != 0; }
  //
  //Returns false if max number of refs to given slot is already reached
  bool addRef(void)
  {
    if ((_refs & 0x7fff) == 0x7fff)
      return false;
    ++_refs;
    return true;
  }
  //
  void unRef(void) { /*if (_refs)*/ --_refs; }
};

// -----------------------------------------------------------------
// Slot of hash table optimized for storing POD-objects
// -----------------------------------------------------------------
template <
  typename  _KeyArg //must be an integer type (signed or unsigned)
, class     _TArg   //MUST BE a POD-object!!!
>
class IHashSlotPOD_T : public IHashSlotHeader {
public:
  typedef IKeyValue_T<_KeyArg, _TArg> value_type;

protected:
  value_type  _kv;

public:
  IHashSlotPOD_T() : IHashSlotHeader()
  { }
  IHashSlotPOD_T(const IHashSlotPOD_T & cp_obj) : IHashSlotHeader(cp_obj)
  {
    if (hasValue())
      init(cp_obj.get()._key, cp_obj.get()._value);
  }
  ~IHashSlotPOD_T()
  { }

  IHashSlotPOD_T & operator=(const IHashSlotPOD_T & cp_obj)
  {
    _refs = cp_obj._refs;
    if (hasValue())
      init(cp_obj.get()._key, cp_obj.get()._value);
    else
      init();
    return *this;
  }

  static void construct(IHashSlotPOD_T * use_dst, const IHashSlotPOD_T * use_src, uint32_t num_elem)
  {
    memcpy(use_dst, use_src, sizeof(IHashSlotPOD_T)*num_elem);
  }

  //
  value_type & get(void) { return _kv; }

  const value_type & get(void) const { return _kv; }

  _TArg * getValue(void) const { return &(_kv._value); }

  value_type & init(void)
  {
    _refs |= 0x8000;
    return (_kv = value_type()); 
  }

  value_type & init(const _KeyArg use_key, const _TArg & use_val)
  {
    _refs |= 0x8000;
    return (_kv = value_type(use_key, use_val));
  }
  //
  void clear(void)
  {
    _refs &= 0x7fff;
    _kv = value_type();
  }
};

// -----------------------------------------------------------------
// Slot of hash table optimized for storing complex objects
// -----------------------------------------------------------------
template <
  typename  _KeyArg //must be an integer type (signed or unsigned)
, class     _TArg   //required default & copying constructors, operator=()
>
class IHashSlot_T : public IHashSlotHeader {
public:
  typedef IKeyValue_T<_KeyArg, _TArg> value_type;

protected:
  union {
    uint8_t   _buf[sizeof(value_type)];
    void *    _aligner;
  } _mem;

public:
  IHashSlot_T() : IHashSlotHeader()
  { }
  IHashSlot_T(const IHashSlot_T & cp_obj) : IHashSlotHeader(cp_obj)
  {
    if (hasValue())
      init(cp_obj.get()._key, cp_obj.get()._value);
  }
  ~IHashSlot_T()
  {
    clear();
  }

  IHashSlot_T & operator=(const IHashSlot_T & cp_obj)
  {
    clear();
    _refs = cp_obj._refs;
    if (hasValue())
      init(cp_obj.get()._key, cp_obj.get()._value);
    return *this;
  }

  static void construct(IHashSlot_T * use_dst, const IHashSlot_T * use_src, uint32_t num_elem)
  {
    for (uint32_t i = 0; i < num_elem; ++i)
      new (use_dst + i)IHashSlot_T(use_src[i]);
  }

  //
  value_type & get(void) { return *(value_type*)(_mem._buf); }
  //
  const value_type & get(void) const { return *(const value_type*)(_mem._buf); }
  //
  _TArg * getValue(void) const { return &(get()._value); }

  //
  value_type & init(void)
  {
    new (_mem._buf)value_type();
    _refs |= 0x8000;
    return get();
  }
  //
  value_type & init(const _KeyArg use_key, const _TArg & use_val)
  {
    new (_mem._buf)value_type(use_key, use_val);
    _refs |= 0x8000;
    return get();
  }
  //
  void clear(void)
  {
    if (_refs & 0x8000) {
      get().~IKeyValue_T();
      _refs &= 0x7fff;
    }
  }
};


// -----------------------------------------------------------------
// 
// -----------------------------------------------------------------
template <
  typename  _KeyArg     //must be an integer type (signed or unsigned)
, class     _TArg       //required default & copying constructors, operator=()
, template <typename _K, class _T>
  class     _SlotTArg   //required IHashSlot_T<_KeyArg, _TArg> interface
                    = IHashSlotPOD_T
, class     _IHKeyTraitsArg   //required IntHashKeyTraitsDflt_T<_KeyArg> interface
                    = IntHashKeyTraitsDflt_T<_KeyArg, 16, 3>
>
class IHashTable_T {
public:
  typedef uint32_t size_type; //must be an unsigned integer type!
  typedef IKeyValue_T<_KeyArg, _TArg> value_type;
  typedef _SlotTArg<_KeyArg, _TArg> slot_type;

protected:
  size_type     _size;  //size of allocated array
  slot_type *   _kvs;   //array of {refCnts, key, value} structures

public:
  static const size_type _maxSize = (size_type)(-1);

  IHashTable_T() : _size(0), _kvs(0)
  { }
  IHashTable_T(const IHashTable_T & src_obj) : _size(src_obj._size), _kvs(0)
  {
    _kvs = (slot_type *)(new uint8_t[sizeof(slot_type) * src_obj._size]);
    slot_type::construct(_kvs, src_obj._kvs, src_obj._size);
  }
  ~IHashTable_T()
  {
    destroy();
  }

  //
  size_type Index(_KeyArg key, uint16_t num_attempt) const
  {
    return _IHKeyTraitsArg::hashKey(key, num_attempt) % _size;
  }

  size_type size(void) const { return _size; }

  value_type & at(size_type at_idx) { return _kvs[at_idx].get(); }

  const value_type & at(size_type at_idx) const { return _kvs[at_idx].get(); }

  _TArg * getValueAt(size_type at_idx) const { return _kvs[at_idx].getValue(); }

  //
  bool hasRefsAt(size_type at_idx) const { return _kvs[at_idx].hasRefs(); }
  //
  //Returns false if max number of refs to given slot is already reached
  bool addRefAt(size_type at_idx) { return _kvs[at_idx].addRef(); }
  //
  void unRefAt(size_type at_idx) { _kvs[at_idx].unRef(); }
  //
  bool isKeyAt(size_type at_idx, const _KeyArg & use_key) const
  {
    return (at(at_idx)._key == use_key);
  }
  //
  bool hasValueAt(size_type at_idx) const  { return _kvs[at_idx].hasValue(); }

  //Adds a new value to empty slot.
  //Returns false if max number of refs to given slot is already reached
  bool addValueAt(size_type at_idx, const _KeyArg & use_key, const _TArg & use_value)
  {
    if (!_kvs[at_idx].addRef())
      return false; //too much refs
    _kvs[at_idx].init(use_key, use_value);
    return true;
  }
  //Overwrites value in given slot
  void setValueAt(size_type at_idx, const _TArg & use_value)
  {
    at(at_idx)._value = use_value;
  }
  //Resets value presence flag at given slot, and clears (sesets to default) value.
  void delValueAt(size_type at_idx) { _kvs[at_idx].clear(); }
  //
  void allocate(size_type new_size)
  {
    _kvs = new slot_type[_size = new_size];
  }
  //
  void destroy(void)
  {
    if (_size) {
      delete [] _kvs; _kvs = 0;
      _size = 0;
    }
  }
  //
  void swap(IHashTable_T & src_obj)
  {
    IHashTable_T tmp;
    //
    tmp._size = _size;
    tmp._kvs = _kvs;
    //
    _size = src_obj._size;
    _kvs = src_obj._kvs;
    //
    src_obj._size = tmp._size;
    src_obj._kvs = tmp._kvs;
    //
    tmp._size = 0;
  }
  //
  IHashTable_T & operator=(const IHashTable_T & src_obj)
  {
    if (this == &src_obj)
      return *this;

    destroy();
    _kvs = (slot_type *)(new uint8_t[sizeof(slot_type) * src_obj._size]);
    slot_type::construct(_kvs, src_obj._kvs, src_obj._size);
    _size = src_obj._size;
    return *this;
  }
};


template <
  typename  _KeyArg
, class     _TArg
, template <typename _K, class _T>
  class     _SlotTArg
, class     _IHKeyTraitsArg
>
const uint32_t IHashTable_T<_KeyArg, _TArg, _SlotTArg, _IHKeyTraitsArg>::_maxSize;

}//buffers
}//core
}//smsc

#endif /* __CORE_BUFFERS_INTHASH_TRAITS */

