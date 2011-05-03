/* ************************************************************************** *
 * Template of generic direct access hash table of elements addressed by 
 * integer key (signed or unsigned).
 * ************************************************************************** */
#ifndef __CORE_BUFFERS_INTHASH_TEMPLATE
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __CORE_BUFFERS_INTHASH_TEMPLATE

#include "util/IntTypes.hpp"

#include <string.h>
#include <string>
#include <stdexcept>

#if defined(INTHASH_USAGE_CHECKING) || defined(INTHASH_USAGE_DEBUG)
#include <stdio.h>
#include "core/synchronization/Mutex.hpp"
#endif

namespace smsc {
namespace core {
namespace buffers {

template < 
  typename  _KeyArg               //must be an integer type (signed or unsigned)
, uint8_t   _CollMaxNumArg
, uint8_t   _CollPercentagePwrArg //NOTE: must belong to range [1..6], target
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


template <
  class     _TArg               //required: _TArg(); and operator=(const _TArg &);
, typename  _KeyArg = unsigned  //must be an integer type (signed or unsigned)
, class     _IHKeyTraitsArg      //required IntHashKeyTraitsDflt_T<> interface
                    = IntHashKeyTraitsDflt_T<_KeyArg, 16, 3>
>
class IntHash_T {
public:
  typedef uint32_t size_type; //must be an unsigned integer type!

#if defined(INTHASH_USAGE_CHECKING) || defined(INTHASH_USAGE_DEBUG)
  explicit IntHash_T(const char * use_id) : _id(use_id ? use_id : ""),
#else
  IntHash_T() :
#endif
    _valCount(0), _collCount(0)
  { }
  //
  IntHash_T(const IntHash_T& src) :
#if defined(INTHASH_USAGE_CHECKING) || defined(INTHASH_USAGE_DEBUG)
    _id(src._id),
#endif
    _valCount(0), _collCount(0)
  {
    *this = src;
  }

#if defined(INTHASH_USAGE_CHECKING) || defined(INTHASH_USAGE_DEBUG)
  explicit IntHash_T(size_type num_to_reserve, const char* id ) : _id(id),
#else
  explicit IntHash_T(size_type num_to_reserve) :
#endif
    _valCount(0), _collCount(0)
  {
    SetSize(num_to_reserve);
  }
  //
  ~IntHash_T()
  {
    Empty();
  }

  IntHash_T& operator=(const IntHash_T& src_obj)
  {
#if defined(INTHASH_USAGE_CHECKING) || defined(INTHASH_USAGE_DEBUG)
    _id = src_obj._id;
#endif
    _hArr = src_obj._hArr;
    _valCount = src_obj._valCount;
    return *this;
  }

  size_type Count(void) const { return _valCount; }
  size_type Size(void) const { return _hArr.size(); }

  void Empty(void)
  {
    _hArr.destroy();
    _valCount = _collCount = 0;
  }

  void SetSize(size_type num_to_reserve) /*throw()*/
  {
    if (num_to_reserve > _hArr.size()) //Rehash cann't fail here
      Rehash(num_to_reserve < _IHKeyTraitsArg::_MAX_COLL_NUM ? _IHKeyTraitsArg::_MAX_COLL_NUM : num_to_reserve);
  }

