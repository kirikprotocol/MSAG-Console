#include "SMPPSessionSwitch.hpp"
#include "SocketPool_Singleton.hpp"
#include "SessionCache.hpp"
#include "IdleSocketsPool.hpp"
#include "PendingOutDataQueue.hpp"
#include "CacheOfIncompleteReadMessages.hpp"

#include <utility>
#include <netinet/in.h>
#include <util/Exception.hpp>

namespace smpp_dmplx {

SMPPSessionSwitch::SMPPSessionSwitch()
  : _log(smsc::logger::Logger::getInstance("sessswtch")) {}

SMPPSessionSwitch::search_result_t
SMPPSessionSwitch::getSharedSessionToSMSC(const std::string& systemId)
{
  smsc_log_info(_log,"SMPPSessionSwitch::getSharedSessionToSMSC::: get shared session to SMSC for systemId=[%s]", systemId.c_str());
  mapRegistredSysIdToSmscSession_t::iterator
    sysIdToSmscSess_iter = _mapRegistredSysIdToSmscSession.find(systemId);
  if ( sysIdToSmscSess_iter == _mapRegistredSysIdToSmscSession.end() ) {
    smsc_log_info(_log,"SMPPSessionSwitch::getSharedSessionToSMSC::: session to SMSC with systemId=%s was not found", systemId.c_str());
    // Если для указанного значения systemId нет установленной сессии к SMSC,
    // то вернуть признак отсутствия активной сессии.
    return std::make_pair(false, SMPPSession());
  } else {
    smsc_log_info(_log,"SMPPSessionSwitch::getSharedSessionToSMSC::: session to SMSC was found. Session=[%s]", sysIdToSmscSess_iter->second.toString().c_str());
    return std::make_pair(true, sysIdToSmscSess_iter->second);
  }
}

void
SMPPSessionSwitch::setSharedSessionToSMSC(SMPPSession& sessionToSMSC,
                                          const std::string& systemId)
{
  smsc_log_info(_log,"SMPPSessionSwitch::setSharedSessionToSMSC::: set shared session=[%s] to SMSC for systemId=[%s]", sessionToSMSC.toString().c_str(), systemId.c_str());
  mapRegistredSysIdToSmscSession_t::iterator
    sysIdToSmscSess_iter = _mapRegistredSysIdToSmscSession.find(systemId);
  if ( sysIdToSmscSess_iter == _mapRegistredSysIdToSmscSession.end() ) {
    // Если для указанного значения systemId нет установленной сессии к SMSC,
    // то создать связку systemId -- новая_сессия_к_SMSC.
    smsc_log_info(_log,"SMPPSessionSwitch::setSharedSessionToSMSC::: shared session to SMSC has set. Session dump=[%s]", sessionToSMSC.toString().c_str());
    _mapRegistredSysIdToSmscSession.insert(std::make_pair(systemId,sessionToSMSC));
  } else {
    smsc_log_error(_log,"SMPPSessionSwitch::setSharedSessionToSMSC::: shared session to SMSC  already exists. Session dump=[%s]", sessionToSMSC.toString().c_str());
    throw smsc::util::Exception("SMPPSessionSwitch::setSharedSessionToSMSC::: failed set shared session to SMSC");
  }
}

void
SMPPSessionSwitch::dropSharedSessionToSMSC(const std::string& systemId)
{
  smsc_log_info(_log,"SMPPSessionSwitch::dropSharedSessionToSMSC::: drop session to SMSC for systemId=[%s]", systemId.c_str());
  mapRegistredSysIdToSmscSession_t::iterator
    sysIdToSmscSess_iter = _mapRegistredSysIdToSmscSession.find(systemId);
  if ( sysIdToSmscSess_iter != _mapRegistredSysIdToSmscSession.end() ) {
    // Удаляем связку systemId -- сессия_с_SMSC
    _mapRegistredSysIdToSmscSession.erase(sysIdToSmscSess_iter);
    // и выкашиваем из диспетчера все сессии с SME для указанного
    // значения systemId
    sysIdToSmeList_t::iterator sysIdToSmeList_iter
      = _sysIdToSmeList.find(systemId);
    if ( sysIdToSmeList_iter != _sysIdToSmeList.end() ) {
      // для каждой сессии с SME, связанной с уничтожаемой сессией с SMSC,
      // удаляем сокет с SME из пула сокетов, на которых ждем отстуствия активности 
      // в течение таймаута для посылки EnqLink
      for (smeSessions_t::iterator smeSessIter = sysIdToSmeList_iter->second->begin(); smeSessIter != sysIdToSmeList_iter->second->end(); ++smeSessIter)
        IdleSocketsPool::getInstance().removeActiveSocket(smeSessIter->getSocketToPeer());

      delete sysIdToSmeList_iter->second;
      _sysIdToSmeList.erase(sysIdToSmeList_iter);
    }
  }
}

void
SMPPSessionSwitch::addActiveSmeSession(SMPPSession& sessionFromSme)
{
  std::string systemId = sessionFromSme.getSystemId();
  smsc_log_info(_log,"SMPPSessionSwitch::addActiveSmeSession::: add active session to SME into cache for systemId=[%s]. Dump of sesson to SME=[%s]", systemId.c_str(), sessionFromSme.toString().c_str());
  if ( _mapRegistredSysIdToSmscSession.find(systemId) != _mapRegistredSysIdToSmscSession.end() ) {
    sysIdToSmeList_t::iterator 
      iter = _sysIdToSmeList.find(systemId);
    if ( iter == _sysIdToSmeList.end() ) {
      std::pair<sysIdToSmeList_t::iterator, bool>
        insertResult = _sysIdToSmeList.insert(std::make_pair(systemId, new smeSessions_t()));
      iter = insertResult.first;
    }
    {
      unsigned numOfDuplicatedSession = 0;
      for(smeSessions_t::iterator smeSessIter = iter->second->begin(), smeSessEndIter = iter->second->end();
          smeSessIter != smeSessEndIter; ++smeSessIter)
        if ( *smeSessIter == sessionFromSme )
          ++numOfDuplicatedSession;
      if ( numOfDuplicatedSession ) {
        smsc_log_error(_log,"SMPPSessionSwitch::addActiveSmeSession::: there is duplicate session for [%s]", sessionFromSme.toString().c_str());
        abort();
      }
    }
    iter->second->push_back(sessionFromSme);
    smsc_log_info(_log,"SMPPSessionSwitch::addActiveSmeSession::: session to SME was added.");
  } else
    throw smsc::util::Exception("SMPPSessionSwitch::addActiveSmeSession::: sessionToSMSC doesn't exist for given systemId");
}

void
SMPPSessionSwitch::removeSmeSession(SMPPSession& sessionFromSme)
{
  std::string systemId = sessionFromSme.getSystemId();
  smsc_log_info(_log,"SMPPSessionSwitch::removeSmeSession::: remove session to SME=[%s]", sessionFromSme.toString().c_str());

  sysIdToSmeList_t::iterator 
    sysIdToSmeList_iter = _sysIdToSmeList.find(systemId);

  if (  sysIdToSmeList_iter != _sysIdToSmeList.end() ) {
    smeSessions_t* smeSessions = sysIdToSmeList_iter->second;
    for(smeSessions_t::iterator iter = smeSessions->begin(); iter != smeSessions->end(); ++iter) {
      if ( *iter == sessionFromSme ) {
        smsc_log_info(_log,"SMPPSessionSwitch::removeSmeSession::: session to SME=[%s] has been removed", sessionFromSme.toString().c_str());
        smeSessions->erase(iter);
        break;
      }
    }
    // Проверяем, что удаленная сессия с SME была последней активной сессией
    // для заданного systemId.
    if ( smeSessions->empty() ) {
      // Если это так, то уничтожаем сессию с SMSC для заданного systemId.
      mapRegistredSysIdToSmscSession_t::iterator smsc_searchRes = _mapRegistredSysIdToSmscSession.find(systemId);
      if ( smsc_searchRes != _mapRegistredSysIdToSmscSession.end() ) {
        smsc_log_info(_log,"SMPPSessionSwitch::removeSmeSession::: session to SME=[%s] is last session from group with the same systemId value, remove corresponding session to SMSC", sessionFromSme.toString().c_str());
        // Удаляем соскет для работы с SMSC из пула сокетов
        smsc::core_ax::network::Socket& socketToSmsc = smsc_searchRes->second.getSocketToPeer();
        SocketPool_Singleton::getInstance().remove_socket(socketToSmsc);
        // Удаляем сессию для сокета с SMSC.
        SessionCache::getInstance().removeSession(socketToSmsc);

        _mapRegistredSysIdToSmscSession.erase(smsc_searchRes);
        smsc_log_info(_log,"SMPPSessionSwitch::removeSmeSession::: session to SMSC has been removed");
        // И удаляем сокет с SMSC из списка сокетов, по которым посылается EnquireLink
        IdleSocketsPool::getInstance().removeActiveSocket(socketToSmsc);
        PendingOutDataQueue::getInstance().cancelScheduledData(socketToSmsc);
        CacheOfIncompleteReadMessages::getInstance().removeCompleteMessageForSocket(socketToSmsc);
        //        CacheOfIncompleteReadMessages::
        smsc_log_info(_log,"SMPPSessionSwitch::removeSmeSession::: session to SMSC for connection [%s] has been removed", socketToSmsc.toString().c_str());
      }
    }
  }
}

SMPPSessionSwitch::search_result_t
SMPPSessionSwitch::getCrossedSession(SMPPSession& session)
{
  smsc_log_info(_log,"SMPPSessionSwitch::getCrossedSession::: get session to SMSC for session=[%s].",session.toString().c_str());
  mapRegistredSysIdToSmscSession_t::iterator
    sysIdToSmscSess_iter = _mapRegistredSysIdToSmscSession.find(session.getSystemId());

  if ( sysIdToSmscSess_iter != _mapRegistredSysIdToSmscSession.end() &&
       sysIdToSmscSess_iter->second != session ) {
    smsc_log_info(_log,"SMPPSessionSwitch::getCrossedSession::: session to SMSC was found. Session dump=[%s]", sysIdToSmscSess_iter->second.toString().c_str());
    return std::make_pair(true, sysIdToSmscSess_iter->second);
  } else {
    smsc_log_info(_log,"SMPPSessionSwitch::getCrossedSession::: session to SMSC was not found.");
    return std::make_pair(false, SMPPSession());
  }
}

SMPPSessionSwitch::search_result_t
SMPPSessionSwitch::getCrossedSession(const std::string& systemId)
{
  smsc_log_info(_log,"SMPPSessionSwitch::getCrossedSession::: get session to SME for systemId=[%s]", systemId.c_str());
  sysIdToSmeList_t::iterator
    sysIdToSmeList_iter = _sysIdToSmeList.find(systemId);
  if ( sysIdToSmeList_iter != _sysIdToSmeList.end() ) {
    smeSessions_t* smeSessions = sysIdToSmeList_iter->second;

    if ( !smeSessions->empty() ) {
      smeSessions_t::iterator smeSessions_iter = smeSessions->begin();
      SMPPSession sessionToSme = *smeSessions_iter;
      smeSessions->splice(smeSessions->end(), *smeSessions, smeSessions_iter);
      smsc_log_info(_log, "SMPPSessionSwitch::getCrossedSession::: active session to SME was found. Session dump=[%s]", sessionToSme.toString().c_str());
      return std::make_pair(true,sessionToSme);
    } else {
      smsc_log_info(_log, "SMPPSessionSwitch::getCrossedSession::: Haven't active session to SME for systemId=[%s]", systemId.c_str());
      return std::make_pair(false,SMPPSession());
    }
  } else {
    smsc_log_info(_log, "SMPPSessionSwitch::getCrossedSession::: Haven't active session to SME for systemId=[%s]", systemId.c_str());
    return std::make_pair(false,SMPPSession());
  }
}

void
SMPPSessionSwitch::broadcastMessageToAllSme(const std::string& systemId,
                                            const SMPP_message& messageToSend)
{
  sysIdToSmeList_t::iterator sysIdToSmeSessions_iter
    = _sysIdToSmeList.find(systemId);
  if ( sysIdToSmeSessions_iter != _sysIdToSmeList.end() ) {
    for (smeSessions_t::iterator smeSessIter = sysIdToSmeSessions_iter->second->begin(); smeSessIter != sysIdToSmeSessions_iter->second->end(); ++smeSessIter) {
      smsc_log_info(_log,"SMPPSessionSwitch::broadcastMessageToAllSme::: send message to session [%s]", smeSessIter->toString().c_str());
      std::auto_ptr<BufferedOutputStream> outBuf = messageToSend.marshal();
      PendingOutDataQueue::getInstance().scheduleDataForSending(*outBuf, smeSessIter->getSocketToPeer());
    }
  }
}

}
