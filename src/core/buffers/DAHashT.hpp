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
, uint8_t   _CollMaxNumArg        //NOTE: must belong to range [1..159], target
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

  static const uint8_t k_maxRehashAtt = (sizeof(size_type)<<3) - 1;
  //
  //Target overall collisions percentage: = 100/(2**PWR)
  static const uint8_t  _TGT_COLL_PERCENTAGE_PWR = _CollPercentagePwrArg;
  //
  static const uint8_t  _MAX_COLL_NUM = (_CollMaxNumArg > 159) ? 159 : _CollMaxNumArg;
  //
  static const uint16_t k_primeNum[160];

  explicit DAHash_T(size_type num_to_reserve = 0) : _valCount(0), _collCount(0)
#if defined(INTHASH_USAGE_CHECKING) || defined(INTHASH_USAGE_DEBUG)
    , _id("")
#endif  /* INTHASH_USAGE_* */
  {
    SetSize(num_to_reserve);
  }
  //
  ~DAHash_T()
  {
    Empty();
  }

#if defined(INTHASH_USAGE_CHECKING) || defined(INTHASH_USAGE_DEBUG)
  void debugInit(const char * log_id)
  {
    _id = log_id;
  }
#endif /* INTHASH_USAGE_* */

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
      doRehash(num_to_reserve < _MAX_COLL_NUM ? _MAX_COLL_NUM : num_to_reserve);
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
      doRehash(_MAX_COLL_NUM);
    else if (_valCount == _hArr.size())
      doRehash(0);

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
        doRehash(0); //throws
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
        doRehash(0); //throws
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
  DAHash_T(const DAHash_T& src);
  DAHash_T& operator=(const DAHash_T& src_obj);

  friend class iterator;

  HashTable_s  _hArr;
  size_type   _valCount;  //number of assigned values
  size_type   _collCount; //total number of collisions occured for inserted elements.

#if defined(INTHASH_USAGE_CHECKING) || defined(INTHASH_USAGE_DEBUG)
  static smsc::core::synchronization::Mutex _idMutex;
  const char* _id;
#endif

protected:
  //Note: throws on corrupted key
  static size_type calcIndex(uint32_t tbl_size, const _KeyArg & use_key, uint16_t num_attempt)
    /*throw(std::exception)*/
  {
    return ((DAHashFunc_T::hashKey(use_key, num_attempt) % tbl_size)
                                  + (k_primeNum[num_attempt] % tbl_size)) % tbl_size;
  }

  //Returns true if restrictions on number of collisions aren't violated.
  bool checkCollisions(uint16_t next_attempt) const
  {
    return (!_collCount || ((_collCount + next_attempt) < (_hArr.size() >> _TGT_COLL_PERCENTAGE_PWR)));
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
  size_type calcNextSize(uint8_t rehash_att) const
  {
    size_type orgSz = !_hArr.size() ? ((_MAX_COLL_NUM+1) >> 1) : _hArr.size();
    size_type nextSz = orgSz << (1 + rehash_att);
    return (nextSz <= orgSz) ? HashTable_s::_maxSize : nextSz;
  }

  //Throws if number of collisions exceeds allowed limit at maximum hash array
  //size or maximum number of rehash attempts is reached.
  void doRehash(size_type tgt_size = 0) /*throw(std::runtime_error)*/
  {
    bool      nextRehash = false;
    uint8_t   rehashAtt = 0;
    size_type nextSz = tgt_size ? tgt_size : calcNextSize(0);

    do {
      size_type   newCollNum = 0;
      HashTable_s newArr;

      newArr.allocate(nextSz);
      nextRehash = false;

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
            nextRehash = true;
            do {
              if (nextSz == HashTable_s::_maxSize)
                throw std::runtime_error("DAHash_T::Rehash - max collisions occured at max size");
              nextSz = calcNextSize(++rehashAtt);
            } while (nextSz <= tgt_size);
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
    } while (nextRehash && (rehashAtt < k_maxRehashAtt));

    if (nextRehash)
      throw std::runtime_error("DAHash_T::Rehash - failed at max attempts");
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
const uint8_t DAHash_T<_KeyArg, _TArg, _SlotTArg, _CollMaxNumArg, _CollPercentagePwrArg>::k_maxRehashAtt;

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

template <
  class     _KeyArg
, class     _TArg
, template <typename _K, class _T>
  class     _SlotTArg
, uint8_t   _CollMaxNumArg
, uint8_t   _CollPercentagePwrArg
>
const uint16_t DAHash_T<_KeyArg, _TArg, _SlotTArg, _CollMaxNumArg, _CollPercentagePwrArg>::k_primeNum[160] = {
    0,   2,   3,   5,   7,  11,  13,  17,  19,  23,  29,  31,  37,  41,  43,  47,
   53,  59,  61,  67,  71,  73,  79,  83,  89,  97, 101, 103, 107, 109, 113, 127,
  131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211,
  223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307,
  311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401,
  409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499,
  503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607,
  613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709,
  719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823,
  827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937
};

}//buffers
}//core
}//smsc

#endif /* __CORE_BUFFERS_DIRECT_ACCESS_HASH_TEMPLATE */

