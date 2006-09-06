#include "SMPPSessionSwitch.hpp"
#include "SocketPool_Singleton.hpp"
#include "SessionCache.hpp"
#include "IdleSocketsPool.hpp"

#include <utility>
#include <netinet/in.h>
#include <util/Exception.hpp>

#include <logger/Logger.h>
extern smsc::logger::Logger* dmplxlog;

smpp_dmplx::SMPPSessionSwitch::search_result_t
smpp_dmplx::SMPPSessionSwitch::getSharedSessionToSMSC(const std::string& systemId)
{
  smsc_log_info(dmplxlog,"SMPPSessionSwitch::getSharedSessionToSMSC::: get shared session to SMSC for systemId=[%s]", systemId.c_str());
  mapRegistredSysIdToSmscSession_t::iterator
    sysIdToSmscSess_iter = _mapRegistredSysIdToSmscSession.find(systemId);
  if ( sysIdToSmscSess_iter == _mapRegistredSysIdToSmscSession.end() ) {
    smsc_log_info(dmplxlog,"SMPPSessionSwitch::getSharedSessionToSMSC::: session to SMSC with systemId=%s was not found", systemId.c_str());
    // Если для указанного значения systemId нет установленной сессии к SMSC,
    // то вернуть признак отсутствия активной сессии.
    return std::make_pair(false, SMPPSession());
  } else {
    smsc_log_info(dmplxlog,"SMPPSessionSwitch::getSharedSessionToSMSC::: session to SMSC was found. Session=[%s]", sysIdToSmscSess_iter->second.toString().c_str());
    return std::make_pair(true, sysIdToSmscSess_iter->second);
  }
}

void
smpp_dmplx::SMPPSessionSwitch::setSharedSessionToSMSC(SMPPSession& sessionToSMSC, const std::string& systemId)
{
  smsc_log_info(dmplxlog,"SMPPSessionSwitch::setSharedSessionToSMSC::: set shared session=[%s] to SMSC for systemId=[%s]", sessionToSMSC.toString().c_str(), systemId.c_str());
  mapRegistredSysIdToSmscSession_t::iterator
    sysIdToSmscSess_iter = _mapRegistredSysIdToSmscSession.find(systemId);
  if ( sysIdToSmscSess_iter == _mapRegistredSysIdToSmscSession.end() ) {
    // Если для указанного значения systemId нет установленной сессии к SMSC,
    // то создать связку systemId -- новая_сессия_к_SMSC.
    smsc_log_info(dmplxlog,"SMPPSessionSwitch::setSharedSessionToSMSC::: shared session to SMSC has set. Session dump=[%s]", sessionToSMSC.toString().c_str());
    _mapRegistredSysIdToSmscSession.insert(std::make_pair(systemId,sessionToSMSC));
  } else
    smsc_log_info(dmplxlog,"SMPPSessionSwitch::setSharedSessionToSMSC::: shared session to SMSC  already exists. Session dump=[%s]", sessionToSMSC.toString().c_str());
}

void
smpp_dmplx::SMPPSessionSwitch::dropSharedSessionToSMSC(const std::string& systemId)
{
  smsc_log_info(dmplxlog,"SMPPSessionSwitch::dropSharedSessionToSMSC::: drop session to SMSC for systemId=[%s]", systemId.c_str());
  mapRegistredSysIdToSmscSession_t::iterator
    sysIdToSmscSess_iter = _mapRegistredSysIdToSmscSession.find(systemId);
  if ( sysIdToSmscSess_iter != _mapRegistredSysIdToSmscSession.end() ) {
    // Удаляем связку systemId -- сессия_с_SMSC
    _mapRegistredSysIdToSmscSession.erase(sysIdToSmscSess_iter);
    // и выкашиваем из диспетчера все сессии с SME для указанного
    // значения systemId
    sysIdToSmeUsageHist_t::iterator sysIdToSmeUsageHist_iter
      = _sysIdToSmeUsageHist.find(systemId);
    if ( sysIdToSmeUsageHist_iter != _sysIdToSmeUsageHist.end() ) {
      // для каждой сессии с SME, связанной с уничтожаемой сессией с SMSC,
      // удаляем сокет с SME из пула сокетов, на которых ждем отстуствия активности 
      // в течение таймаута для посылки EnqLink
      for (smeSessionUsageHistogram_t::iterator smeSessIter = sysIdToSmeUsageHist_iter->second->begin(); smeSessIter != sysIdToSmeUsageHist_iter->second->end(); ++smeSessIter)
        IdleSocketsPool::getInstance().removeActiveSocket(smeSessIter->second.getSocketToPeer());

      delete sysIdToSmeUsageHist_iter->second;
      _sysIdToSmeUsageHist.erase(sysIdToSmeUsageHist_iter);
    }
  }
}

