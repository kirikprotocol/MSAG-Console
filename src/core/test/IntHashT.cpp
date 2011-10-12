#include <stdio.h>
#include <stdlib.h>
#include <map>

//#define INTHASH_USAGE_DEBUG
#define INTHASH_USAGE_CHECKING
//#include "core/buffers/IntHashT.hpp"
#include "core/buffers/DAHashT.hpp"
#include "core/buffers/DAHashFuncInts.hpp"
#include "util/csv/CSValueTraitsInt.hpp"


//#define VAL_INSTANCES_DEBUG
template <
  class _TArg //must be an integer type (signed or unsigned)
>
class TstElement_T {
protected:
  uint32_t  _instId;
  _TArg     _val;

public:
  //Note: instanceId == 0 is reserved!!!
  TstElement_T() : _instId(0), _val(0)
  { }
  explicit TstElement_T(const _TArg & use_val)
    : _instId(1)
  {
    _val = use_val;
#ifdef VAL_INSTANCES_DEBUG
    fprintf(stderr, "TstElement_T(inst=%u): %s constructed\n",  _instId,
            smsc::util::csv::CSValueTraits_T<_TArg>::val2str(_val).c_str());
#endif
  }
  //
  TstElement_T(const TstElement_T & cp_obj)
    : _instId(cp_obj._instId + 1)
  {
    _val = cp_obj._val;
#ifdef VAL_INSTANCES_DEBUG
    fprintf(stderr, "TstElement_T(inst=%u): %s copied\n",  _instId,
            smsc::util::csv::CSValueTraits_T<_TArg>::val2str(_val).c_str());
#endif
  }
  //
  ~TstElement_T()
  {
#ifdef VAL_INSTANCES_DEBUG
    fprintf(stderr, "TstElement_T(inst=%u): %s destroyed\n",  _instId,
            smsc::util::csv::CSValueTraits_T<_TArg>::val2str(_val).c_str());
#endif
  }

  const _TArg & get(void) const { return _val; }

  std::string toStr(void) const
  {
    return smsc::util::csv::CSValueTraits_T<_TArg>::val2str(_val);
  }

  TstElement_T & operator=(const TstElement_T & cmp_obj)
  {
#ifdef VAL_INSTANCES_DEBUG
    fprintf(stderr, "TstElement_T(inst=%u): %s clearing ..\n",  _instId,
            smsc::util::csv::CSValueTraits_T<_TArg>::val2str(_val).c_str());
#endif
    if ((_instId = cmp_obj._instId))
      ++_instId;
    _val = cmp_obj._val;
#ifdef VAL_INSTANCES_DEBUG
    fprintf(stderr, "TstElement_T(inst=%u): %s assigned\n",  _instId,
            smsc::util::csv::CSValueTraits_T<_TArg>::val2str(_val).c_str());
#endif
    return *this;
  }

  bool operator==(const TstElement_T & cmp_obj) const
  {
    return _val == cmp_obj._val;
  }
};


template <
  typename _KeyArg      //must be an integer type (signed or unsigned)
>
struct HashTester_T {
  typedef uint32_t size_type; //DAHash_T<>::size_type

  typedef TstElement_T<_KeyArg>
    CachedValue;
  typedef smsc::core::buffers::DAHash_T<
    _KeyArg, CachedValue, smsc::core::buffers::DAHashSlot_T, 16, 3
  > TstCache_t;

  const char *    _hashId;

  explicit HashTester_T(const char * use_id) : _hashId(use_id)
  { }
  ~HashTester_T()
  { }


  size_type genAttempt(size_type del_range)
  {
    long val = lrand48(); //nrand48(_xi);
    //fprintf(stderr, "\nIntHash_T<%s> lrand48() res: %ld\n", _hashId, val % del_range);
    return (val % del_range);
  }

  bool testHash_rand_del(_KeyArg min_id, _KeyArg max_id, size_type del_range)
  {
    typedef std::multimap<size_type/*att_num*/, _KeyArg>
      DelQueue;

    bool        rval = true;
    DelQueue    delQueue;
    TstCache_t  tCache;

  #if defined(INTHASH_USAGE_CHECKING) || defined(INTHASH_USAGE_DEBUG)
    tCache.debugInit(_hashId);
  #endif
    //test sequential insertion/find, random delete
    fprintf(stderr, "\nIntHash_T<%s>::inserting elements sequentially, deleting randomly.. \n", _hashId);
    size_type attNum = 0;
    _KeyArg   i = min_id;
    do {
      //delete expired elements first
      if (!delQueue.empty()) {
        typename DelQueue::iterator nit = delQueue.begin();
        do {
          typename DelQueue::iterator it = nit++;
          if (it->first != attNum)
            break;
          CachedValue tVal(it->second);
          if (!tCache.Pop(it->second, tVal)) {
            fprintf(stderr, "IntHash_T<%s>::Pop(%s) at attempt(%lu) failed\n", _hashId,
                    tVal.toStr().c_str(), (unsigned long)it->first);
            rval = false;
            break;
          }
          //else fprintf(stderr, "IntHash_T<%s>::Pop(%s) at attempt(%lu) Ok\n", _hashId,
          //          tVal.toStr().c_str(), (unsigned long)it->first);
          delQueue.erase(it);
        } while (nit != delQueue.end());
      }
      if (!rval)
        break;

      CachedValue tVal(i);
      try { tCache.Insert(i, tVal);
      } catch (const std::exception & exc) {
        fprintf(stderr, "IntHash_T<%s>::Insert(%s) exception: %s\n", _hashId,
                tVal.toStr().c_str(), exc.what());
        rval = false;
        break;
      }
      ++attNum;
      CachedValue * pVal = tCache.GetPtr(i);
      if (!pVal) {
        fprintf(stderr, "IntHash_T<%s>::GetPtr(%s) failed\n", _hashId, tVal.toStr().c_str());
        rval = false;
      } else if (!(*pVal == tVal)) {
        fprintf(stderr, "IntHash_T<%s>::GetPtr(%s) returned other value: %s\n",
                _hashId, tVal.toStr().c_str(), pVal->toStr().c_str());
        rval = false;
      } else {
        //generate expiration count
        size_type delAtt = attNum + genAttempt(del_range);
        delQueue.insert(typename DelQueue::value_type(delAtt, i));
      }
    } while (rval && attNum && (i++ != max_id));

    fprintf(stderr, "\nIntHash_T<%s> %u elements remains inserted, hash size: %u\n", _hashId,
            tCache.Count(), tCache.Size());
    fprintf(stderr, "IntHash_T<%s> insertion test: %s\n", _hashId, rval ? "Ok" : "Failed");

    //delete remaining elements
    if (!delQueue.empty()) {
      for (typename DelQueue::iterator it = delQueue.begin(); it != delQueue.end(); ++it) {
        CachedValue tVal(it->second);
        if (!tCache.Pop(it->second, tVal)) {
          fprintf(stderr, "IntHash_T<%s>::Pop(%s) at attempt(%lu) failed\n", _hashId,
                  tVal.toStr().c_str(), (unsigned long)it->first);
          rval = false;
          break;
        }
      }
      fprintf(stderr, "IntHash_T<%s> deletion test: %s\n", _hashId, rval ? "Ok" : "Failed");
    }
    return rval;
  }


