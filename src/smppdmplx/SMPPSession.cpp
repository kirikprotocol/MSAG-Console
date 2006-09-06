#include "SMPPSession.hpp"
#include <core_ax/network/SocketPool.hpp>
#include "Configuration.hpp"
#include <sstream>

#include <logger/Logger.h>
extern smsc::logger::Logger* dmplxlog;

smpp_dmplx::SMPPSession::session_state_t
smpp_dmplx::SMPPSession::_transition_table[6][5] = 
  { INVALID/*(0,0)->0*/, INVALID/*(0,1)->0*/, INVALID/*(0,2)->0*/, INVALID/*(0,3)->0*/, INVALID/*(0,4)->0*/,
    INVALID/*(1,0)->0*/, INVALID/*(1,1)->0*/, INVALID/*(1,2)->0*/, INVALID/*(1,3)->0*/, INVALID/*(1,4)->0*/,
    BIND_IN_PROGRESS/*(2,0)->3*/, INVALID/*(2,1)->0*/, INVALID/*(2,2)->0*/, INVALID/*(2,3)->0*/, INVALID/*(2,4)->0*/,
    INVALID/*(3,0)->0*/, BOUND/*(3,1)->4*/, INVALID/*(3,2)->0*/, INVALID/*(3,3)->0*/, CLOSED/*(3,4)->1*/,
    INVALID/*(4,0)->0*/, INVALID/*(4,1)->0*/, BOUND/*(4,2)->4*/, CLOSED/*(4,3)->1*/, INVALID/*(4,4)->0*/,
    INVALID/*(5,0)->0*/, INVALID/*(5,1)->0*/, INVALID/*(5,2)->0*/, INVALID/*(5,3)->0*/, INVALID/*(5,4)->0*/,
  };

smpp_dmplx::SMPPSession::SMPPSession(const std::string& systemId)
{
  _shared_session_info = new shared_session_info(UNINITIALIZED, systemId);
}

smpp_dmplx::SMPPSession::SMPPSession(const std::string& systemId, smsc::core_ax::network::Socket& connectedSocket)
{
  _shared_session_info = new shared_session_info(OPENED, connectedSocket, systemId);
}

smpp_dmplx::SMPPSession::SMPPSession(const SMPPSession& rhs)
{
  _shared_session_info = rhs._shared_session_info;
  _shared_session_info->incrementRefCount();
}

smpp_dmplx::SMPPSession&
smpp_dmplx::SMPPSession::operator=(const SMPPSession& rhs)
{
  if ( this != &rhs ) {
    _shared_session_info->decrementRefCount();
    if (  _shared_session_info->getRefCount() == 0 )
      delete _shared_session_info;

    _shared_session_info = rhs._shared_session_info;
    _shared_session_info->incrementRefCount();
  }
  return *this;
}

smpp_dmplx::SMPPSession::~SMPPSession()
{
  _shared_session_info->decrementRefCount();
  if (  _shared_session_info->getRefCount() == 0 )
    delete _shared_session_info;
}

smpp_dmplx::SMPPSession::auth_result_t
smpp_dmplx::SMPPSession::authenticate(const std::string& password)
{
  smpp_dmplx::Configuration& 
    configuration = smpp_dmplx::Configuration::getInstance();

  Configuration::systemid_to_passwd_map_t::iterator iter =
    configuration.sysid_Passwd_map.find(getSystemId());
  if ( iter != configuration.sysid_Passwd_map.end() &&
       iter->second == password ) {
    smsc_log_info(dmplxlog,"SMPPSession::authenticate::: SME authentication with id=[%s] is successful", getSystemId().c_str());
    return AUTH_SUCCESS;
  } else {
    smsc_log_info(dmplxlog,"SMPPSession::authenticate::: SME authentication with id=%s is falied", getSystemId().c_str());
    return AUTH_FAILED;
  }
}

void
smpp_dmplx::SMPPSession::connectSession()
{
  const char* hostSMSC = Configuration::getInstance().smscHost.c_str();
  in_port_t portToSMSC = Configuration::getInstance().smscPort;

  _shared_session_info->_socketToPeer = smsc::core_ax::network::Socket(hostSMSC, portToSMSC);
  _shared_session_info->_socketToPeer.connect();
  _shared_session_info->_socketToPeer.setNonBlocking();
  _shared_session_info->_state = OPENED;
}

void smpp_dmplx::SMPPSession::closeSession()
{
  _shared_session_info->_state = CLOSED;
}


smsc::core_ax::network::Socket& smpp_dmplx::SMPPSession::getSocketToPeer() const
{
  return _shared_session_info->_socketToPeer;
}

const std::string& smpp_dmplx::SMPPSession::getSystemId() const
{
  return _shared_session_info->_systemId;
}

smpp_dmplx::SMPPSession::operation_result_t
smpp_dmplx::SMPPSession::updateSessionState(event_t event)
{
  smsc_log_info(dmplxlog,"SMPPSession::updateSessionState::: event=[%d], current_session_state=[%d]", event, _shared_session_info->_state);
  _shared_session_info->_state = 
    _transition_table[_shared_session_info->_state][event];
  if ( _shared_session_info->_state == INVALID )
    return OPERATION_FAILED;
  else
    return OPERATION_SUCCESS;
}

bool
smpp_dmplx::SMPPSession::bindInProgress() const
{
  return _shared_session_info->_state == BIND_IN_PROGRESS;
}

bool smpp_dmplx::SMPPSession::operator<(const SMPPSession& rhs) const
{
  return _shared_session_info < rhs._shared_session_info;
}

bool smpp_dmplx::SMPPSession::operator==(const SMPPSession& rhs) const
{
  return _shared_session_info == rhs._shared_session_info;
}

bool smpp_dmplx::SMPPSession::operator!=(const SMPPSession& rhs) const
{
  return !(*this == rhs);
}

const std::string
smpp_dmplx::SMPPSession::toString() const
{
  std::ostringstream obuf;

  obuf << "systemId=[" << getSystemId() 
       << "] socket=[" << getSocketToPeer().toString()
       << "]";
  return obuf.str();
}
