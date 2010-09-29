/* ************************************************************************** *
 * 
 * ************************************************************************** */
#ifndef _CORE_TIMERS_TIMEOUTS_QUEUE_HPP
#ident "@(#)$Id$"
#define _CORE_TIMERS_TIMEOUTS_QUEUE_HPP

#include <list>
#include <map>

#include "eyeline/corex/timers/ExpirationTime.hpp"

namespace eyeline {
namespace corex {
namespace timers {

//FIFO Queue of expiration times based on relative timeouts (not absolute time values)
class TimeoutsQueue : public ExpirationQueueIface {
protected:
  //Timeouts queue entry
  struct QueueEntry {
    uint32_t          _swId;
    struct timespec   _tgtTime;

    explicit QueueEntry() : _swId(0)
    {
      _tgtTime.tv_sec = 0;
      _tgtTime.tv_nsec = 0;
    }
    QueueEntry(uint32_t sw_id, const struct timespec & tgt_time)
      : _swId(sw_id), _tgtTime(tgt_time)
    { }
  };

  //Queue of expiration times based on same timeout value.
  //Arranged as indexed FIFO list
  class Chain { 
  protected:
    typedef std::list<QueueEntry> TMSQueue;
    typedef std::map<uint32_t /*sw_id*/, TMSQueue::iterator> TMSIterMap;

    TMSQueue    _tmsQueue;
    TMSIterMap  _tmsIter;
    
  public:
    Chain()
    { }
    ~Chain()
    { }

    bool empty(void) const { return _tmsQueue.empty(); }

    const QueueEntry & back(void) const
    {
      return _tmsQueue.back();
    }

    const QueueEntry & front(void) const
    {
      return _tmsQueue.front();
    }
    //
    QueueEntry pop_front(void)
    {
      QueueEntry rval = _tmsQueue.front();
      _tmsIter.erase(rval._swId);
      _tmsQueue.pop_front();
      return rval;
    }
    //
    void insert(uint32_t sw_id, const struct timespec & tgt_time)
    {
      _tmsQueue.push_back(QueueEntry(sw_id, tgt_time));
      _tmsIter.insert(std::make_pair(sw_id, --_tmsQueue.end()));
    }
    //
    void erase(uint32_t sw_id)
    {
      TMSIterMap::iterator it = _tmsIter.find(sw_id);
      if (it != _tmsIter.end()) {
        _tmsQueue.erase(it->second);
        _tmsIter.erase(it);
      }
    }
  };

  typedef std::map<TimeSlice, Chain>  TMSChainsMap;

  uint32_t      _tmsCnt;  //overall number of timespecs in queue
  TMSChainsMap  _tmsMap;
  TMSChainsMap::const_iterator  _nextChain; //chain with nearest expiration time at front

  void updateNextChain(const TMSChainsMap::const_iterator & it)
  {
    if ((_nextChain == _tmsMap.end())
        || (it->second.front()._tgtTime) < _nextChain->second.front()._tgtTime)
      _nextChain = it;
  }
  void findNextChain(void);

public:
  TimeoutsQueue() : _tmsCnt(0)
  {
    _nextChain = _tmsMap.end();
  }
  ~TimeoutsQueue()
  { }

  // -----------------------------------------------
  // -- ExpirationQueueIface mehods implementation
  // -----------------------------------------------
  //NOTE: ExpirationTime is accepted only in valTmo form
  virtual bool empty(void) const { return _tmsCnt == 0; }
  //
  virtual uint32_t size(void) const { return _tmsCnt; }
  //
  virtual bool insert(uint32_t sw_id, const ExpirationTime & tgt_time);
  //
  virtual void erase(uint32_t sw_id, const ExpirationTime & tgt_time);

  //Exports nearest expiration time of all entries
  //Returns false if queue is empty.
  virtual bool nextTime(struct timespec & exp_tm) const
  {
    if (!empty()) {
      exp_tm = _nextChain->second.front()._tgtTime;
      return true;
    }
    return false;
  }
  //Exports the last expiration time of all entries
  //Returns false if queue is empty.
  virtual bool lastTime(struct timespec & exp_tm) const;

  //Pops one of entry with expiration time less than specified.
  virtual bool popExpired(const struct timespec & exp_tm, uint32_t & sw_id);
  //Pops one of entry with expiration time less than current one.
  virtual bool popExpired(uint32_t & sw_id);
};


} //timers
} //corex
} //eyeline
#endif /* _CORE_TIMERS_TIMEOUTS_QUEUE_HPP */

