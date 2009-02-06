#include "CacheOfSMPP_message_traces.hpp"
#include <util/Exception.hpp>

namespace smpp_dmplx {

CacheOfSMPP_message_traces::CacheOfSMPP_message_traces()
  : _log(smsc::logger::Logger::getInstance("msg_hndlr")) {}

void
CacheOfSMPP_message_traces::putMessageTraceToCache(uint32_t origSeqNum, const SMPPSession& sessionFromSME, uint32_t substSeqNum)
{
  std::string systemId = sessionFromSME.getSystemId();
  MessageTrace_t whatCaching(origSeqNum, sessionFromSME);
  SeqNumSessId_pair_t surrogateKey(substSeqNum, systemId);

  smsc_log_info(_log,"CacheOfSMPP_message_traces::putMessageTraceToCache::: It is replacing origSeqNum=%d from session to SME=[%s] to monotonous increasing seq.num.=%d for session to SMSC", origSeqNum, sessionFromSME.toString().c_str(), substSeqNum);
  smsc_log_info(_log,"CacheOfSMPP_message_traces::putMessageTraceToCache::: put socked=[%s] to cache", sessionFromSME.getSocketToPeer().toString().c_str());
  _mapper[surrogateKey]=whatCaching;
}

CacheOfSMPP_message_traces::MessageTrace_t
CacheOfSMPP_message_traces::getMessageTraceFromCache(uint32_t substSeqNum, const std::string& systemId)
{
  SeqNumSessId_pair_t surrogateKey(substSeqNum, systemId);
  smsc_log_info(_log,"CacheOfSMPP_message_traces::getMessageTraceFromCache::: It is finding origSeqNum by seq.num.=%d getting from SMSC for session with systemId=%s", substSeqNum, systemId.c_str());
  CacheType::iterator iter = _mapper.find(surrogateKey);
  if ( iter != _mapper.end() ) {
    MessageTrace_t retValue = iter->second;
    smsc_log_info(_log,"CacheOfSMPP_message_traces::getMessageTraceFromCache::: original seq. num.=%d was found, socket to sme=[%s]", iter->second.first, iter->second.second.getSocketToPeer().toString().c_str());
    _mapper.erase(iter);
    return retValue;
  } else
    throw MessageTraceNotFound("smpp_dmplx::CacheOfSMPP_message_traces::getMessageTraceFromCache:: Cached element was not found");
}

void
CacheOfSMPP_message_traces::removeMessageTracesFromCache(const smsc::core_ax::network::Socket& socketToSme)
{
  smsc_log_info(_log,"CacheOfSMPP_message_traces::removeMessageTracesFromCache::: try remove messages traces for socket=[%s]", socketToSme.toString().c_str());
  unsigned count=0;
  for (CacheType::iterator iter = _mapper.begin(); iter != _mapper.end();) {
    MessageTrace_t& messageTrace = iter->second;
    if ( messageTrace.second.getSocketToPeer() == socketToSme ) {
      _mapper.erase(iter++);
      ++count;
    } else
      ++iter;
  }
  if ( count )
    smsc_log_info(_log,"CacheOfSMPP_message_traces::removeMessageTracesFromCache::: removed %d messages traces for socket=[%s]", count, socketToSme.toString().c_str());
}

}
