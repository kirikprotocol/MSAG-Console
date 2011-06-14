/* ************************************************************************** *
 * Template of generic direct access hash table. 
 * Note: hash table size type is uint32_t. 
 * ************************************************************************** */
#ifndef __CORE_BUFFERS_DIRECT_ACCESS_HASH_TEMPLATE
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __CORE_BUFFERS_DIRECT_ACCESS_HASH_TEMPLATE

#include "core/buffers/DAHashFunc.hpp"
#include "core/buffers/DAHashDefs.hpp"

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
  class  _KeyArg             //required default & copying constructors, operator=()
, class  _TArg               //required default & copying constructors, operator=()
, template <typename _K, class _T>
  class     _SlotTArg       //required DAHashSlot_T<_KeyArg, _TArg> interface
                = DAHashSlot_T //assume complex objects by default
, uint8_t   _CollMaxNumArg
                = 16
, uint8_t   _CollPercentagePwrArg //NOTE: must belong to range [1..6], target
                = 3               //percentage is calculated as := 100/(2**PWR)
                                  //is [50% 25% 12.5% 6.25% 3.125% 1.56%]
>
class DAHash_T {
protected:
  typedef DAHashTable_T<_KeyArg, _TArg, _SlotTArg>  HashTable_s;

public:
  typedef uint32_t                    size_type;
  typedef HKeyValue_T<_KeyArg, _TArg> value_type;

  static const uint8_t  _MAX_COLL_NUM = _CollMaxNumArg;
  //Target overall collisions percentage: = 100/(2**PWR)
  static const uint8_t  _TGT_COLL_PERCENTAGE_PWR = _CollPercentagePwrArg;


#if defined(INTHASH_USAGE_CHECKING) || defined(INTHASH_USAGE_DEBUG)
  explicit DAHash_T(const char * use_id) : _id(use_id ? use_id : ""),
#else
  DAHash_T() :
#endif
    _valCount(0), _collCount(0)
  { }
  //
  DAHash_T(const DAHash_T& src) :
#if defined(INTHASH_USAGE_CHECKING) || defined(INTHASH_USAGE_DEBUG)
    _id(src._id),
#endif
    _valCount(0), _collCount(0)
  {
    *this = src;
  }

#if defined(INTHASH_USAGE_CHECKING) || defined(INTHASH_USAGE_DEBUG)
  explicit DAHash_T(size_type num_to_reserve, const char* id ) : _id(id),
#else
  explicit DAHash_T(size_type num_to_reserve) :
#endif
    _valCount(0), _collCount(0)
  {
    SetSize(num_to_reserve);
  }
  //
  ~DAHash_T()
  {
    Empty();
  }

