#pragma ident "$Id$"
/* ************************************************************************* *
 * 
 * ************************************************************************* */
#ifndef __SMSC_INMAN_CONNECT_MANAGER_HPP
#define __SMSC_INMAN_CONNECT_MANAGER_HPP

#include "inman/interaction/connect.hpp"
using smsc::inman::interaction::Connect;
using smsc::inman::interaction::ConnectListenerITF;

#include "inman/interaction/messages.hpp"
using smsc::inman::interaction::INPPacketAC;

#include "inman/common/TimeWatcher.hpp"
using smsc::core::timers::TimeWatchersRegistry;
using smsc::core::timers::TimeWatcherTMO;
using smsc::core::timers::TimerHdl;
using smsc::core::timers::TimerListenerITF;
using smsc::core::timers::OPAQUE_OBJ;

namespace smsc    {
namespace inman   {

class ConnectManagerAC;

class WorkerAC {
protected:
    unsigned  _wId; //unique worker id
    Logger *  logger;
    ConnectManagerAC * _mgr;

public:
    WorkerAC(unsigned w_id, ConnectManagerAC * owner, Logger * use_log = NULL)
        : _wId(w_id), _mgr(owner), logger(use_log)
    { }
    virtual ~WorkerAC() { }

    unsigned getId(void) const { return _wId; }

    virtual void     handleCommand(INPPacketAC* cmd) = 0;
    virtual void     Abort(const char * reason = NULL) = 0;
};

#define MAX_CONNECT_MGR_NAME 50
class ConnectManagerAC : public ConnectListenerITF {
protected:
    typedef std::map<unsigned, WorkerAC*>   WorkersMap;
    typedef std::list<WorkerAC*>            WorkerList;

    Mutex       _mutex;
    unsigned    _cmId;
    Connect*    _conn;
    Logger*     logger;
    WorkersMap   workers;
    WorkerList  corpses;
    //logging prefix, f.ex: "ConnectManagerAC[%u]"
    char        _logId[MAX_CONNECT_MGR_NAME + sizeof("[%u]") + sizeof(unsigned)*3 + 1];

    inline void cleanUpWorkers(void)
    {
        if (!corpses.empty()) {
            for (WorkerList::iterator it = corpses.begin(); it != corpses.end(); it++)
                delete (*it);
            corpses.clear();
        }
    }

public:
    ConnectManagerAC(unsigned cm_id, Connect* conn, Logger * uselog = NULL)
        : _cmId(cm_id), _conn(conn), logger(uselog)
    { _logId[0] = 0; }

    unsigned cmId(void) const { return _cmId; }

    CustomException * connectError(void) const
    { return _conn ? _conn->hasException() : NULL; }

    virtual ~ConnectManagerAC()
    {
        MutexGuard grd(_mutex);
        cleanUpWorkers();   //delete died workers first

        if (!workers.empty()) { //abort all active workers
            for (WorkersMap::iterator it = workers.begin(); it != workers.end(); it++) {
                WorkerAC * worker = (*it).second;
                worker->Abort("Connect destroyed");
                delete worker;
            }
            workers.clear();
        }
    }

    //sends command, returns true on success
    virtual bool sendCmd(INPPacketAC* cmd)
    { return (_conn ? (bool)(_conn->sendPck(cmd) > 0) : false); }

    virtual void workerDone(WorkerAC* worker)
    {
        MutexGuard grd(_mutex);
        cleanUpWorkers();   //delete died workers first

        unsigned int wId = worker->getId();
        WorkersMap::iterator it = workers.find(wId);
        if (it == workers.end())
            smsc_log_error(logger, "%s: Attempt to free unregistered Worker[%u]",
                            _logId, wId);
        else
            workers.erase(it);
        corpses.push_back(worker);
        return;
    }

    // -- ConnectListenerITF interface methods
    //virtual void onPacketReceived(Connect* conn, std::auto_ptr<SerializablePacketAC>& recv_cmd)
    //                     /*throw(std::exception)*/ = 0;
    //Stops all Workers due to error condition pending on socket
    virtual void onConnectError(Connect* conn, std::auto_ptr<CustomException>& p_exc)
    {
        MutexGuard grd(mutex);
        const char * reason = p_exc->what();
        smsc_log_error(logger, "%s: %s", _logId, reason);

        if (!workers.empty()) { //abort all active workers
            for (WorkersMap::iterator it = workers.begin(); it != workers.end(); it++) {
                WorkerAC * worker = (*it).second;
                worker->Abort(reason);
                corpses.push_back(worker);
            }
            workers.clear();
        }
        return;
    }
};

template <class ConfigTA>
class ConnectManagerT : public ConnectManagerAC {
protected:
    ConfigTA    _cfg;

public:
    ConnectManagerT(const ConfigTA * cfg, unsigned cm_id, Connect* conn, Logger * uselog = NULL)
        : ConnectManagerAC(cm_id, conn, uselog), _cfg(*cfg)
    { }

    const ConfigTA & getConfig(void) const { return _cfg;}
};


class TimeoutHDL {
protected:
    unsigned short  _tmo; //timeout in seconds
    TimeWatcherTMO * _tw;

public:
    TimeoutHDL(unsigned short tmo_secs = 0)
        : _tmo(tmo_secs), _tw(0)
    { }
    ~TimeoutHDL()
    { }

    inline unsigned short Value(void) const { return _tmo; }

    inline void Init(TimeWatchersRegistry * tw_reg, uint32_t num_tmrs = 0)
    {
        _tw = tw_reg->getTmoTimeWatcher((long)_tmo, false, num_tmrs);
    }

    inline TimerHdl CreateTimer(TimerListenerITF * listener,
                                OPAQUE_OBJ * opaque_obj = NULL)
    {
        return _tw->CreateTimer(listener, opaque_obj);
    }

    inline TimeoutHDL & operator= (unsigned short tmo_secs)
    {
        _tmo = tmo_secs;
        return *this;
    }
};


} //inman
} //smsc
#endif /* __SMSC_INMAN_CONNECT_MANAGER_HPP */

