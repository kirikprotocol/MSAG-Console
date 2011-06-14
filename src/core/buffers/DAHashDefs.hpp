/* ************************************************************************** *
 * Direct access hash table housekeeping structures definitions. 
 * Note: assumed that hash table size type is uint32_t.
 * ************************************************************************** */
#ifndef __CORE_BUFFERS_DIRECT_ACCESS_HASH_TRAITS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __CORE_BUFFERS_DIRECT_ACCESS_HASH_TRAITS

#include <inttypes.h>

namespace smsc {
namespace core {
namespace buffers {

// -----------------------------------------------------------------
// Solid structure aggregating direct access hash key and value
// -----------------------------------------------------------------
template <
  class  _KeyArg //required default & copying constructors
, class  _TArg   //required default & copying constructors
>
class HKeyValue_T {
protected:
  _KeyArg   _key;

public:
  mutable _TArg   _value;

  HKeyValue_T()
  { }
  //
  HKeyValue_T(const _KeyArg & use_key, const _TArg & use_val)
    : _key(use_key), _value(use_val)
  { }
  //
  explicit HKeyValue_T(const HKeyValue_T & cp_obj)
    : _key(cp_obj._key), _value(cp_obj._value)
  { }
  //
  ~HKeyValue_T()
  { }

  const _KeyArg & key(void) const { return _key; }
};

// -----------------------------------------------------------------
// Header of slot of direct access hash table.
// -----------------------------------------------------------------
class DAHashSlotHeader {
protected:
  uint16_t    _refs;  //slot refCounts, Note: MSB - is a initialized value flag

public:
  DAHashSlotHeader() : _refs(0)
  { }
  DAHashSlotHeader(const DAHashSlotHeader & cp_obj) : _refs(cp_obj._refs)
  { }
  ~DAHashSlotHeader()
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

// --------------------------------------------------------------------
// Slot of direct access hash table optimized for storing POD-objects
// --------------------------------------------------------------------
template <
  class  _KeyArg //MUST BE a POD-object!
, class  _TArg   //MUST BE a POD-object!
>
class DAHashSlotPOD_T : public DAHashSlotHeader {
public:
  typedef HKeyValue_T<_KeyArg, _TArg> value_type;

protected:
  value_type  _kv;

public:
  DAHashSlotPOD_T() : DAHashSlotHeader()
  { }
  DAHashSlotPOD_T(const DAHashSlotPOD_T & cp_obj) : DAHashSlotHeader(cp_obj)
  {
    if (hasValue())
      init(cp_obj.get().key(), cp_obj.get()._value);
  }
  ~DAHashSlotPOD_T()
  { }

  DAHashSlotPOD_T & operator=(const DAHashSlotPOD_T & cp_obj)
  {
    _refs = cp_obj._refs;
    if (hasValue())
      init(cp_obj.get().key(), cp_obj.get()._value);
    else
      init();
    return *this;
  }

  static void construct(DAHashSlotPOD_T * use_dst, const DAHashSlotPOD_T * use_src, uint32_t num_elem)
  {
    memcpy(use_dst, use_src, sizeof(DAHashSlotPOD_T)*num_elem);
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

// ----------------------------------------------------------------------
// Slot of direct access hash table optimized for storing objects, which
// have complex constructor/destructor.
// ----------------------------------------------------------------------
template <
  class  _KeyArg //required default & copying constructors, operator=()
, class  _TArg   //required default & copying constructors, operator=()
>
class DAHashSlot_T : public DAHashSlotHeader {
public:
  typedef HKeyValue_T<_KeyArg, _TArg> value_type;

protected:
  union {
    uint8_t   _buf[sizeof(value_type)];
    void *    _aligner;
  } _mem;

public:
  DAHashSlot_T() : DAHashSlotHeader()
  { }
  DAHashSlot_T(const DAHashSlot_T & cp_obj) : DAHashSlotHeader(cp_obj)
  {
    if (hasValue())
      init(cp_obj.get().key(), cp_obj.get()._value);
  }
  ~DAHashSlot_T()
  {
    clear();
  }

  DAHashSlot_T & operator=(const DAHashSlot_T & cp_obj)
  {
    clear();
    _refs = cp_obj._refs;
    if (hasValue())
      init(cp_obj.get().key(), cp_obj.get()._value);
    return *this;
  }

  static void construct(DAHashSlot_T * use_dst, const DAHashSlot_T * use_src, uint32_t num_elem)
  {
    for (uint32_t i = 0; i < num_elem; ++i)
      new (use_dst + i)DAHashSlot_T(use_src[i]);
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
      get().~HKeyValue_T<_KeyArg, _TArg>();
      _refs &= 0x7fff;
    }
  }
};


// -----------------------------------------------------------------
// Structure holding direct access hash table data.
// -----------------------------------------------------------------
template <
  class  _KeyArg     //required default & copying constructors, operator=()
, class  _TArg       //required default & copying constructors, operator=()
, template <typename _K, class _T>
  class     _SlotTArg   //required DAHashSlot_T<_KeyArg, _TArg> interface
                = DAHashSlot_T //assume complex objects by default
>
class DAHashTable_T {
public:
  typedef uint32_t  size_type;
  typedef HKeyValue_T<_KeyArg, _TArg> value_type;
  typedef _SlotTArg<_KeyArg, _TArg> slot_type;

protected:
  size_type     _size;  //size of allocated array
  slot_type *   _kvs;   //array of {refCnts, key, value} structures

public:
  static const size_type _maxSize = (size_type)(-1);

  DAHashTable_T() : _size(0), _kvs(0)
  { }
  DAHashTable_T(const DAHashTable_T & src_obj) : _size(src_obj._size), _kvs(0)
  {
    _kvs = (slot_type *)(new uint8_t[sizeof(slot_type) * src_obj._size]);
    slot_type::construct(_kvs, src_obj._kvs, src_obj._size);
  }
  ~DAHashTable_T()
  {
    destroy();
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
    return (at(at_idx).key() == use_key);
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
  void swap(DAHashTable_T & src_obj)
  {
    DAHashTable_T tmp;
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
  DAHashTable_T & operator=(const DAHashTable_T & src_obj)
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
>
const uint32_t DAHashTable_T<_KeyArg, _TArg, _SlotTArg>::_maxSize;

}//buffers
}//core
}//smsc

#endif /* __CORE_BUFFERS_DIRECT_ACCESS_HASH_TRAITS */