  DAHash_T& operator=(const DAHash_T& src_obj)
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
      Rehash(num_to_reserve < _MAX_COLL_NUM ? _MAX_COLL_NUM : num_to_reserve);
  }

  const _TArg & Get(const _KeyArg & use_key) const /*throw(std::exception)*/
  {
    size_type valIdx;
    if(!findVal(use_key, valIdx))
      throw std::runtime_error((!_hArr.size() || !_valCount) ? "DAHash_T::Get - empty inthash"
                                                 : "DAHash_T::Get - item not found");
    return _hArr.at(valIdx)._value;
  }

  _TArg & Get(const _KeyArg & use_key) /*throw(std::exception)*/
  {
    size_type valIdx;
    if(!findVal(use_key, valIdx))
      throw std::runtime_error((!_hArr.size() || !_valCount) ? "DAHash_T::Get - empty inthash"
                                                 : "DAHash_T::Get - item not found");
    return _hArr.at(valIdx)._value;
  }

  _TArg * GetPtr(const _KeyArg & use_key) const
  {
    size_type valIdx;
    return findVal(use_key, valIdx) ? _hArr.getValueAt(valIdx) : NULL;
  }

  bool Get(const _KeyArg & use_key, _TArg & out_val) const
  {
    size_type valIdx;
    if(!findVal(use_key, valIdx))
      return false;
    out_val = _hArr.at(valIdx)._value;
    return true;
  }

  bool Exist(const _KeyArg & use_key) const
  {
    size_type valIdx;
    return findVal(use_key, valIdx);
  }

  bool Delete(const _KeyArg & use_key)
  {
    size_type valIdx;
    return unmarkVal(use_key, valIdx, NULL);
  }

  bool Pop(const _KeyArg & use_key, _TArg & out_value)
  {
    size_type valIdx;
    return unmarkVal(use_key, valIdx, &out_value);
  }

  //Note: it's not recommended to insert new elements to hash table while
  //accessing its elements via iterators. Iterator may become invalid due
  //to rehashing enforced by insertion.
  class iterator {
  protected:
    size_type   _hIdx;
    DAHash_T *  _pHash;

    friend class DAHash_T;
    iterator(DAHash_T & use_owner, size_type use_idx)
      : _hIdx(use_idx), _pHash(&use_owner)
    {
      searchNext();
    }

    void searchNext(void)
    {
      while ((_hIdx < _pHash->_hArr.size()) && !_pHash->_hArr.hasValueAt(_hIdx))
        ++_hIdx;
    }

  public:
    iterator() : _hIdx(0), _pHash(NULL)
    { }
    iterator(const iterator & src)
      : _hIdx(src._hIdx), _pHash(src._pHash)
    { }
    //
    ~iterator()
    { }

    iterator & operator=(const iterator & src)
    {
      _hIdx = src._hIdx;
      _pHash = src._pHash;
      return *this;
    }
    //
    void seekEnd(void) { _hIdx = _pHash->_hArr.size(); }
    //
    bool isEnd(void) const { return (_hIdx == _pHash->_hArr.size()); }

    value_type * get(void)
    {
      return ((_hIdx < _pHash->_hArr.size()) && _pHash->_hArr.hasValueAt(_hIdx)) ?
            &_pHash->_hArr.at(_hIdx) : NULL;
    }
    //
    const value_type * get(void) const
    {
      return ((_hIdx < _pHash->_hArr.size()) && _pHash->_hArr.hasValueAt(_hIdx)) ?
            &_pHash->_hArr.at(_hIdx) : NULL;
    }

    //
    const value_type * operator->(void) const { return get(); }
    //
    value_type * operator->(void) { return get(); }

    const value_type & operator*(void) const { return *get(); }
    value_type & operator*(void) { return *get(); }

    bool operator==(const iterator & cmp_obj) const
    {
      return (_pHash == cmp_obj._pHash) && (_hIdx == cmp_obj._hIdx);
    }
    bool operator!=(const iterator & cmp_obj) const
    {
      return !(*this == cmp_obj);
    }

    iterator & operator++() //preincrement
    {
      if (!_pHash->_valCount)
        _hIdx = _pHash->_hArr.size();
      else {
        if (_hIdx >= _pHash->_hArr.size())
          _hIdx = 0;
        else 
          ++_hIdx;
        searchNext();
      }
      return *this;
    }
    //
    iterator operator++(int) //postincrement
    {
      iterator  tmp = *this;
      ++(*this);
      return tmp;
    }
  };

  class const_iterator {
  protected:
    size_type         _hIdx;
    const DAHash_T * _pHash;

    friend class DAHash_T;
    const_iterator(const DAHash_T & use_owner, size_type use_idx)
      : _hIdx(use_idx), _pHash(&use_owner)
    {
      searchNext();
    }

    void searchNext(void)
    {
      while ((_hIdx < _pHash->_hArr.size()) && !_pHash->_hArr.hasValueAt(_hIdx))
        ++_hIdx;
    }

  public:
    const_iterator() : _hIdx(0), _pHash(NULL)
    { }
    const_iterator(const const_iterator & src)
      : _hIdx(src._hIdx), _pHash(src._pHash)
    { }
    const_iterator(const iterator & src)
      : _hIdx(src._hIdx), _pHash(src._pHash)
    { }
    //
    ~const_iterator()
    { }

    const_iterator & operator=(const const_iterator & src)
    {
      _hIdx = src._hIdx;
      _pHash = src._pHash;
      return *this;
    }
    //
    void seekEnd(void) { _hIdx = _pHash->_hArr.size(); }
    //
    bool isEnd(void) const { return (_hIdx == _pHash->_hArr.size()); }

    //
    const value_type * get(void) const
    {
      return ((_hIdx < _pHash->_hArr.size()) && _pHash->_hArr.hasValueAt(_hIdx)) ?
            &_pHash->_hArr.at(_hIdx) : NULL;
    }
    //
    const value_type * operator->(void) const { return get(); }
    //
    const value_type & operator*(void) const { return *get(); }

    bool operator==(const const_iterator & cmp_obj) const
    {
      return (_pHash == cmp_obj._pHash) && (_hIdx == cmp_obj._hIdx);
    }
    bool operator!=(const const_iterator & cmp_obj) const
    {
      return !(*this == cmp_obj);
    }

    const_iterator & operator++() //preincrement
    {
      if (!_pHash->_valCount)
        _hIdx = _pHash->_hArr.size();
      else {
        if (_hIdx >= _pHash->_hArr.size())
          _hIdx = 0;
        else 
          ++_hIdx;
        searchNext();
      }
      return *this;
    }
    //
    const_iterator operator++(int) //postincrement
    {
      const_iterator  tmp = *this;
      ++(*this);
      return tmp;
    }
  };

  iterator begin(void)
  {
    return iterator(*this, 0);
  }
  iterator end(void)
  {
    return iterator(*this, _hArr.size());
  }
  const_iterator begin(void) const
  {
    return iterator(*this, 0);
  }
  const_iterator end(void) const
  {
    return iterator(*this, _hArr.size());
  }

  //
  void erase(const iterator & use_it)
  {
    if (use_it.get())
      Delete(use_it->_key);
  }

  //Throws if collisions limit is reached at max possible table size.
  iterator Insert(const _KeyArg & key, const _TArg & value)  /*throw(std::runtime_error)*/
  {
    if (!_hArr.size())
      Rehash(_MAX_COLL_NUM);
    else if (_valCount == _hArr.size()) {
      size_type newSz = _hArr.size() << 1;
      Rehash(newSz < _hArr.size() ? HashTable_s::_maxSize : newSz); //throws
    }

    uint16_t  attempt = 0;
    size_type idx;
    bool      newVal = false;

    for (;;) {
      idx = calcIndex(_hArr.size(), key, attempt); //throws
      if (!_hArr.hasValueAt(idx)) {  //insert a new value
        if (checkCollisions(attempt) && (newVal = _hArr.addValueAt(idx, key, value))) {
          _collCount += attempt;
#ifdef INTHASH_USAGE_DEBUG
          { smsc::core::synchronization::MutexGuard mg(_idMutex);
            fprintf(stderr,"DAHash_T(%s)::Inserting at idx=%u, attempt=%u, size=%u, _valCount=%u, _collCount=%u\n",
                  _id, idx, (unsigned)attempt, _hArr.size(), _valCount, _collCount);
          }
#endif
          break;
        }
        //else rehash is required
#if defined(INTHASH_USAGE_CHECKING) || defined(INTHASH_USAGE_DEBUG)
        { smsc::core::synchronization::MutexGuard mg(_idMutex);
          fprintf(stderr,"DAHash_T(%s)::Insert need rehashing at attempt=%u, orgSize=%u, _valCount=%u, _collCount=%u\n",
                  _id, (unsigned)attempt, _hArr.size(), _valCount, _collCount);
        }
#endif
        size_type newSz = _hArr.size() << 1;
        Rehash(newSz < _hArr.size() ? HashTable_s::_maxSize : newSz); //throws
        attempt = 0;
        continue;
      }
      if (_hArr.isKeyAt(idx, key)) {  //overwrite an existing value
#ifdef INTHASH_USAGE_DEBUG
        { smsc::core::synchronization::MutexGuard mg(_idMutex);
          fprintf(stderr,"DAHash_T(%s)::Overwritting at idx=%u, attempt=%u, size=%u, _valCount=%u, _collCount=%u\n",
                  _id, idx, (unsigned)attempt, _hArr.size(), _valCount, _collCount);
        }
#endif
        _hArr.setValueAt(idx, value);
        break;
      }
      if (!_hArr.addRefAt(idx) || (++attempt >= _MAX_COLL_NUM)) {
#if defined(INTHASH_USAGE_CHECKING) || defined(INTHASH_USAGE_DEBUG)
        { smsc::core::synchronization::MutexGuard mg(_idMutex);
          fprintf(stderr,"DAHash_T(%s)::Insert need rehashing at attempt=%u, orgSize=%u, _valCount=%u, _collCount=%u\n",
              _id, (unsigned)attempt, _hArr.size(), _valCount, _collCount);
        }
#endif
        size_type newSz = _hArr.size() << 1;
        Rehash(newSz < _hArr.size() ? HashTable_s::_maxSize : newSz); //throws
        attempt = 0;
      }
    }
#if defined(INTHASH_USAGE_CHECKING) || defined(INTHASH_USAGE_DEBUG)
    if (attempt) {
      smsc::core::synchronization::MutexGuard mg(_idMutex);
      fprintf(stderr,"DAHash_T(%s)::Inserted at attempt=%u, orgSize=%u, _valCount=%u, _collCount=%u\n",
              _id, (unsigned)attempt, _hArr.size(), _valCount, _collCount);
    }
#endif
    if (newVal)
      ++_valCount;
    return iterator(*this, idx);
  }

  //Returns end() if no value associated with given key is found.
  iterator getIterator(const _KeyArg & use_key)
  {
    size_type atIdx = _hArr.size();
    findVal(use_key, atIdx);
    return iterator(*this, atIdx);
  }
  const_iterator getIterator(const _KeyArg & use_key) const
  {
    size_type atIdx = _hArr.size();
    findVal(use_key, atIdx);
    return iterator(*this, atIdx);
  }

  //AdHoc method: returns a iterator associated with value stored in hash table.
  //Note: value reference MUST BE previously obtained by GetPtr() call.
  //Returns end() if given value doesn't contained in hash.
  const_iterator getIterator(const _TArg & hashed_val) const
  {
    size_type atIdx = _hArr.size();
    _hArr.detectIdx(hashed_val, atIdx);
    return const_iterator(*this, atIdx);
  }

  //AdHoc method: returns a key associated with value stored in hash table.
  //Note: value reference MUST BE previously obtained by GetPtr() call.
  //Returns NULL if given value doesn't contained in hash.
  const _KeyArg * GetKey(const _TArg & hashed_val) const
  {
    size_type atIdx = _hArr.size();
    return _hArr.detectIdx(hashed_val, atIdx) ? &(_hArr.at(atIdx)._key) : NULL;
  }

