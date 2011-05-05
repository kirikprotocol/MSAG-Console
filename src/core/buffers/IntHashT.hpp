/* ************************************************************************** *
 * Template of generic direct access hash table of elements addressed by 
 * integer key (signed or unsigned).
 * ************************************************************************** */
#ifndef __CORE_BUFFERS_INTHASH_TEMPLATE
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __CORE_BUFFERS_INTHASH_TEMPLATE

#include "core/buffers/IntHashTraits.hpp"

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
  class     _TArg               //required default & copying constructors, operator=()
, typename  _KeyArg             //must be an integer type (signed or unsigned)
, template <typename _K, class _T>
  class     _SlotTArg           //required IHashSlot_T<> interface
                    = IHashSlotPOD_T
, class     _IHKeyTraitsArg      //required IntHashKeyTraitsDflt_T<> interface
                    = IntHashKeyTraitsDflt_T<_KeyArg, 16, 3>
>
class IntHash_T {
public:
  typedef uint32_t size_type; //must be an unsigned integer type!
  typedef IKeyValue_T<_KeyArg, _TArg> value_type;
  typedef IHashTable_T<_KeyArg, _TArg, _SlotTArg, _IHKeyTraitsArg>  IHashTable;

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

  const _TArg & Get(_KeyArg key) const /*throw(std::exception)*/
  {
    size_type valIdx;
    if(!findVal(key, valIdx))
      throw std::runtime_error((!_hArr.size() || !_valCount) ? "IntHash_T::Get - empty inthash"
                                                 : "IntHash_T::Get - item not found");
    return _hArr.at(valIdx)._value;
  }

  _TArg & Get(_KeyArg key) /*throw(std::exception)*/
  {
    size_type valIdx;
    if(!findVal(key, valIdx))
      throw std::runtime_error((!_hArr.size() || !_valCount) ? "IntHash_T::Get - empty inthash"
                                                 : "IntHash_T::Get - item not found");
    return _hArr.at(valIdx)._value;
  }

  _TArg * GetPtr(_KeyArg key) const
  {
    size_type valIdx;
    return findVal(key, valIdx) ? _hArr.getValueAt(valIdx) : NULL;
  }

  bool Get(_KeyArg key, _TArg & value) const
  {
    size_type valIdx;
    if(!findVal(key, valIdx))
      return false;
    value = _hArr.at(valIdx)._value;
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
    return unmarkVal(key, valIdx, NULL);
  }

  bool Pop(_KeyArg key, _TArg & out_value)
  {
    size_type valIdx;
    return unmarkVal(key, valIdx, &out_value);
  }

  //Note: it's not recommended to insert new elements to hash table while
  //accessing its elements via iterators. Iterator may become invalid due
  //to rehashing enforced by insertion.
  class iterator {
  protected:
    size_type         _hIdx;
    const IntHash_T * _pHash;

    friend class IntHash_T;
    iterator(const IntHash_T & use_owner, size_type use_idx)
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

/*    value_type * get(void)
    {
      return ((_hIdx < _pHash->_hArr.size()) && _pHash->_hArr.hasValueAt(_hIdx)) ?
            &_pHash->_hArr.at(_hIdx) : NULL;
    }*/
    //
    //const 
    value_type * get(void) const
    {
      return ((_hIdx < _pHash->_hArr.size()) && _pHash->_hArr.hasValueAt(_hIdx)) ?
            &_pHash->_hArr.at(_hIdx) : NULL;
    }

    //
    //const value_type * operator->(void) const { return get(); }
    //
    value_type * operator->(void) const { return get(); }

    value_type & operator*(void) const { return *get(); }

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

  iterator begin(void) const
  {
    return iterator(*this, 0);
  }
  iterator end(void) const
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
  iterator Insert(_KeyArg key, const _TArg & value)  /*throw(std::runtime_error)*/
  {
    if (!_hArr.size())
      Rehash(_IHKeyTraitsArg::_MAX_COLL_NUM);
    else if (_valCount == _hArr.size()) {
      size_type newSz = _hArr.size() << 1;
      Rehash(newSz < _hArr.size() ? IHashTable::_maxSize : newSz); //throws
    }

    uint16_t  attempt = 0;
    size_type idx;
    bool      newVal = false;

    for (;;) {
      idx = _hArr.Index(key, attempt);
      if (!_hArr.hasValueAt(idx)) {  //insert a new value
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
        Rehash(newSz < _hArr.size() ? IHashTable::_maxSize : newSz); //throws
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
        Rehash(newSz < _hArr.size() ? IHashTable::_maxSize : newSz); //throws
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
    return iterator(*this, idx);
  }

  //Returns end() if no value associated with given key is found.
  iterator getIterator(_KeyArg use_key) const
  {
    size_type atIdx = _hArr.size();
    findVal(use_key, atIdx);
    return iterator(*this, atIdx);
  }

  //AdHoc method: returns a iterator associated with value stored in hash table.
  //Note: value reference MUST BE previously obtained by GetPtr() call.
  //Returns end() if given value doesn't contained in hash.
  iterator getIterator(const _TArg & hashed_val) const
  {
    size_type atIdx = _hArr.size();
    _hArr.detectIdx(hashed_val, atIdx);
    return iterator(*this, atIdx);
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
  IHashTable  _hArr;
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
    } while (!_hArr.hasValueAt(val_idx) || !(_hArr.isKeyAt(val_idx, use_key)));

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
  bool unmarkVal(_KeyArg use_key, size_type & val_idx, _TArg * p_value = NULL)
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
    } while (!_hArr.hasValueAt(val_idx) || !(_hArr.isKeyAt(val_idx, use_key)));

    --attempt;
#ifdef INTHASH_USAGE_DEBUG
    { smsc::core::synchronization::MutexGuard mg(_idMutex);
      fprintf(stderr,"IntHash_T(%s)::unmark at idx=%u, attempt=%u, size=%u, _valCount=%u\n",
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
    return org_sz > org_sz*factor ? IHashTable::_maxSize : org_sz*factor;
  }

  //Throws if number of collisions exceeds allowed limit at maximum hash array size.
  void Rehash(size_type tgt_size) /*throw(std::runtime_error)*/
  {
    bool      nextRehash;
    size_type orgSz = !_hArr.size() ? (_IHKeyTraitsArg::_MAX_COLL_NUM>>1) : _hArr.size();

    do {
      IHashTable newArr;
      newArr.allocate(tgt_size);
      nextRehash = false;
      size_type newCollNum = 0;

      for (size_type i = 0; i < _hArr.size(); ++i) {
        if (_hArr.hasValueAt(i)) {
          //Note: don't use Insert() in order to avoid recursive Rehash calls!
          uint16_t  attNum = 0;
          do {
            size_type idx = newArr.Index(_hArr.at(i)._key, attNum);
            if (!newArr.hasValueAt(idx)) {
              if (newArr.addValueAt(idx, _hArr.at(i)._key, _hArr.at(i)._value)) {
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
            if (tgt_size == IHashTable::_maxSize)
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
, template <typename _K, class _T>
  class     _SlotTArg
, class     _IHKeyTraitsArg
>
smsc::core::synchronization::Mutex IntHash_T< _TArg, _KeyArg, _SlotTArg, _IHKeyTraitsArg>::_idMutex;
#endif

}//buffers
}//core
}//smsc

#endif /* __CORE_BUFFERS_INTHASH_TEMPLATE */

