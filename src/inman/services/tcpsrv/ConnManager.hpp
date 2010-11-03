/* ************************************************************************* *
 * ConnectManagerT: generalizes implementation of ConnectListenerITF for
 * handling INMan asynchronous TCP protocols.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_CONNECT_MANAGER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_CONNECT_MANAGER_HPP

#include "inman/interaction/connect.hpp"
#include "inman/interaction/messages.hpp"

namespace smsc {
namespace inman {
namespace tcpsrv {

using smsc::inman::interaction::Connect;
using smsc::inman::interaction::ConnectListenerITF;
using smsc::inman::interaction::INPPacketAC;

class ConnectManagerAC;

//NOTE: worker MUST notify ConnectManagerAC about its completion by
//      calling ConnectManagerAC::workerDone()
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

    //Prints some information about worker state/status
    virtual void    logState(std::string & use_str) const = 0;
    //
    virtual void    handleCommand(INPPacketAC* cmd) = 0;
    //
    virtual void    Abort(const char * reason = NULL) = 0;
};

#define MAX_CONNECT_MGR_NAME 50
class ConnectManagerAC : public ConnectListenerITF {
private:
    typedef std::map<unsigned, WorkerAC*>   WorkersMap;
    typedef std::list<WorkerAC*>            WorkersList;

    WorkersMap    _workers; //workers registry, may NOT contain NULL !!!
    WorkersList   _corpses; //died workers are to delete

    volatile bool         _running;
    WorkersMap::iterator  _itLocked; //node of workers registry that is locked
                                     //(cann't be changed or erased)
    WorkersMap::iterator  _itToErase;  //node of workers registry is to erase

    //NOTE: _mutex SHOULD NOT be locked upon entry!
    void cleanUpWorkers(void)
    {
        MutexGuard grd(_mutex);
        while (!_corpses.empty()) {
          WorkerAC * pWorker = _corpses.front();
          _corpses.pop_front();
          {
            ReverseMutexGuard rGrd(_mutex);
            delete pWorker;
          }
        }
    }

protected:
    Mutex       _mutex;
    unsigned    _cmId;
    Connect*    _conn;
    Logger*     logger;
    //logging prefix, f.ex: "ConnectManagerAC[%u]"
    char        _logId[MAX_CONNECT_MGR_NAME + sizeof("[%u]") + sizeof(unsigned)*3 + 1];

    /* -------------------------------------------------------------- */
    /* NOTE: _mutex must be locked prior to calling protected methods */
    /* -------------------------------------------------------------- */
    unsigned numWorkers(void) const
    {
        return (unsigned)_workers.size();
    }
    //dumps workers state to string
    void dumpWorkers(std::string & dump)
    {
      if (!_workers.empty()) {
        _itLocked = _workers.begin();
        do {
          if (_itLocked != _itToErase) {
            ReverseMutexGuard rGrd(_mutex);
            _itLocked->second->logState(dump);
            dump += ", ";
          }
        } while (++_itLocked != _workers.end());
      }
    }
    //
    WorkerAC * getWorker(unsigned w_id)
    {
        WorkersMap::iterator it = _workers.find(w_id);
        return it != _workers.end() ? it->second : NULL;
    }

    bool insWorker(WorkerAC * p_worker)
    {
        std::pair<WorkersMap::iterator, bool> res = 
          _workers.insert(WorkersMap::value_type(p_worker->getId(), p_worker));
        return res.second;
    }

    bool isRunning(void) const { return _running; }

public:
    ConnectManagerAC(unsigned cm_id, Connect* conn, Logger * uselog = NULL)
        : _running(true), _cmId(cm_id), _conn(conn), logger(uselog)
    {
      _logId[0] = 0;
      _itLocked = _itToErase = _workers.end();
    }

    virtual ~ConnectManagerAC()
    {
        Abort("Connect destroyed"); //abort active _workers
        cleanUpWorkers();   //delete died _workers
    }

    unsigned cmId(void) const { return _cmId; }

    CustomException * connectError(void) const
    {
      return _conn ? _conn->hasException() : NULL;
    }

    void Bind(Connect* use_conn)
    {
        MutexGuard  grd(_mutex);
        _conn = use_conn;
    }

    //sends command, returns true on success
    bool sendCmd(INPPacketAC* cmd)
    {
        MutexGuard  grd(_mutex);
        return (_conn ? (bool)(_conn->sendPck(cmd) > 0) : false);
    }

    void workerDone(WorkerAC * p_worker)
    {
        cleanUpWorkers();   //delete died workers first

        MutexGuard grd(_mutex);
        unsigned int wId = p_worker->getId();
        WorkersMap::iterator it = _workers.find(wId);
        if (it == _workers.end()) {
          smsc_log_error(logger, "%s: Attempt to free unregistered Worker[%u]",
                          _logId, wId);
        } else if (_itLocked == it) {
          _itToErase = it; //keep node intact, just mark it as target to erase
        } else {
          _workers.erase(it);
          _corpses.push_back(p_worker);
        }
        return;
    }

    void Abort(const char * reason = NULL)
    {
        MutexGuard grd(_mutex);
        _running = false; //no new worker will be created
        if (reason && reason[0])
            smsc_log_error(logger, "%s: aborting, reason: %s", _logId, reason);
        else
            smsc_log_error(logger, "%s: aborting ..", _logId);

        if (!_workers.empty()) { //abort all active _workers
          _itLocked = _workers.begin();
          do {
            {
              ReverseMutexGuard rGrd(_mutex);
              _itLocked->second->Abort(reason); //may set _itToErase
            }
            if (_itLocked == _itToErase) {
              ++_itLocked;
              _corpses.push_back(_itToErase->second);
              _workers.erase(_itToErase);
              _itToErase = _workers.end();
            } else
              ++_itLocked;
          } while (_itLocked != _workers.end());
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

