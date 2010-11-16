#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/abprov/facility/IAPThrFacility.hpp"
#include <algorithm>

using smsc::core::synchronization::ReverseMutexGuard;

namespace smsc {
namespace inman {
namespace iaprvd {
/* ************************************************************************** *
 * class IAPQueryAC implementation:
 * ************************************************************************** */
IAPQueryAC::IAPQueryAC(unsigned q_id, IAPQueryManagerITF * owner,
                       unsigned timeout_secs, Logger * use_log/* = NULL*/)
    : ThreadedTask(), _qId(q_id), _owner(owner), usage(0), timeOut(timeout_secs)
    , _qStatus(IAPQStatus::iqOk), _qError(0), logger(use_log)
{ }

IAPQueryAC::~IAPQueryAC()
{
    MutexGuard tmp(_mutex);
    if (!isReleased) {
        isReleased = true;
        _owner->releaseQuery(this);
    }
}

void IAPQueryAC::mkTaskName(void)
{
    char buf[sizeof("[%u:%lu]") + 1];
    snprintf(buf, sizeof(buf), "[%u:%lu]", _qId, usage);
    tName = taskType();
    tName += buf;
}

bool IAPQueryAC::init(const AbonentId & ab_number)
{
    MutexGuard tmp(_mutex);
    _qStatus = IAPQStatus::iqOk;
    _qError = 0;
    abonent = ab_number;
    abInfo.clear();
    isReleased = isStopping = false;
    usage++;
    mkTaskName();
    _exc.clear();
    return true;
}

RCHash  IAPQueryAC::getError(void) const
{
  return (!_qError && (_qStatus != IAPQStatus::iqOk)) ?
          _RCS_IAPQStatus->mkhash(_qStatus) : _qError;
}

std::string IAPQueryAC::status2Str(void) const
{
    std::string st;
    switch (_qStatus) {
    case IAPQStatus::iqOk:  st += "finished"; break;
    case IAPQStatus::iqCancelled: st += "cancelled"; break;
    case IAPQStatus::iqTimeout: {
        st += "timed out";
        if (!_exc.empty()) {
            st += ", "; st += _exc;
        }
    } break;
    default: {
        st += "failed";
        if (!_exc.empty()) {
            st += ", "; st += _exc;
        } else if (_qError) {
            st += ", "; st += URCRegistry::explainHash(_qError);
        }
    }
    } /* eosw */
    return st;
}


//This one is called by ThreadPool on Exceute() completion
void IAPQueryAC::onRelease(void)
{
    MutexGuard tmp(_mutex);
    isReleased = true;
    _owner->releaseQuery(this);
}

/* ************************************************************************** *
 * class IAProviderFacility implementation:
 * ************************************************************************** */
IAPQueryFacility::IAPQueryFacility(const IAPFacilityCFG & in_cfg, Logger * uselog/* = NULL*/)
    : _cfg(in_cfg), _lastQId(0), _logId("IAPrvd")
{
    logger = uselog ? uselog : Logger::getInstance("smsc.inman.iaprvd");
    pool.setMaxThreads((int)_cfg.max_queries);
}

IAPQueryFacility::~IAPQueryFacility()
{
    //stop and wait for active quieries
    Stop(true);
    //clean up quieries pool
    for (QueriesList::iterator it = qryPool.begin(); it != qryPool.end(); ++it)
        delete *it;
    //clean up quieries cache
    if (qryCache.GetUsage()) {
        smsc_log_warn(logger, "%s: shutdown - queries cache is not empty!", _logId);
    }
    qryCache.Empty();
    smsc_log_debug(logger, "%s: shutdown complete", _logId);
}

bool IAPQueryFacility::Start(void)
{
    MutexGuard  guard(qrsGuard);
    if (_cfg.init_threads) {
        pool.preCreateThreads((int)_cfg.init_threads);
        _cfg.init_threads = 0;
    }
    return true;
}

void IAPQueryFacility::Stop(bool do_wait/* = false*/)
{
    cancelAllQueries();
    if (do_wait)
        pool.shutdown(); //waits for  threads (kills if necessary)
    else
        pool.stopNotify();
    return;
}

void IAPQueryFacility::cancelAllQueries(void)
{
    MutexGuard  guard(qrsGuard);

    char *          ab_number = NULL; //AbonentId.value !!!
    CachedQuery *   ab_rec = NULL;

    QueriesHash::Iterator cit = qryCache.getIterator();
    while (cit.Next(ab_number, ab_rec)) {
        smsc_log_debug(logger, "%s: %s(%s): cancelling, %u listeners set", _logId,
                       ab_rec->query->taskName(), ab_number, ab_rec->cbList.size());
        ab_rec->cbList.clear();
        {
          ReverseMutexGuard rGrd(qrsGuard);
          ab_rec->query->stop();
        }
    }
    return;
}


bool IAPQueryFacility::hasListeners(const AbonentId & ab_number)
{
    MutexGuard  guard(qrsGuard);
    CachedQuery * ab_rec = qryCache.GetPtr(ab_number.getSignals());
    return (ab_rec && !ab_rec->cbList.empty()) ? true : false;
}

//This one is called from ThreadedTask on DB query completion.
//Notifies query listeners and releases query.
void IAPQueryFacility::releaseQuery(IAPQueryAC * query)
{
    //Notify listeners if any, and remove query from active queries cache
    {
        qrsGuard.Lock();
        CachedQuery *   qry_rec = qryCache.GetPtr((query->getAbonentId()).getSignals());

        if (qry_rec) {
            if (qry_rec->cbList.empty())
                smsc_log_debug(logger, "%s: %s(%s): has been cancelled", _logId,
                               query->taskName(), (query->getAbonentId()).getSignals());
            else {
                //tell listeners the contract type, even if query was failed (abtUnknown)
                do {
                    IAPQueryListenerITF * hdl = qry_rec->cbList.front();
                    qry_rec->cbList.pop_front();
                    qrsGuard.Unlock();
                    try { hdl->onIAPQueried(query->getAbonentId(),
                                        query->getAbonentInfo(), query->getError());
                    } catch (const std::exception & exc) {
                        smsc_log_error(logger, "%s: %s(%s): listener exception: %s", _logId,
                                query->taskName(), (query->getAbonentId()).getSignals(),
                                exc.what());
                    }
                    qrsGuard.Lock();
                } while (!qry_rec->cbList.empty());
            }
            qryCache.Delete((query->getAbonentId()).getSignals());
        } else
            smsc_log_error(logger, "%s: %s(%s): not in cache", _logId,
                           query->taskName(), (query->getAbonentId()).getSignals());
        qrsGuard.Unlock();
    }
    {   //update queries pool
        MutexGuard  guard(qrsGuard); 
        if (!query->delOnCompletion())
            qryPool.push_back(query);
        //else query is being deleted by PooledThread::Execute()
        if (query->Status() == IAPQStatus::iqOk)
            smsc_log_info(logger, "%s: %s(%s): finished, %s", _logId,
                    query->taskName(), query->getAbonentId().getSignals(),
                    query->getAbonentInfo().toString().c_str());
        else
            smsc_log_info(logger, "%s: %s(%s): %s", _logId,
                query->taskName(), query->getAbonentId().getSignals(),
                query->status2Str().c_str());
    }
    return;
}

// ----------------------------------------------
// InAbonentProviderITF interface implementation:
// ----------------------------------------------
//Starts query and binds listener to it.
//Returns true if query succesfully started, false otherwise
//NOTE: the AbonentId is copied into AbonentQuery
bool IAPQueryFacility::startQuery(const AbonentId & ab_number, 
                                   IAPQueryListenerITF * pf_cb/* = NULL*/)
{
    CachedQuery  qryRec;
    {
        MutexGuard  guard(qrsGuard);
        CachedQuery * qry_rec = qryCache.GetPtr(ab_number.getSignals());
        if (qry_rec) { //allocated query exists, just add callback to list
            qry_rec->cbList.push_back(pf_cb);
            smsc_log_debug(logger, "%s: %s(%s): listener is added", _logId,
                           qry_rec->query->taskName(), ab_number.getSignals());
            return true;
        }
    
        if (!qryPool.empty()) {
            qryRec.query = qryPool.front();
            qryPool.pop_front();
        } else {
            _lastQId++;
            qryRec.query = _cfg.qryPlant->newQuery(_lastQId, this, logger);
            if (!qryRec.query)
                return false;
        }
        qryRec.cbList.push_back(pf_cb);
        if (!qryRec.query->init(ab_number)) {
            smsc_log_error(logger, "%s: %s(%s): failed to init", _logId,
                            qryRec.query->taskName(), ab_number.getSignals());
            return false;
        }
        qryCache.Insert(ab_number.getSignals(), qryRec);
        smsc_log_debug(logger, "%s: %s(%s): added to queue", _logId,
                        qryRec.query->taskName(), ab_number.getSignals());
    }
    pool.startTask(qryRec.query, !_cfg.qryMultiRun); //do not delete task on completion !!!
    return true;
}

//Unbinds query listener and cancels query
void IAPQueryFacility::cancelQuery(const AbonentId & ab_number, IAPQueryListenerITF * pf_cb)
{
    MutexGuard  guard(qrsGuard);
    CachedQuery * qry_rec = qryCache.GetPtr(ab_number.getSignals());
    if (!qry_rec) {
        smsc_log_error(logger, "%s: attempt to cancel unexisting query!", _logId);
        return;
    }
    QueryCBList::iterator it = std::find(qry_rec->cbList.begin(),
                                         qry_rec->cbList.end(), pf_cb);
    if (it != qry_rec->cbList.end())
        qry_rec->cbList.erase(it);

    if (logger->isDebugEnabled()) {
      if (!qry_rec->cbList.empty())
          smsc_log_debug(logger, "%s: %s(%s): %u listeners remain", _logId,
                         qry_rec->query->taskName(), ab_number.getSignals(),
                         qry_rec->cbList.size());
      else
          smsc_log_debug(logger, "%s: %s(%s): cancelled", _logId,
                          qry_rec->query->taskName(), ab_number.getSignals());
    }
    return;
}

} //iaprvd
} //inman
} //smsc