  _TArg & Insert(_KeyArg key, const _TArg & value)  /*throw(std::runtime_error)*/
  {
    if (!_hArr.size())
      Rehash(_IHKeyTraitsArg::_MAX_COLL_NUM);
    else if (_valCount == _hArr.size()) {
      size_type newSz = _hArr.size() << 1;
      Rehash(newSz < _hArr.size() ? HashArray::_maxSize : newSz); //throws
    }

    uint16_t  attempt = 0;
    size_type idx;
    bool      newVal = false;

    for (;;) {
      idx = _hArr.Index(key, attempt);
      if (!_hArr.isValueAt(idx)) {  //insert a new value
        if (checkCollisions(attempt) && (newVal = _hArr.addValueAt(idx, key, value))) {
          _collCount += attempt;
#ifdef INTHASH_USAGE_DEBUG
          { smsc::core::synchronization::MutexGuard mg(_idMutex);
            fprintf(stderr,"IntHash_T(%s)::Inserting at idx=%u, attempt=%u, size=%u, _valCount=%u, _collCount=%u\n",
                  _id, idx, (unsigned)attempt, _hArr.size(), _valCount, _collCount);
          }
#endif
          break;
        }
        //else rehash is required
#if defined(INTHASH_USAGE_CHECKING) || defined(INTHASH_USAGE_DEBUG)
        { smsc::core::synchronization::MutexGuard mg(_idMutex);
          fprintf(stderr,"IntHash_T(%s)::Insert need rehashing at attempt=%u, orgSize=%u, _valCount=%u, _collCount=%u\n",
                  _id, (unsigned)attempt, _hArr.size(), _valCount, _collCount);
        }
#endif
        size_type newSz = _hArr.size() << 1;
        Rehash(newSz < _hArr.size() ? HashArray::_maxSize : newSz); //throws
        attempt = 0;
        continue;
      }
      if (_hArr.isKeyAt(idx, key)) {  //overwrite an existing value
#ifdef INTHASH_USAGE_DEBUG
        { smsc::core::synchronization::MutexGuard mg(_idMutex);
          fprintf(stderr,"IntHash_T(%s)::Overwritting at idx=%u, attempt=%u, size=%u, _valCount=%u, _collCount=%u\n",
                  _id, idx, (unsigned)attempt, _hArr.size(), _valCount, _collCount);
        }
#endif
        _hArr.setValueAt(idx, value);
        break;
      }
      if (!_hArr.addRefAt(idx) || (++attempt >= _IHKeyTraitsArg::_MAX_COLL_NUM)) {
#if defined(INTHASH_USAGE_CHECKING) || defined(INTHASH_USAGE_DEBUG)
        { smsc::core::synchronization::MutexGuard mg(_idMutex);
          fprintf(stderr,"IntHash_T(%s)::Insert need rehashing at attempt=%u, orgSize=%u, _valCount=%u, _collCount=%u\n",
              _id, (unsigned)attempt, _hArr.size(), _valCount, _collCount);
        }
#endif
        size_type newSz = _hArr.size() << 1;
        Rehash(newSz < _hArr.size() ? HashArray::_maxSize : newSz); //throws
        attempt = 0;
      }
    }
#if defined(INTHASH_USAGE_CHECKING) || defined(INTHASH_USAGE_DEBUG)
    if (attempt) {
      smsc::core::synchronization::MutexGuard mg(_idMutex);
      fprintf(stderr,"IntHash_T(%s)::Inserted at attempt=%u, orgSize=%u, _valCount=%u, _collCount=%u\n",
              _id, (unsigned)attempt, _hArr.size(), _valCount, _collCount);
    }
#endif
    if (newVal)
      ++_valCount;
    return _hArr._vals[idx];
  }

  const _TArg & Get(_KeyArg key)const
  {
    size_type valIdx;
    if(!findVal(key, valIdx))
      throw std::runtime_error((!_hArr.size() || !_valCount) ? "IntHash_T::Get - empty inthash"
                                                 : "IntHash_T::Get - item not found");
    return _hArr._vals[valIdx];
  }

  _TArg & Get(_KeyArg key)
  {
    size_type valIdx;
    if(!findVal(key, valIdx))
      throw std::runtime_error((!_hArr.size() || !_valCount) ? "IntHash_T::Get - empty inthash"
                                                 : "IntHash_T::Get - item not found");
    return _hArr._vals[valIdx];
  }

  _TArg * GetPtr(_KeyArg key)const
  {
    size_type valIdx;
    return findVal(key, valIdx) ? &_hArr._vals[valIdx] : NULL;
  }

  bool Get(_KeyArg key, _TArg & value) const
  {
    size_type valIdx;
    if(!findVal(key, valIdx))
      return false;
    value = _hArr._vals[valIdx];
    return true;
  }

  bool Exist(_KeyArg key) const
  {
    size_type valIdx;
    return findVal(key, valIdx);
  }

  bool Delete(_KeyArg key)
  {
    size_type valIdx;
    if (!unmarkVal(key, valIdx))
      return false;
    _hArr.resetValueAt(valIdx);
    return true;
  }

