#ifndef __SMPPDMPLX_CACHEOFSMPP_MESSAGE_TRACES_HPP__
# define __SMPPDMPLX_CACHEOFSMPP_MESSAGE_TRACES_HPP__

# include <sys/types.h>
# include <string>
# include <map>
# include <utility>

# include <logger/Logger.h>
# include <util/Singleton.hpp>
# include <smppdmplx/SMPPSession.hpp>
# include <smppdmplx/core_ax/network/Socket.hpp>

namespace smpp_dmplx {

class CacheOfSMPP_message_traces : public smsc::util::Singleton<CacheOfSMPP_message_traces> {
public:
  CacheOfSMPP_message_traces();
  /*
  ** ����� �� �������� sequenceNumber, ����������� �� SME � �������� systemId
  ** ��� �������� ������ �� SME, ��������  ����� sequenceNumber ��� ��������,
  ** ������������ � SMSC. ������, ���������� � ���������, ����������� � ����.
  ** ����� sequenceNumber ������������ � ���� ���������� ������.
  */
  void putMessageTraceToCache(uint32_t origSeqNum, const SMPPSession& sessionFromSME, uint32_t substSeqNum);

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

  void removeMessageTracesFromCache(const smsc::core_ax::network::Socket& socketToSme);

private:
  typedef std::pair<uint32_t, std::string> SeqNumSessId_pair_t;

  typedef std::map<SeqNumSessId_pair_t,
                   MessageTrace_t> CacheType;
  CacheType _mapper;
  smsc::logger::Logger* _log;
};

class MessageTraceNotFound : public smsc::util::Exception {
public:
  explicit MessageTraceNotFound(const char* what) : Exception(what) {}
};

}

#endif