private:
  friend class iterator;

#if defined(INTHASH_USAGE_CHECKING) || defined(INTHASH_USAGE_DEBUG)
  static smsc::core::synchronization::Mutex _idMutex;
  const char* _id;
#endif
  HashTable_s  _hArr;
  size_type   _valCount;  //number of assigned values
  size_type   _collCount; //total number of collisions occured for inserted elements.

protected:
  //Note: throws on corrupted key
  static size_type calcIndex(uint32_t tbl_size, const _KeyArg & use_key, uint16_t num_attempt)
    /*throw(std::exception)*/
  {
    return DAHashFunc_T::hashKey(use_key, num_attempt) % tbl_size;
  }

  bool checkCollisions(uint16_t next_attempt) const
  {
    return  !next_attempt || (_collCount < (_valCount >> _TGT_COLL_PERCENTAGE_PWR));
  }

  //Searches for value associated with given key.
  //Returns true if value is found, false - otherwise.
  bool findVal(const _KeyArg & use_key, size_type & val_idx) const /*throw(std::exception)*/
  {
    if (!_hArr.size() || !_valCount)
      return false;

    uint16_t attempt = 0;
    do {
      if (attempt >= _MAX_COLL_NUM)
        return false;
      val_idx = calcIndex(_hArr.size(), use_key, attempt++); //throws
      if (!_hArr.hasRefsAt(val_idx))
        return false;
    } while (!_hArr.hasValueAt(val_idx) || !(_hArr.isKeyAt(val_idx, use_key)));

#ifdef INTHASH_USAGE_DEBUG
    { smsc::core::synchronization::MutexGuard mg(_idMutex);
      fprintf(stderr,"DAHash_T(%s)::found at idx=%u, attempt=%u, size=%u, _valCount=%u\n",
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
  bool unmarkVal(const _KeyArg & use_key, size_type & val_idx, _TArg * p_value = NULL) /*throw(std::exception)*/
  {
    if (!_hArr.size())
      return false;

    size_type refList[_MAX_COLL_NUM + 1];
    uint16_t  attempt = 0;
    do {
      if (attempt >= _MAX_COLL_NUM)
        return false;
      val_idx = calcIndex(_hArr.size(), use_key, attempt); //throws
      if (!_hArr.hasRefsAt(val_idx))
        return false;
      refList[attempt++] = val_idx;
    } while (!_hArr.hasValueAt(val_idx) || !(_hArr.isKeyAt(val_idx, use_key)));

    --attempt;
#ifdef INTHASH_USAGE_DEBUG
    { smsc::core::synchronization::MutexGuard mg(_idMutex);
      fprintf(stderr,"DAHash_T(%s)::unmark at idx=%u, attempt=%u, size=%u, _valCount=%u\n",
              _id, val_idx, (unsigned)attempt, _hArr.size(), _valCount);
    }
#endif
    if (p_value)
      *p_value = _hArr.at(val_idx)._value;
    _hArr.delValueAt(val_idx);

    for (uint16_t i = 0; i <= attempt; ++i) {
#ifdef INTHASH_USAGE_CHECKING
      if (!_hArr.hasRefsAt(refList[i])) {
        smsc::core::synchronization::MutexGuard mg(_idMutex);
        fprintf(stderr,"DAHash_T(%s)::unRefAt(%u) - no refs!\n", _id, refList[i]);
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
    return org_sz > org_sz*factor ? HashTable_s::_maxSize : org_sz*factor;
  }

  //Throws if number of collisions exceeds allowed limit at maximum hash array size.
  void Rehash(size_type tgt_size) /*throw(std::runtime_error)*/
  {
    bool      nextRehash;
    size_type orgSz = !_hArr.size() ? (_MAX_COLL_NUM>>1) : _hArr.size();

    do {
      HashTable_s newArr;
      newArr.allocate(tgt_size);
      nextRehash = false;
      size_type newCollNum = 0;

      for (size_type i = 0; i < _hArr.size(); ++i) {
        if (_hArr.hasValueAt(i)) {
          //Note: don't use Insert() in order to avoid recursive Rehash calls!
          uint16_t  attNum = 0;
          do {
            size_type idx = calcIndex(newArr.size(), _hArr.at(i).key(), attNum); //throws
            if (!newArr.hasValueAt(idx)) {
              if (newArr.addValueAt(idx, _hArr.at(i).key(), _hArr.at(i)._value)) {
#ifdef INTHASH_USAGE_CHECKING
                if (attNum)
#endif
#if defined(INTHASH_USAGE_DEBUG) || defined(INTHASH_USAGE_CHECKING)
                { smsc::core::synchronization::MutexGuard mg(_idMutex);
                  fprintf(stderr,"DAHash_T(%s)::Rehashed to idx=%u, attempt=%u, newSize=%u, _valCount=%u\n",
                          _id, idx, (unsigned)attNum, newArr.size(), _valCount);
                }
#endif
                newCollNum += attNum;
                break;
              }
              nextRehash = true;
            } else
              nextRehash = !newArr.addRefAt(idx);
          } while (!nextRehash && (++attNum < _MAX_COLL_NUM));

          if (nextRehash || (attNum >= _MAX_COLL_NUM)) {
            if (tgt_size == HashTable_s::_maxSize)
              throw std::runtime_error("DAHash_T::Rehash - max collisions occured at max size");
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
          fprintf(stderr,"DAHash_T(%s)::Rehashed - orgSize=%u, newSize=%u, _valCount=%u, _collCount=%u\n",
                _id, _hArr.size(), newArr.size(), _valCount, newCollNum);
        }
#endif
        _hArr.swap(newArr);
        _collCount = newCollNum;
      }
    } while (nextRehash);
  }
}; //DAHash_T

#if defined(INTHASH_USAGE_CHECKING) || defined(INTHASH_USAGE_DEBUG)
template <
  class     _KeyArg
, class     _TArg
, template <typename _K, class _T>
  class     _SlotTArg
, uint8_t   _CollMaxNumArg
, uint8_t   _CollPercentagePwrArg
>
smsc::core::synchronization::Mutex
  DAHash_T<_KeyArg, _TArg, _SlotTArg, _CollMaxNumArg, _CollPercentagePwrArg>::_idMutex;
#endif

template <
  class     _KeyArg
, class     _TArg
, template <typename _K, class _T>
  class     _SlotTArg
, uint8_t   _CollMaxNumArg
, uint8_t   _CollPercentagePwrArg
>
const uint8_t DAHash_T<_KeyArg, _TArg, _SlotTArg, _CollMaxNumArg, _CollPercentagePwrArg>::_MAX_COLL_NUM;

template <
  class     _KeyArg
, class     _TArg
, template <typename _K, class _T>
  class     _SlotTArg
, uint8_t   _CollMaxNumArg
, uint8_t   _CollPercentagePwrArg
>
const uint8_t DAHash_T<_KeyArg, _TArg, _SlotTArg, _CollMaxNumArg, _CollPercentagePwrArg>::_TGT_COLL_PERCENTAGE_PWR;

}//buffers
}//core
}//smsc

#endif /* __CORE_BUFFERS_DIRECT_ACCESS_HASH_TEMPLATE */