  bool Pop(_KeyArg key, _TArg & value)
  {
    size_type valIdx;
    if (!unmarkVal(key, valIdx))
      return false;
    value = _hArr.valueAt(valIdx);
    _hArr.resetValueAt(valIdx);
    return true;
  }

  //Note: it's not recommended to modify cache while accessing its elements
  //via iterators. Iterator may become invalid due to insertion of new elements.
  class Iterator {
  protected:
    size_type         idx;
    const IntHash_T * h;

  public:
    Iterator(const IntHash_T& owner)
      : idx(0), h(&owner)
    { }
    Iterator(const Iterator& src)
      : idx(src.idx), h(src.h)
    { }

    Iterator& operator=(const Iterator& src)
    {
      idx = src.idx;
      h = src.h;
      return *this;
    }

    bool Next(_KeyArg& k, _TArg*& v)
    {
      if ((idx >= h->_hArr.size()) || !h->_valCount)
        return false;
      while ((idx < h->_hArr.size()) && !h->_hArr.isValueAt(idx))
        ++idx;
      if (idx >= h->_hArr.size())
        return false;
      k = h->_hArr.keyAt(idx);
      v = &h->_hArr._vals[idx];
      ++idx;
      return true;
    }

    bool Next(_KeyArg& k, _TArg & v)
    {
      _TArg * pVal = NULL;
      if(!Next(k, pVal))
        return false;
      v = *pVal;
      return true;
    }

    void First(void) { idx = 0; }
  };
  friend class Iterator;

  Iterator First()const
  {
    return Iterator(*this);
  }

private:
  class HashArray {
  protected:
    size_type   _size;  //size of allocated arrays: keys, values, refCnts
    uint16_t *  _refs;  //array of used slots refCounts, Note: MSB - is a assigned value flag
    _KeyArg *   _keys;  //array of keys

  public:
    static const size_type _maxSize = (size_type)(-1);

    _TArg *     _vals;  //array of values

    HashArray() : _size(0), _refs(NULL), _keys(NULL), _vals(NULL)
    { }
    ~HashArray()
    {
      destroy();
    }

    size_type size(void) const { return _size; }
    //
    size_type Index(_KeyArg key, uint16_t num_attempt) const
    {
      return _IHKeyTraitsArg::hashKey(key, num_attempt) % _size;
    }
    //
    bool hasRefsAt(size_type at_idx) const
    {
      return _refs[at_idx] != 0;
    }
    //
    bool isKeyAt(size_type at_idx, const _KeyArg & use_key) const
    {
      return (_keys[at_idx] == use_key);
    }
    //
    const _KeyArg & keyAt(size_type at_idx) const { return _keys[at_idx]; }
    //
    bool isValueAt(size_type at_idx) const
    {
      return (_refs[at_idx] & 0x8000) != 0;
    }
    //
    const _TArg & valueAt(size_type at_idx) const { return _vals[at_idx]; }
    //Returns false if max number of refs to given slot is already reached
    bool addRefAt(size_type at_idx)
    {
      if ((_refs[at_idx] & 0x7fff) == 0x7fff)
        return false;
      ++_refs[at_idx];
      return true;
    }
    //
    void unRefAt(size_type at_idx) { --_refs[at_idx]; }
    //Adds a new value to empty slot.
    //Returns false if max number of refs to given slot is already reached
    bool addValueAt(size_type at_idx, const _KeyArg & use_key, const _TArg & use_value)
    {
      if ((_refs[at_idx] & 0x7fff) == 0x7fff)
        return false;

      _keys[at_idx] = use_key;
      _vals[at_idx] = use_value;
      ++_refs[at_idx];
      _refs[at_idx] |= 0x8000;
      return true;
    }
    //Overwrites value in given slot
    void setValueAt(size_type at_idx, const _TArg & use_value)
    {
      _vals[at_idx] = use_value;
    }
    //Resets value in given slot to default
    void resetValueAt(size_type at_idx)
    {
      _vals[at_idx] = _TArg();
    }
    //Resets value presence flag at given slot
    void rlseValueAt(size_type at_idx)
    {
      _refs[at_idx] &= 0x7fff;
    }
    //
    void allocate(size_type new_size)
    {
      _refs = new uint16_t[new_size];
      memset(_refs, 0, sizeof(uint16_t)*new_size);
      _keys = new _KeyArg[new_size];
      _vals = new _TArg[_size = new_size];
    }
    //
    void destroy(void)
    {
      if (_size) {
        delete [] _refs; _refs = NULL;
        delete [] _keys; _keys = NULL;
        delete [] _vals; _vals = NULL;
        _size = 0;
      }
    }
    //
    void swap(HashArray & src_obj)
    {
      HashArray tmp;
      //
      tmp._size = _size;
      tmp._refs = _refs;
      tmp._keys = _keys;
      tmp._vals = _vals;
      //
      _size = src_obj._size;
      _refs = src_obj._refs;
      _keys = src_obj._keys;
      _vals = src_obj._vals;
      //
      src_obj._size = tmp._size;
      src_obj._refs = tmp._refs;
      src_obj._keys = tmp._keys;
      src_obj._vals = tmp._vals;
      //
      tmp._size = 0;
    }
    //
    HashArray & operator=(const HashArray & src_obj)
    {
      if (this == &src_obj)
        return *this;

      destroy();
      allocate(src_obj._size);

      memcpy(_refs, src_obj._refs, sizeof(uint16_t)*_size);
      memcpy(_keys, src_obj._keys, sizeof(_KeyArg)*_size);
      for (size_type i = 0; i < src_obj._size; ++i) {
        if (_refs[i] & 0x8000)
          _vals[i] = src_obj._vals[i];
      }
      return *this;
    }
  };

#if defined(INTHASH_USAGE_CHECKING) || defined(INTHASH_USAGE_DEBUG)
  static smsc::core::synchronization::Mutex _idMutex;
  const char* _id;
#endif
  HashArray   _hArr;
  size_type   _valCount;  //number of assigned values
  size_type   _collCount; //total number of collisions occured for inserted elements.

protected:
  bool checkCollisions(uint16_t next_attempt) const
  {
    return  !next_attempt || (_collCount < (_valCount >> _IHKeyTraitsArg::_TGT_COLL_PERCENTAGE_PWR));
  }

