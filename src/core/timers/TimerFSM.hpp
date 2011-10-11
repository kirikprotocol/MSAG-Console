/* ************************************************************************** *
 * MT Realtime timers facility: 
 * TimerFSM  - an object managing timer lifecycle.
 * ************************************************************************** */
#ifndef SMSC_CORE_TIMERS_TIMERFSM_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_CORE_TIMERS_TIMERFSM_HPP

#include "core/synchronization/Mutex.hpp"
#include "core/buffers/FixedLengthString.hpp"
#include "core/buffers/IntrList.hpp"
#include "core/buffers/IntrusivePoolOfUniqueT.hpp"

#include "core/timers/TimerDefs.hpp"
#include "core/timers/ExpirationTime.hpp"

namespace smsc {
namespace core {
namespace timers {

class TimerRefereeIface;  //forward declaration

//Timer Finite State Machine.
class TimerFSM : public smsc::util::UniqueObj_T<smsc::core::synchronization::Mutex, TimerUId> {
public:
  enum ProcState_e {
    swIdle  = 0x0 //Timer is idle (unused)
  , swInited      //Timer is initialized (but not activated)
  , swActive      //Timer expiration is currently monitored
  , swIsToSignal  //Timer is expired/aborted and awaiting to be signalled.
  , swSignalling  //Timer is currently signalling
  };

  //Timer activation result
  enum ActivationResult_e {
    actOk = 0x0         // succeeded.
  , actBadTime = 0x1    //expiration time is invalid or has a unsupported from.
  , actRfrState = 0x2   //referee unable to monitor timer due to its own state.
  , actFsmState = 0x3   //FSM isn't in swInited state.
  };

  static const size_t k_maxStatPfxSZ = 16; //Timer statistics string prefix
  //Timer statistics string. Contains info about timer unique number and usage.
  static const size_t k_maxStatStrSZ = k_maxStatPfxSZ + 2*sizeof(uint32_t)*3 + 4;
  typedef smsc::core::buffers::FixedLengthString<k_maxStatStrSZ + 1>  StrStat_t;

  static const char * nmState(ProcState_e st_val);

  typedef smsc::core::buffers::IntrusivePoolOfUnique_T<TimerFSM, TimerUId>::ObjRef
    ObjRef_t;
  
  class ObjLink : public smsc::core::buffers::IntrListNodeBase<ObjLink> {
  protected:
    ObjRef_t          mSwGrd;
    struct timespec   mTgtTMS; //absolute value of timer target time

  public:
    ObjLink() : smsc::core::buffers::IntrListNodeBase<ObjLink>()
    {
      mTgtTMS.tv_sec = mTgtTMS.tv_nsec = 0;
    }
    ~ObjLink()
    { }

    void init(const ObjRef_t & use_grd, const struct timespec & use_tms)
    {
      mTgtTMS = use_tms; mSwGrd = use_grd;
    }
    //
    void clear(void)
    {
      mSwGrd.release(); 
      mTgtTMS.tv_sec = mTgtTMS.tv_nsec = 0;
    }

    bool isLinked(void) const { return (this->ilPrevNode || this->ilNextNode); }
    bool empty(void) const { return mSwGrd.empty(); }

    const ObjRef_t & getRef(void) const { return mSwGrd; }
    const struct timespec & getTms(void) const { return mTgtTMS; }

    TimerFSM * operator->(void) const { return mSwGrd.get(); }
  };

  // -- ******************************************************************** --
  // -- NOTE: following methods require TimerFSM being locked !!!
  // -- ******************************************************************** --
  //
  bool  getState(void) const { return mSwState; }
  const char * nmState(void) const { return nmState(mSwState); }
  //Returns timer statistics string.
  const char * getStatStr(void) const { return mStat.c_str(); }
  //
  const ExpirationTime & getTgtTime(void) const { return mTgtTime; }

  //Switches FSM to swInited.
  //Returns false in case of inconsistent FSM state.
  bool init(TimerRefereeIface & use_rfr, const char * id_pfx,
                    TimerListenerIface & p_hdl, const ExpirationTime & tgt_time);

  //Attempts to switch FSM to swActive and asks referee to monitor expiration.
  ActivationResult_e activate(const ObjRef_t & this_ref);
  //switches FSM to swIsToSignal 
  bool setToSignal(void);
  //switches FSM to swSignalling, notifies Timer listener and
  //switches FSM to eiher swInited or swIdle states.
  TimerEventResult_e notify(void);
  //tries to switch FSM to swInited and unlinks itself from owning queue.
  //returns true if succeeded, false if Timer is currently signaling.
  bool stop(void);
  //tries to switche FSM to swIdle and unlinks itself from owning queue.
  //returns true if succeeded, false if Timer is currently signaling
  bool release(void);

private:
  StrStat_t             mStat;
  uint32_t              mUsage;     //stopwatch usage cyclic counter
  volatile ProcState_e  mSwState;   //current Timer FSM state
  volatile ProcState_e  mNxtState;  //target FSM state the Timer ought to advance after signalling
  ExpirationTime        mTgtTime;
  TimerRefereeIface *   mReferee;
  TimerListenerIface *  mEventHdl;
  ObjLink               mLink;

  void mkIdent(const char * id_pfx);

protected:
  explicit TimerFSM(TimerUId tm_id)
    : smsc::util::UniqueObj_T<smsc::core::synchronization::Mutex, TimerUId>(tm_id)
    , mUsage(0), mSwState(swIdle), mNxtState(swIsToSignal), mReferee(0), mEventHdl(0)
  {
    mkIdent("");
  }
  ~TimerFSM()
  { }
};

typedef TimerFSM::StrStat_t       TimerStat_t;
typedef TimerFSM::ProcState_e     TimerState_e;
typedef TimerFSM::ObjLink         TimerLink;
typedef TimerFSM::ObjRef_t        TimerRef;
typedef smsc::core::buffers::IntrusivePoolOfUnique_T<TimerFSM, TimerUId> TimersPool;

//
class TimerRefereeIface {
public:
  //Inserts given TimerLink to pool of monitored timers.
  virtual TimerFSM::ActivationResult_e enqueue(TimerLink & sw_link) /*throw()*/= 0;
  //NOTE: It's a caller responsibility to ensure that given TimerLink was
  //      inserted previously !!!
  virtual void unlink(TimerLink & sw_link) /*throw()*/= 0;

protected:
  TimerRefereeIface()
  { }
  virtual ~TimerRefereeIface()
  { }
};

} //timers
} //core
} //smsc
#endif /* SMSC_CORE_TIMERS_TIMERFSM_HPP */