void
smpp_dmplx::SMPPSessionSwitch::addActiveSmeSession(SMPPSession& sessionFromSme)
{
  std::string systemId = sessionFromSme.getSystemId();
  smsc_log_info(dmplxlog,"SMPPSessionSwitch::addActiveSmeSessionForSysId::: add active session to SME into cache for systemId=[%s]. Dump of sesson to SME=[%s]", systemId.c_str(), sessionFromSme.toString().c_str());
  if ( _mapRegistredSysIdToSmscSession.find(systemId) != _mapRegistredSysIdToSmscSession.end() ) {
    sysIdToSmeUsageHist_t::iterator 
      sysIdToSmeUsageHist_iter = _sysIdToSmeUsageHist.find(systemId);
    if ( sysIdToSmeUsageHist_iter == _sysIdToSmeUsageHist.end() ) {
      std::pair<sysIdToSmeUsageHist_t::iterator, bool>
        insertResult = _sysIdToSmeUsageHist.insert(std::make_pair(systemId, new smeSessionUsageHistogram_t()));
      sysIdToSmeUsageHist_iter = insertResult.first;
    }
    sysIdToSmeUsageHist_iter->second->insert(std::make_pair(1,sessionFromSme));
    smsc_log_info(dmplxlog,"SMPPSessionSwitch::addActiveSmeSessionForSysId::: session to SME was added.");
  } else
    throw smsc::util::Exception("smpp_dmplx::SMPPSessionSwitch::setSessionsRelation::: sessionToSMSC doesn't exist for given systemId");
}

void
smpp_dmplx::SMPPSessionSwitch::removeSmeSession(SMPPSession& sessionFromSme)
{
  std::string systemId = sessionFromSme.getSystemId();
  smsc_log_info(dmplxlog,"SMPPSessionSwitch::removeSmeSession::: remove session to SME=[%s]", sessionFromSme.toString().c_str());
  _sysIdToSmeUsageHist.find(systemId);

  sysIdToSmeUsageHist_t::iterator 
    sysIdToSmeUsageHist_iter = _sysIdToSmeUsageHist.find(systemId);

  if (  sysIdToSmeUsageHist_iter != _sysIdToSmeUsageHist.end() ) {
    smeSessionUsageHistogram_t* smeSessionUsageHistogram = sysIdToSmeUsageHist_iter->second;
    for(smeSessionUsageHistogram_t::iterator iter = smeSessionUsageHistogram->begin(); iter != smeSessionUsageHistogram->end(); ++iter) {
      if ( iter->second == sessionFromSme ) {
        smeSessionUsageHistogram->erase(iter);
        break;
      }
    }
    // Проверяем, что удаленная сессия с SME была последней активной сессией
    // для заданного systemId.
    if ( smeSessionUsageHistogram->empty() ) {
      // Если это так, то уничтожаем сессию с SMSC для заданного systemId.
      mapRegistredSysIdToSmscSession_t::iterator smsc_searchRes = _mapRegistredSysIdToSmscSession.find(systemId);
      if ( smsc_searchRes != _mapRegistredSysIdToSmscSession.end() ) {
        // Удаляем соскет для работы с SMSC из пула сокетов
        smsc::core_ax::network::Socket& socketToSmsc = smsc_searchRes->second.getSocketToPeer();
        SocketPool_Singleton::getInstance().remove_socket(socketToSmsc);
        // Удаляем сессию для сокета с SMSC.
        SessionCache::getInstance().removeSession(socketToSmsc);
        _mapRegistredSysIdToSmscSession.erase(smsc_searchRes);
        // И удаляем сокет с SMSC из списка сокетов, по которым посылается EnquireLink
        smpp_dmplx::IdleSocketsPool::getInstance().removeActiveSocket(socketToSmsc);
      }
    }
  }
}