  //Searches for value associated with given key.
  //Returns true if value is found, false - otherwise.
  bool findVal(_KeyArg use_key, size_type & val_idx) const
  {
    if (!_hArr.size() || !_valCount)
      return false;

    uint16_t attempt = 0;
    do {
      if (attempt >= _IHKeyTraitsArg::_MAX_COLL_NUM)
        return false;
      val_idx = _hArr.Index(use_key, attempt++);
      if (!_hArr.hasRefsAt(val_idx))
        return false;
    } while (!_hArr.isValueAt(val_idx) || !(_hArr.isKeyAt(val_idx, use_key)));

#ifdef INTHASH_USAGE_DEBUG
    { smsc::core::synchronization::MutexGuard mg(_idMutex);
      fprintf(stderr,"IntHash_T(%s)::found at idx=%u, attempt=%u, size=%u, _valCount=%u\n",
              _id, val_idx, (unsigned)attempt-1, _hArr.size(), _valCount);
    }
#endif
    return true;
  }

  //Searches for value associated with given key,
  //marks it as deleted. Additionally composes
  //intermediate array of traversed nodes and
  //adjusts its  refCounts accordingly.
  //Returns true if value is found, false - otherwise.
  bool unmarkVal(_KeyArg use_key, size_type & val_idx)
  {
    if (!_hArr.size())
      return false;

    size_type refList[_IHKeyTraitsArg::_MAX_COLL_NUM + 1];
    uint16_t  attempt = 0;
    do {
      if (attempt >= _IHKeyTraitsArg::_MAX_COLL_NUM)
        return false;
      val_idx = _hArr.Index(use_key, attempt);
      if (!_hArr.hasRefsAt(val_idx))
        return false;
      refList[attempt++] = val_idx;
    } while (!_hArr.isValueAt(val_idx) || !(_hArr.isKeyAt(val_idx, use_key)));

    --attempt;
#ifdef INTHASH_USAGE_DEBUG
    { smsc::core::synchronization::MutexGuard mg(_idMutex);
      fprintf(stderr,"IntHash_T(%s)::unmark at idx=%u, attempt=%u, size=%u, _valCount=%u\n",
              _id, val_idx, (unsigned)attempt, _hArr.size(), _valCount);
    }
#endif
    _hArr.rlseValueAt(val_idx);
    for (uint16_t i = 0; i <= attempt; ++i) {
#ifdef INTHASH_USAGE_CHECKING
      if (!_hArr.hasRefsAt(refList[i])) {
        smsc::core::synchronization::MutexGuard mg(_idMutex);
        fprintf(stderr,"IntHash_T(%s)::unRefAt(%u) - no refs!\n", _id, refList[i]);
      }
#endif
      _hArr.unRefAt(refList[i]);
    }
    --_valCount;
    _collCount -= attempt;
    return true;
  }

