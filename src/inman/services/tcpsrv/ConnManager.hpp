/* ************************************************************************* *
 * 
 * ************************************************************************* */
#ifndef __SMSC_INMAN_CONNECT_MANAGER_HPP
#ident "@(#)$Id$"
#define __SMSC_INMAN_CONNECT_MANAGER_HPP

#include "inman/interaction/connect.hpp"
using smsc::inman::interaction::Connect;
using smsc::inman::interaction::ConnectListenerITF;

#include "inman/interaction/messages.hpp"
using smsc::inman::interaction::INPPacketAC;

namespace smsc {
namespace inman {
namespace tcpsrv {

class ConnectManagerAC;

class WorkerAC {
protected:
    unsigned            _wId; //unique worker id
    ConnectManagerAC *  _mgr;
    Logger *            logger;

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
            for (WorkerList::iterator it = corpses.begin(); it != corpses.end(); ++it)
                delete (*it);
            corpses.clear();
        }
    }

public:
    ConnectManagerAC(unsigned cm_id, Connect* conn, Logger * uselog = NULL)
        : _cmId(cm_id), _conn(conn), logger(uselog)
    { _logId[0] = 0; }

    virtual ~ConnectManagerAC()
    {
        Abort("Connect destroyed"); //abort active workers
        MutexGuard grd(_mutex);
        cleanUpWorkers();   //delete died workers
    }

    unsigned cmId(void) const { return _cmId; }

    CustomException * connectError(void) const
    { return _conn ? _conn->hasException() : NULL; }

    void Bind(Connect* use_conn)
    {
        MutexGuard  grd(_mutex);
        _conn = use_conn;
    }

    //sends command, returns true on success
    virtual bool sendCmd(INPPacketAC* cmd)
    {
        MutexGuard  grd(_mutex);
        return (_conn ? (bool)(_conn->sendPck(cmd) > 0) : false);
    }

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

    virtual void Abort(const char * reason = NULL)
    {
        MutexGuard grd(_mutex);
        if (!workers.empty()) { //abort all active workers
            if (reason && reason[0])
                smsc_log_error(logger, "%s: aborting, reason: %s", _logId, reason);
            else
                smsc_log_error(logger, "%s: aborting ..", _logId);

            for (WorkersMap::iterator it = workers.begin(); it != workers.end(); ++it) {
                WorkerAC * worker = it->second;
                worker->Abort(reason);
                corpses.push_back(worker);
            }
            workers.clear();
        }
    }

    // -- ConnectListenerITF interface methods
    //virtual void onPacketReceived(Connect* conn, std::auto_ptr<SerializablePacketAC>& recv_cmd)
    //                     /*throw(std::exception)*/ = 0;
    //Stops all Workers due to error condition pending on socket
    virtual void onConnectError(Connect* conn, std::auto_ptr<CustomException>& p_exc)
    {
        Abort(p_exc->what());
    }
};


template <class _ConfigTArg>
class ConnectManagerT : public ConnectManagerAC {
protected:
    const _ConfigTArg & _cfg;

public:
    ConnectManagerT(const _ConfigTArg & cfg, unsigned cm_id,
                                Connect* conn, Logger * uselog = NULL)
        : ConnectManagerAC(cm_id, conn, uselog), _cfg(cfg)
    { }

    const _ConfigTArg & getConfig(void) const { return _cfg;}
};

} //tcpsrv
} //inman
} //smsc
#endif /* __SMSC_INMAN_CONNECT_MANAGER_HPP */

