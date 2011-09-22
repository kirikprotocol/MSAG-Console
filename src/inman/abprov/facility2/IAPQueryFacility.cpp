#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/abprov/facility2/IAPQueryFacility.hpp"
using smsc::core::synchronization::MutexGuard;
using smsc::core::synchronization::ReverseMutexGuard;

namespace smsc {
namespace inman {
namespace iaprvd {

/* ************************************************************************** *
 * class IAPQueryFacility implementation:
 * ************************************************************************** */
const TimeSlice  IAPQueryFacility::_dflt_ShutdownTmo(300, TimeSlice::tuMSecs);

//Returns false in case of inconsistent facility state.
//NOTE: setting max_threads to 0 turns off threads number limitation!
bool IAPQueryFacility::init(IAPQueriesPoolIface & use_pool, uint16_t max_threads/* = 0*/)
{
  MutexGuard   grd(mSync);
  if (mState <= fctInited) {
    mQryPool = &use_pool;
    mIapNtfr.init(1, max_threads);
    mState = fctInited;
    return true;
  }
  return false;
}

//Returns false in case of inconsistent facility state
bool IAPQueryFacility::start(void)
{
  MutexGuard   grd(mSync);
  if (mState < fctInited)
    return false;
  if (mState < fctStopping)
    mIapNtfr.start();
  mState = fctRunning;
  return true;
}

//Stops processing of a new queries (switches facility to fctStopping state).
//if a 'do_wait' argument is set, kills all active queries as shutdown timeout
//is expired and switches facility to fctInited state.
IAPQueryFacility::State_e IAPQueryFacility::stop(bool do_wait)
{
  cancelAllQueries();
  if (do_wait) {
    mIapNtfr.stop(mShtdTmo);
    {
      MutexGuard   grd(mSync);
      mState = fctInited;
    }
  }
  return mState;
}

bool IAPQueryFacility::isRegistered(const AbonentId & ab_number) const
{
  MutexGuard   grd(mSync);
  return (mQryReg.GetPtr(ab_number) != 0);
}

bool IAPQueryFacility::isRegistered(const IAPQueryRef & p_qry) const
{
  MutexGuard   grd(mSync);
  const IAPQueryRef * oldRef = mQryReg.GetPtr(p_qry->getAbonentId());
  return (oldRef && ((*oldRef).get() == p_qry.get()));
}

bool IAPQueryFacility::activateQuery(const IAPQueryRef & p_qry, IAPQueryListenerITF & pf_cb)
{
  MutexGuard grd(*(p_qry.get()));
  p_qry->addListener(pf_cb); //cann't fail

  IAPQueryAC::ProcResult_e rval = p_qry->start();
  if (rval != IAPQueryAC::procOk) {
    smsc_log_error(mLogger, "%s: failed to start %s(%s)", mlogId,
                   p_qry->taskName(), p_qry->getAbonentId().getSignals());
    p_qry->removeListener(pf_cb); //listener will know the query result by return code
    if (rval == IAPQueryAC::procNeedReport)
      procQueryEvent(p_qry); //handle qryStopping/qryDone event
    return false;
  }
  smsc_log_debug(mLogger, "%s: started %s(%s)", mlogId,
                 p_qry->taskName(), p_qry->getAbonentId().getSignals());
  return true;
}

//NOTE: query MUST be locked upon entry, but Facility sync - unlocked!!!
void IAPQueryFacility::procQueryEvent(const IAPQueryRef & p_qry)
{
  if (p_qry->getStage() >= IAPQueryAC::qryStopping) { //unregister reported query
    MutexGuard   grd(mSync);
    if (!mQryReg.Delete(p_qry->getAbonentId())) {
      smsc_log_debug(mLogger, "%s: non-registered %s(%s) reported event(%s)", mlogId,
                    p_qry->taskName(), p_qry->getAbonentId().getSignals(), p_qry->nmStage());
    }
  } else {
    if (!isRegistered(p_qry->getAbonentId())) {
      smsc_log_error(mLogger, "%s: non-registered %s(%s) reported event(%s)", mlogId,
                    p_qry->taskName(), p_qry->getAbonentId().getSignals(), p_qry->nmStage());
    } else {
      smsc_log_debug(mLogger, "%s: %s(%s) reported event(%s)", mlogId,
                    p_qry->taskName(), p_qry->getAbonentId().getSignals(), p_qry->nmStage());
    }
  }
  if (p_qry->getStage() < IAPQueryAC::qryDone)
    mIapNtfr.onQueryEvent(p_qry); //handle event by one of notifier's thread
}

// ------------------------------------------
// -- IAPQueryRefereeIface interface methods
// ------------------------------------------
void IAPQueryFacility::onQueryEvent(IAPQueryId qry_id)
{
  IAPQueryRef pQry = mQryPool->atQuery(qry_id);
  if (pQry.empty()) { //interanl inconsistency !!!
    smsc_log_fatal(mLogger, "%s: unknown query(UId = %u) reported event", mlogId, (unsigned)qry_id);
  } else {
    MutexGuard   grd(*(pQry.get()));
    procQueryEvent(pQry);
  }
}

// ----------------------------------------------
// IAPQueryProcessorITF interface implementation:
// ----------------------------------------------
//Starts query and binds listener to it.
//Returns true if query succesfully started, false otherwise
bool IAPQueryFacility::startQuery(const AbonentId & ab_number, IAPQueryListenerITF & pf_cb)
{
  bool rval = false;
  do {
    IAPQueryRef   pQry;
    IAPQueryRef * oldRef = 0;
    {
      MutexGuard grd(mSync);
      if (mState != fctRunning)
        return false;

      if (!(oldRef = mQryReg.GetPtr(ab_number))) {  //start a new query
        pQry = mQryPool->allcQuery();
        if (pQry.empty()) {
          smsc_log_error(mLogger, "%s: unable to allocate query", mlogId);
          return false;
        }
        pQry->init(*this, ab_number);
        mQryReg.Insert(ab_number, pQry);
        /* */
      } else  //add a listener to existing query
        pQry = *oldRef;
    }
    if (!oldRef)
      return activateQuery(pQry, pf_cb);

    //add a listener to query
    {
      MutexGuard grd(*pQry.get());
      if (pQry->addListener(pf_cb) != IAPQueryAC::procLater) {
        smsc_log_debug(mLogger, "%s: listener is added to %s(%s)", mlogId,
                       pQry->taskName(), ab_number.getSignals());
        rval = true;
      }
      //RARE case: query is just switched to qryStopping and is still
      //registered, so wait until it will be discharged.
      do {
        smsc_log_debug(mLogger, "%s: awaiting %s(%s) to be unregistered", mlogId,
                       pQry->taskName(), ab_number.getSignals());
        pQry->wait();
      } while (isRegistered(pQry));
      pQry.release();
    }
  } while (!rval);
  return rval;
}

//Unbinds query listener, cancels query if no listeners remain.
bool IAPQueryFacility::cancelQuery(const AbonentId & ab_number, IAPQueryListenerITF & pf_cb)
{
  IAPQueryRef pQry;
  {
    MutexGuard  grd(mSync);
    IAPQueryRef * pRef = mQryReg.GetPtr(ab_number);
    if (!pRef)
      return true;
    pQry = *pRef;
  }
  {
    MutexGuard  grd(*(pQry.get()));
    if (pQry->removeListener(pf_cb) == IAPQueryAC::procLater)
      return false;
    smsc_log_debug(mLogger, "%s: listener is removed from %s(%s)", mlogId,
                   pQry->taskName(), ab_number.getSignals());

    if (!pQry->hasListener()) {
      smsc_log_debug(mLogger, "%s: cancelling %s(%s)", mlogId,
                     pQry->taskName(), ab_number.getSignals());

      if (pQry->cancel(false) == IAPQueryAC::procNeedReport) //cann't fail here
        procQueryEvent(pQry);
    }
  }
  return true;
}

//
bool IAPQueryFacility::cancelAllQueries(void)
{
  bool rval = true;
  MutexGuard   grd(mSync);

  if (mState == fctRunning)
    mState = fctStopping; //no new queries will be created

  if (mQryReg.Count()) { //abort all registered queries
    smsc_log_info(mLogger, "%s: cancelling %u queries ..", mlogId, (unsigned)mQryReg.Count());
    WorkersMap::iterator it = mQryReg.begin();
    do {
      IAPQueryRef pQry = it->_value;
      {
        ReverseMutexGuard rGrd(mSync);
        {
          MutexGuard  qGrd(*(pQry.get()));
          IAPQueryAC::ProcResult_e res = pQry->cancel(false);
          if (res == IAPQueryAC::procNeedReport)
            procQueryEvent(pQry);
          if (res == IAPQueryAC::procLater)
            rval = false;
        }
      }
      pQry.release();
    } while (!(++it).isEnd()); //mQryReg specific!
  }
  return rval;
}

} //iaprvd
} //inman
} //smsc


