#ifndef __SMPPDMPLX_CACHEOFSMPP_MESSAGE_TRACES_HPP__
# define __SMPPDMPLX_CACHEOFSMPP_MESSAGE_TRACES_HPP__ 1

# include <sys/types.h>
# include <string>
# include <map>
# include <utility>

# include <util/Singleton.hpp>
# include "SMPPSession.hpp"

namespace smpp_dmplx {

class CacheOfSMPP_message_traces : public smsc::util::Singleton<CacheOfSMPP_message_traces> {
public:
  /*
  ** ћетод по значению sequenceNumber, полученному от SME и значению systemId
  ** дл€ вход€щей сессии от SME, получает  новый sequenceNumber дл€ запросов,
  ** передаваемых в SMSC. —есси€, переданна€ в аргументе, сохран€етс€ в кеше.
  ** Ќовый sequenceNumber возвращаетс€ в виде результата метода.
  */
  uint32_t
  putMessageTraceToCache(uint32_t origSeqNum, const SMPPSession& sessionFromSME);

  /*
  ** ѕервый элемент пары - seqNum от SME, второй элемент пары - сесси€
  ** от которой был получен оригинальный запрос.
  */
  typedef std::pair<uint32_t, SMPPSession> MessageTrace_t;

  /*
  ** ћетод по значению sequenceNumber, полученному от SMSC и значению systemId
  ** дл€ сессии с SMSC, возвращает из кеша пару
  ** <исходное_значение_sequenceNumber,сесси€_с_SME>
  */
  MessageTrace_t
  getMessageTraceFromCache(uint32_t substSeqNum, const std::string& systemId);
private:
  typedef std::pair<uint32_t, std::string> SeqNumSessId_pair_t;

  typedef std::map<SeqNumSessId_pair_t,
                   MessageTrace_t> CacheType;
  CacheType mapper;
};

}

#endif
