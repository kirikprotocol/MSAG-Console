#include "CacheOfSMPP_message_traces.hpp"
#include "SequenceNumberGenerator.hpp"
#include <util/Exception.hpp>

#include <logger/Logger.h>
extern smsc::logger::Logger* dmplxlog;

uint32_t
smpp_dmplx::CacheOfSMPP_message_traces::putMessageTraceToCache(uint32_t origSeqNum, const SMPPSession& sessionFromSME)
{
  std::string systemId = sessionFromSME.getSystemId();
  uint32_t nextSeq = SequenceNumberGenerator::getInstance(systemId)->getNextValue();
  MessageTrace_t whatCaching(origSeqNum, sessionFromSME);
  SeqNumSessId_pair_t surrogateKey(nextSeq, systemId);

  smsc_log_info(dmplxlog,"CacheOfSMPP_message_traces::putMessageTraceToCache::: It is replacing origSeqNum=%d from session to SME=[%s] to monotonous increasing seq.num.=%d for session to SMSC", origSeqNum, sessionFromSME.toString().c_str(), nextSeq);
  smsc_log_info(dmplxlog,"CacheOfSMPP_message_traces::putMessageTraceToCache::: put socked=[%s] to cache", sessionFromSME.getSocketToPeer().toString().c_str());
  mapper[surrogateKey]=whatCaching;

  return nextSeq;
}

smpp_dmplx::CacheOfSMPP_message_traces::MessageTrace_t
smpp_dmplx::CacheOfSMPP_message_traces::getMessageTraceFromCache(uint32_t substSeqNum, const std::string& systemId)
{
  SeqNumSessId_pair_t surrogateKey(substSeqNum, systemId);
  smsc_log_info(dmplxlog,"CacheOfSMPP_message_traces::getMessageTraceFromCache::: It is finding origSeqNum by seq.num.=%d getting from SMSC for session with systemId=%s", substSeqNum, systemId.c_str());
  CacheType::iterator iter = mapper.find(surrogateKey);
  if ( iter != mapper.end() ) {
    MessageTrace_t retValue = iter->second;
    smsc_log_info(dmplxlog,"CacheOfSMPP_message_traces::getMessageTraceFromCache::: original seq. num.=%d was found, socket to sme=[%s]", iter->second.first, iter->second.second.getSocketToPeer().toString().c_str());
    mapper.erase(iter);
    return retValue;
  } else
    throw smsc::util::Exception("smpp_dmplx::CacheOfSMPP_message_traces::getMessageTraceFromCache:: Cached element was not found");
}