  //calculates a new size as a multiple of original size
  //Returns: new_sz >= req_sz
  size_type calcSize(size_type req_sz, size_type org_sz)
  {
    size_type factor = req_sz/org_sz + (req_sz % org_sz  ? 1 : 0);
    return org_sz > org_sz*factor ? HashArray::_maxSize : org_sz*factor;
  }

  //Throws if number of collisions exceeds allowed limit at maximum hash array size.
  void Rehash(size_type tgt_size) /*throw(std::runtime_error)*/
  {
    bool      nextRehash;
    size_type orgSz = !_hArr.size() ? (_IHKeyTraitsArg::_MAX_COLL_NUM>>1) : _hArr.size();

    do {
      HashArray newArr;
      newArr.allocate(tgt_size);
      nextRehash = false;
      size_type newCollNum = 0;

      for (size_type i = 0; i < _hArr.size(); ++i) {
        if (_hArr.isValueAt(i)) {
          //Note: don't use Insert() in order to avoid recursive Rehash calls!
          uint16_t  attNum = 0;
          do {
            size_type idx = newArr.Index(_hArr.keyAt(i), attNum);
            if (!newArr.isValueAt(idx)) {
              if (newArr.addValueAt(idx, _hArr.keyAt(i), _hArr.valueAt(i))) {
#ifdef INTHASH_USAGE_CHECKING
                if (attNum)
#endif
#if defined(INTHASH_USAGE_DEBUG) || defined(INTHASH_USAGE_CHECKING)
                { smsc::core::synchronization::MutexGuard mg(_idMutex);
                  fprintf(stderr,"IntHash_T(%s)::Rehashed to idx=%u, attempt=%u, newSize=%u, _valCount=%u\n",
                          _id, idx, (unsigned)attNum, newArr.size(), _valCount);
                }
#endif
                newCollNum += attNum;
                break;
              }
              nextRehash = true;
            } else
              nextRehash = !newArr.addRefAt(idx);
          } while (!nextRehash && (++attNum < _IHKeyTraitsArg::_MAX_COLL_NUM));

          if (nextRehash || (attNum >= _IHKeyTraitsArg::_MAX_COLL_NUM)) {
            if (tgt_size == HashArray::_maxSize)
              throw std::runtime_error("IntHash_T::Rehash - max collisions occured at max size");
            //recalc new tgt_size
            tgt_size = calcSize(orgSz, tgt_size + (tgt_size % orgSz ? 0 : 1));
            nextRehash = true;
            break;
          }
        }
      }
      if (!nextRehash) {
#ifdef INTHASH_USAGE_CHECKING
        { smsc::core::synchronization::MutexGuard mg(_idMutex);
          fprintf(stderr,"IntHash_T(%s)::Rehashed - orgSize=%u, newSize=%u, _valCount=%u, _collCount=%u\n",
                _id, _hArr.size(), newArr.size(), _valCount, newCollNum);
        }
#endif
        _hArr.swap(newArr);
        _collCount = newCollNum;
      }
    } while (nextRehash);
  }
}; //IntHash_T

#if defined(INTHASH_USAGE_CHECKING) || defined(INTHASH_USAGE_DEBUG)
template <
  class     _TArg
, typename  _KeyArg
, class     _IHKeyTraitsArg
>
smsc::core::synchronization::Mutex IntHash_T< _TArg, _KeyArg, _IHKeyTraitsArg>::_idMutex;
#endif

}//buffers
}//core
}//smsc

#endif /* __CORE_BUFFERS_INTHASH_TEMPLATE */

