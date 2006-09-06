#ifndef  __SMPPDMPLX_SMPPSESSION_HPP__
# define __SMPPDMPLX_SMPPSESSION_HPP__ 1

# include <core_ax/network/Socket.hpp>
# include <string>
# include <logger/Logger.h>
extern smsc::logger::Logger* dmplxlog;

namespace smpp_dmplx {

/*
** ����������, � ������������ �����������, �����.
** ��������������� ������ - ������������ ��������� ��� ����������
** SMPP-������. ������ ������ SMPPSession ������ ���������� � ���������
** ������ - INVALID (�� ����������������), CLOSED (�������), OPENED (�������),
** BOUND (������ � ��������� ���������� ��������).
** ������ ����� ���������, ����������, ���������� � ����� ���������.
** ������ ������ �����, ��������������� �������������
** ���������� (��������� - ��� ������ �� SME, ���������� - ��� ������ � SMSC)
** ����� ������ �������������� ��� ������ ��� � ���������
** ������� � SMSC, ��� � �� �������� ������� �� SME.
*/
class SMPPSession
{
public:
  typedef enum {INVALID=0, CLOSED = 1, OPENED = 2, BIND_IN_PROGRESS = 3, BOUND = 4, UNINITIALIZED = 5} session_state_t;
  typedef enum {OPERATION_SUCCESS = 0, OPERATION_FAILED = -1} operation_result_t;
  typedef enum {GOT_BIND_REQ = 0, GOT_BIND_RESP = 1, GOT_APP_MESSAGE = 2, GOT_UNBIND = 3, GOT_NEGATIVE_BIND_RESP = 4} event_t;
  // ����������� ������� ������ � ��������� UNINITIALIZED
  explicit SMPPSession(const std::string& systemId="");

  // ����������� ������� ������ � ��������� OPENED
  explicit SMPPSession(const std::string& systemId, smsc::core_ax::network::Socket& connectedSocket);

  SMPPSession(const SMPPSession& rhs);
  SMPPSession& operator=(const SMPPSession& rhs);

  ~SMPPSession();

  // ���������� ������������ ���������� ��� ������.
  void connectSession();

  void closeSession();

  typedef enum {AUTH_SUCCESS = 0, AUTH_FAILED = -1} auth_result_t;
  auth_result_t authenticate(const std::string& pasword);

  //  session_state_t getSessionState() const;
  smsc::core_ax::network::Socket& getSocketToPeer() const;

  const std::string& getSystemId() const;

  operation_result_t updateSessionState(event_t event);
  bool bindInProgress() const;

  bool operator<(const SMPPSession&) const;
  bool operator==(const SMPPSession&) const;
  bool operator!=(const SMPPSession&) const;
  const std::string toString() const;
private:
  // _transition_table[_state][event]
  static session_state_t _transition_table[6][5];

  struct shared_session_info {
    // ����������� ��� �������� ������ �� SME
    shared_session_info(session_state_t state, smsc::core_ax::network::Socket& socketToPeer, const std::string& systemId) : _state(state), _ref_count(1), _socketToPeer(socketToPeer), _systemId(systemId) {
      smsc_log_info(dmplxlog,"shared_session_info::shared_session_info::: object [this=%p] is created", this);
    }
    // ����������� ��� �������� ������ � SMSC
    shared_session_info(session_state_t state, const std::string& systemId) : _state(state), _ref_count(1), _systemId(systemId) {
      smsc_log_info(dmplxlog,"shared_session_info::shared_session_info::: object [this=%p] is created", this);
    }
    ~shared_session_info() {
      smsc_log_info(dmplxlog,"shared_session_info::~shared_session_info::: object [this=%p] is destroyed", this);
      /*decrementRefCount();*/
    }

    void decrementRefCount() { --_ref_count; }
    void incrementRefCount() { ++_ref_count; }
    int getRefCount() { return _ref_count; }

    session_state_t _state;
    int _ref_count;
    smsc::core_ax::network::Socket _socketToPeer;
    std::string _systemId;
  };
  shared_session_info* _shared_session_info;
};

}

#endif