  bool testHash_plain(_KeyArg min_id, _KeyArg max_id)
  {
    bool rval = true;
    TstCache_t  tCache
  #if defined(INTHASH_USAGE_CHECKING) || defined(INTHASH_USAGE_DEBUG)
      (_hashId)
  #endif
    ;

    //test insertion/find
    fprintf(stderr, "\nIntHash_T<%s>::inserting elements .. \n", _hashId);
    _KeyArg i = min_id;
    do {
      CachedValue tVal(i);

      try { tCache.Insert(i, tVal);
      } catch (const std::exception & exc) {
        fprintf(stderr, "IntHash_T<%s>::Insert(%s) exception: %s\n", _hashId,
                tVal.toStr().c_str(), exc.what());
      }

      CachedValue * pVal = tCache.GetPtr(i);
      if (!pVal) {
        fprintf(stderr, "IntHash_T<%s>::GetPtr(%s) failed\n", _hashId, tVal.toStr().c_str());
        rval = false;
      } else if (!(*pVal == tVal)) {
        fprintf(stderr, "IntHash_T<%s>::GetPtr(%s) returned other value: %s\n",
                _hashId, tVal.toStr().c_str(), pVal->toStr().c_str());
        rval = false;
      }
    } while (rval && (i++ != max_id));

    fprintf(stderr, "\nIntHash_T<%s> %u elements inserted, hash size: %u\n", _hashId,
            tCache.Count(), tCache.Size());
    fprintf(stderr, "IntHash_T<%s> insertion test: %s\n", _hashId, rval ? "Ok" : "Failed");

    if (rval) {
      //test deletion
      fprintf(stderr, "\nIntHash_T<%s>::deleting elements .. \n", _hashId);
      i = min_id;
      do {
        CachedValue tVal;
        if (!tCache.Pop(i, tVal)) {
          fprintf(stderr, "IntHash_T<%s>::Pop(%s) failed\n", _hashId, tVal.toStr().c_str());
          rval = false;
        }
      } while (rval && (i++ != max_id));
      fprintf(stderr, "IntHash_T<%s> deletion test: %s\n", _hashId, rval ? "Ok" : "Failed");
    }
    return rval;
  }
};

//static const char * _hashId = "k16c16";

int main(void)
{
  int rval = 0;
/*
  HashTester_T<uint8_t>  hash8_16("ku8c16");
  if (!hash8_16.testHash_plain(0, 255)) {
    --rval;
  }
  HashTester_T<int8_t>  hashi8_16("ki8c16");
  if (!hashi8_16.testHash_plain(-128, 127)) {
    --rval;
  }

  HashTester_T<uint16_t>  hash16_16("ku16c16");
  if (!hash16_16.testHash_plain(0, 65535)) {
    --rval;
  }
  HashTester_T<int16_t>  hashi16_16("ki16c16");
  if (!hashi16_16.testHash_plain(-32768, 32767)) {
    --rval;
  }
*/
  HashTester_T<uint32_t>  hash32_16("ku32c16");
  //if (!hash32_16.testHash_plain(0, 70000000))
  //    --rval;
  if (!hash32_16.testHash_rand_del(0, 100000000, 8192))
    --rval;
  
/*
  HashTester_T<int32_t>  hashi32_16("ki32c16");
  if (!hashi32_16.testHash_plain(0, 3650))
    --rval;
*/
  HashTester_T<uint64_t>  hash64_16("ku64c16");
/*
  if (!hash64_16.testHash_plain(0, 70000000))
    --rval;*/
   if (!hash64_16.testHash_rand_del(0, 100000000, 8192))
    --rval;

/*  
  HashTester_T<int64_t>  hashi64_16("ki64c16");
  if (!hashi64_16.testHash_plain(0, 3650))
    --rval;
*/
  return rval;
}