smpp_dmplx::SMPPSessionSwitch::search_result_t
smpp_dmplx::SMPPSessionSwitch::getCrossedSession(SMPPSession& session)
{
  smsc_log_info(dmplxlog,"SMPPSessionSwitch::getCrossedSession::: get session to SMSC for session=[%s].",session.toString().c_str());
  mapRegistredSysIdToSmscSession_t::iterator
    sysIdToSmscSess_iter = _mapRegistredSysIdToSmscSession.find(session.getSystemId());

  if ( sysIdToSmscSess_iter != _mapRegistredSysIdToSmscSession.end() &&
       sysIdToSmscSess_iter->second != session ) {
    smsc_log_info(dmplxlog,"SMPPSessionSwitch::getCrossedSession::: session to SMSC was found. Session dump=[%s]", sysIdToSmscSess_iter->second.toString().c_str());
    return std::make_pair(true, sysIdToSmscSess_iter->second);
  } else {
    smsc_log_info(dmplxlog,"SMPPSessionSwitch::getCrossedSession::: session to SMSC was not found.");
    return std::make_pair(false, SMPPSession());
  }
}

void 
smpp_dmplx::SMPPSessionSwitch::dumpRegistredSmeSession(const char* where, smeSessionUsageHistogram_t* sessionUsageHist)
{
  for (smeSessionUsageHistogram_t::iterator iter=sessionUsageHist->begin();
       iter != sessionUsageHist->end(); ++iter)
    smsc_log_debug(dmplxlog,"%s exists session=[%s] with usage counter=%d", where, iter->second.toString().c_str(), iter->first);
}

smpp_dmplx::SMPPSessionSwitch::search_result_t
smpp_dmplx::SMPPSessionSwitch::getCrossedSession(const std::string& systemId)
{
  smsc_log_info(dmplxlog,"SMPPSessionSwitch::getCrossedSession::: get session to SME for systemId=[%s]", systemId.c_str());
  sysIdToSmeUsageHist_t::iterator
    sysIdToSmeUsage_iter = _sysIdToSmeUsageHist.find(systemId);
  if ( sysIdToSmeUsage_iter != _sysIdToSmeUsageHist.end() ) {
    smeSessionUsageHistogram_t* sessionUsageHist = sysIdToSmeUsage_iter->second;
    smeSessionUsageHistogram_t::iterator
      smeSessionUsageHist_iter = sessionUsageHist->begin();
    // for debug only
    dumpRegistredSmeSession("SMPPSessionSwitch::getCrossedSession:::",sessionUsageHist);
    if ( smeSessionUsageHist_iter != sessionUsageHist->end() ) {
      std::pair<int,SMPPSession> entry = *smeSessionUsageHist_iter;
      sessionUsageHist->erase(smeSessionUsageHist_iter);
      ++entry.first;
      sessionUsageHist->insert(entry);
      smsc_log_info(dmplxlog, "smpp_dmplx::SMPPSessionSwitch::getCrossedSession::: active session to SME was found. Session dump=[%s]", entry.second.toString().c_str());
      return std::make_pair(true,entry.second);
    } else {
      smsc_log_info(dmplxlog, "smpp_dmplx::SMPPSessionSwitch::getCrossedSession::: Haven't active session to SME for systemId=[%s]", systemId.c_str());
      return std::make_pair(false,SMPPSession());
    }
  } else {
    smsc_log_info(dmplxlog, "smpp_dmplx::SMPPSessionSwitch::getCrossedSession::: Haven't active session to SME for systemId=[%s]", systemId.c_str());
    return std::make_pair(false,SMPPSession());
  }
}

