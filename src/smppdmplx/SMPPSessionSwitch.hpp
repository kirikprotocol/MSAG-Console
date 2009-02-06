#ifndef __SMPPDMPLX_SMPPSESSIONSWITCH_HPP__
# define __SMPPDMPLX_SMPPSESSIONSWITCH_HPP__

/*
** ��������������� ������-
** 1. �������� ������ systemId -- ������_�_SMSC
** 2. ������� ������_�_SMSC �� ������� �� ��������� �������� systemId
** 3. �������� ������ smpp_������_c_SME -- smpp_������_�_SMC
** 4. ������� �� ������� ����������� ������ � SMSC. � �������
**    ����������� ������, ������������� ������� �� SME.
** 5. �������� ������ smpp_������_c_SME -- smpp_������_�_SMC.
**    ��� ��������� ��������� ������ smpp_������_c_SME -- smpp_������_�_SMC 
**    ��� ������ ������_�_SMC, ������������ ���������� � SMC 
**    ������ �����������.
** 6. ������� ������ ��� ������ � SME (�����_SME) �� ������ � �������
**    ������ � SMSC. ������ � SME ����� ������������
**    � ������������ �� ���������� round-robin, ��� ���-�� ���.
*/

# include <map>
# include <list>
# include <utility>

# include <logger/Logger.h>
# include <util/Singleton.hpp>
# include <smppdmplx/SMPPSession.hpp>
# include <smppdmplx/SMPP_message.hpp>
# include <smppdmplx/core_ax/network/Socket.hpp>

namespace smpp_dmplx {

class SMPPSessionSwitch : public smsc::util::Singleton<SMPPSessionSwitch> {
public:
  SMPPSessionSwitch();

  typedef std::pair<bool,SMPPSession> search_result_t;

  /*
  **  �������� ����������� ������ � SMSC ��� ��� ��������� �������� 
  **  systemId. ���� ����� ������ ����������, �� ������� first ������
  **  std::pair<bool,SMPPSession> ��������� �������� true.
  */
  search_result_t getSharedSessionToSMSC(const std::string& systemId);

  /*
  **  ������� ����������� ������ � SMSC ��� ��������� �������� 
  **  systemId.
  */
  void setSharedSessionToSMSC(SMPPSession& sessionToSMSC, const std::string& systemId);

  /*
  **  ������� ����������� ������ �� ���� ��� ��������� �������� systemId.
  */
  void dropSharedSessionToSMSC(const std::string& systemId);

  /*
  ** �������� � ��� ������_�_SME
  */
  void addActiveSmeSession(SMPPSession& sessionFromSme);

  /*
  ** ������� ������ � SME �� ����
  */
  void removeSmeSession(SMPPSession& sessionFromSme);

  /*
  ** �������� ����������� ������ � SMSC. � ������� ���������� ������ � SME.
  ** ���� � ������������ ���� first == true, �� � second �������� ������, �����
  ** ������ ������_�_SME -- ������_�_SMSC �� ����������.
  */
  search_result_t
  getCrossedSession(SMPPSession& sessionFromSme);

  /*
  ** �� �������� systemId �������� ������ ��� ������ � SME. ��� �������
  ** �������� systemId ����� ������������ ��������� �������� ������ � SME.
  ** ����� ��������� ������ ���������� ������������ ����������. �� ������� 
  ** ������ �������� ������� ��������� round-robin.
  */
  search_result_t
  getCrossedSession(const std::string& systemId);

  void broadcastMessageToAllSme(const std::string& systemId, const SMPP_message& messageToSend);
private:
  smsc::logger::Logger* _log;
  /*
  ** ��� ��� �������� ������ � SME.
  */
  typedef std::list<SMPPSession> smeSessions_t;

  /* 
  ** ��� ��� ��������� ���������� ������������� ������ � SME ��
  ** �������� �������������� systemId.
  */
  typedef std::map<std::string, smeSessions_t*> sysIdToSmeList_t;
  sysIdToSmeList_t _sysIdToSmeList;

  /*
  ** ��� ��� �������� ������ systemId -> ������_�_SMSC
  */
  typedef std::map<std::string, SMPPSession> mapRegistredSysIdToSmscSession_t;
  mapRegistredSysIdToSmscSession_t _mapRegistredSysIdToSmscSession;

  //for debug only
  //void dumpRegistredSmeSession(const char* where, smeSessionUsageHistogram_t* sessionUsageHist);
};

}

#endif
