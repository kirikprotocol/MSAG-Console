#include "CacheOfPendingBindReqFromSME.hpp"
#include "PendingOutDataQueue.hpp"
#include "SMPP_Constants.hpp"

#include <logger/Logger.h>
extern smsc::logger::Logger* dmplxlog;

void
smpp_dmplx::CacheOfPendingBindReqFromSME::putPendingReqToCache(SMPPSession& smppSessionToSME, SMPP_BindRequest* bindRequest)
{
  std::string systemId = smppSessionToSME.getSystemId();
  smsc_log_info(dmplxlog,"CacheOfPendingBindReqFromSME::putPendingReqToCache::: put BindRequest from SME [sme session=[%s]] to cache", smppSessionToSME.toString().c_str());

  sysid_to_cache_of_requests_map_t::iterator
    map_iter = map_sysIdToRequestCache.find(systemId);

  if ( map_iter == map_sysIdToRequestCache.end() ) {
    std::pair<sysid_to_cache_of_requests_map_t::iterator, bool> 
      insert_res = map_sysIdToRequestCache.insert(std::make_pair(systemId, new list_of_cached_request_t()));
    map_iter = insert_res.first;
  }

  map_iter->second->push_back(std::make_pair(smppSessionToSME, bindRequest));
}

void
smpp_dmplx::CacheOfPendingBindReqFromSME::commitPendingReqInCache(const std::string& systemId, const SMPP_BindResponse& bindResp)
{
  smsc_log_info(dmplxlog,"CacheOfPendingBindReqFromSME::commitPendingReqInCache::: Try find cached bind requests from another ESME for system id=%s", systemId.c_str());

  sysid_to_cache_of_requests_map_t::iterator
    map_iter = map_sysIdToRequestCache.find(systemId);

  if ( map_iter != map_sysIdToRequestCache.end() ) {
    // Если для systemId полученного в запросе BindRequest в очереди есть 
    // неподтвержденные запросы BindRequest
    list_of_cached_request_t* list_of_cached_req = map_iter->second;
    list_of_cached_request_t::iterator list_req_iter = list_of_cached_req->begin();
    // То для каждого неподтвержденного запроса
    while ( !list_of_cached_req->empty() ) {
      cached_request_t cached_value = list_of_cached_req->front();

      // Создаем копию ответа BindResponse, полученного от SMSC
      std::auto_ptr<SMPP_BindResponse> sendingBindResponse = bindResp.makeCopy();
      // и заменяем в копии sequenceNumber на значение, полученное в запросе 
      // BindRequest от SME.
      sendingBindResponse->setSequenceNumber(cached_value.second->getSequenceNumber());
      std::auto_ptr<BufferedOutputStream>
        bufToWriteForSme = sendingBindResponse->marshal();

      // послали ответ в SME
      smsc_log_info(dmplxlog,"CacheOfPendingBindReqFromSME::commitPendingReqInCache::: send BIND_RESPONSE message to SME over socket=[%s]. Message dump=[%s]", cached_value.first.getSocketToPeer().toString().c_str(), sendingBindResponse->toString().c_str());
      PendingOutDataQueue::scheduleDataForSending(*bufToWriteForSme,cached_value.first.getSocketToPeer());
      if ( bindResp.getCommandStatus() != ESME_ROK )
        cached_value.first.updateSessionState(SMPPSession::GOT_NEGATIVE_BIND_RESP);
      else
        cached_value.first.updateSessionState(SMPPSession::GOT_BIND_RESP);
      // удаляем из очереди запрос от SME
      list_of_cached_req->pop_front();
      // удаляем собственно объект-запрос
      delete cached_value.second;
    }
  }
}
