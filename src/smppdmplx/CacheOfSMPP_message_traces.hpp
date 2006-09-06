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
  ** ����� �� �������� sequenceNumber, ����������� �� SME � �������� systemId
  ** ��� �������� ������ �� SME, ��������  ����� sequenceNumber ��� ��������,
  ** ������������ � SMSC. ������, ���������� � ���������, ����������� � ����.
  ** ����� sequenceNumber ������������ � ���� ���������� ������.
  */
  uint32_t
  putMessageTraceToCache(uint32_t origSeqNum, const SMPPSession& sessionFromSME);

  /*
  ** ������ ������� ���� - seqNum �� SME, ������ ������� ���� - ������
  ** �� ������� ��� ������� ������������ ������.
  */
  typedef std::pair<uint32_t, SMPPSession> MessageTrace_t;

  /*
  ** ����� �� �������� sequenceNumber, ����������� �� SMSC � �������� systemId
  ** ��� ������ � SMSC, ���������� �� ���� ����
  ** <��������_��������_sequenceNumber,������_�_SME>
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
