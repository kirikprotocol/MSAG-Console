#include "CacheOfPendingBindReqFromSME.hpp"
#include "PendingOutDataQueue.hpp"
#include "SMPP_Constants.hpp"

namespace smpp_dmplx {

CacheOfPendingBindReqFromSME::CacheOfPendingBindReqFromSME()
  : _log(smsc::logger::Logger::getInstance("bindcache")) {}

void
CacheOfPendingBindReqFromSME::putPendingReqToCache(SMPPSession& smppSessionToSME, SMPP_BindRequest* bindRequest)
{
  std::string systemId = smppSessionToSME.getSystemId();
  smsc_log_info(_log,"CacheOfPendingBindReqFromSME::putPendingReqToCache::: put BindRequest from SME [sme session=[%s]] to cache", smppSessionToSME.toString().c_str());

  sysid_to_cache_of_requests_map_t::iterator
    map_iter = _map_sysIdToRequestCache.find(systemId);

  if ( map_iter == _map_sysIdToRequestCache.end() ) {
    std::pair<sysid_to_cache_of_requests_map_t::iterator, bool> 
      insert_res = _map_sysIdToRequestCache.insert(std::make_pair(systemId, new list_of_cached_request_t()));
    map_iter = insert_res.first;
  }

  map_iter->second->push_back(std::make_pair(smppSessionToSME, bindRequest));
}

void
CacheOfPendingBindReqFromSME::commitPendingReqInCache(const std::string& systemId, const SMPP_BindResponse& bindResp)
{
  smsc_log_info(_log,"CacheOfPendingBindReqFromSME::commitPendingReqInCache::: Try find cached bind requests from another ESME for system id=%s", systemId.c_str());

  sysid_to_cache_of_requests_map_t::iterator
    map_iter = _map_sysIdToRequestCache.find(systemId);

  if ( map_iter != _map_sysIdToRequestCache.end() ) {
    // ���� ��� systemId ����������� � ������� BindRequest � ������� ���� 
    // ���������������� ������� BindRequest
    list_of_cached_request_t* list_of_cached_req = map_iter->second;
    // �� ��� ������� ����������������� �������
    while ( !list_of_cached_req->empty() ) {
      cached_request_t cached_value = list_of_cached_req->front();

      // ������� ����� ������ BindResponse, ����������� �� SMSC
      std::auto_ptr<SMPP_BindResponse> sendingBindResponse = bindResp.makeCopy();
      // � �������� � ����� sequenceNumber �� ��������, ���������� � ������� 
      // BindRequest �� SME.
      sendingBindResponse->setSequenceNumber(cached_value.second->getSequenceNumber());
      std::auto_ptr<BufferedOutputStream>
        bufToWriteForSme = sendingBindResponse->marshal();

      // ������� ����� � SME
      smsc_log_info(_log,"CacheOfPendingBindReqFromSME::commitPendingReqInCache::: send BIND_RESPONSE message to SME over socket=[%s]. Message dump=[%s]", cached_value.first.getSocketToPeer().toString().c_str(), sendingBindResponse->toString().c_str());
      PendingOutDataQueue::getInstance().scheduleDataForSending(*bufToWriteForSme,cached_value.first.getSocketToPeer());
      if ( bindResp.getCommandStatus() != ESME_ROK )
        cached_value.first.updateSessionState(SMPPSession::GOT_NEGATIVE_BIND_RESP);
      else
        cached_value.first.updateSessionState(SMPPSession::GOT_BIND_RESP);
      // ������� �� ������� ������ �� SME
      list_of_cached_req->pop_front();
      // ������� ���������� ������-������
      delete cached_value.second;
    }

    delete map_iter->second;
    _map_sysIdToRequestCache.erase(map_iter);
  }
}

}
