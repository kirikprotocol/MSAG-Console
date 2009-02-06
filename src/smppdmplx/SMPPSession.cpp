#include <sstream>
#include <util/Exception.hpp>
#include <smppdmplx/core_ax/network/SocketPool.hpp>
#include "SMPPSession.hpp"
#include "Configuration.hpp"

namespace smpp_dmplx {

SMPPSession::session_state_t
SMPPSession::_transition_table[6][5] = 
  { INVALID/*(0,0)->0*/, INVALID/*(0,1)->0*/, INVALID/*(0,2)->0*/, INVALID/*(0,3)->0*/, INVALID/*(0,4)->0*/,
    INVALID/*(1,0)->0*/, INVALID/*(1,1)->0*/, INVALID/*(1,2)->0*/, INVALID/*(1,3)->0*/, INVALID/*(1,4)->0*/,
    BIND_IN_PROGRESS/*(2,0)->3*/, INVALID/*(2,1)->0*/, INVALID/*(2,2)->0*/, INVALID/*(2,3)->0*/, INVALID/*(2,4)->0*/,
    INVALID/*(3,0)->0*/, BOUND/*(3,1)->4*/, INVALID/*(3,2)->0*/, INVALID/*(3,3)->0*/, CLOSED/*(3,4)->1*/,
    INVALID/*(4,0)->0*/, INVALID/*(4,1)->0*/, BOUND/*(4,2)->4*/, CLOSED/*(4,3)->1*/, INVALID/*(4,4)->0*/,
    INVALID/*(5,0)->0*/, INVALID/*(5,1)->0*/, INVALID/*(5,2)->0*/, INVALID/*(5,3)->0*/, INVALID/*(5,4)->0*/,
  };

SMPPSession::SMPPSession(const std::string& systemId)
  : _shared_session_info(new shared_session_info(UNINITIALIZED, systemId)),
    _log(smsc::logger::Logger::getInstance("smppsess"))
{}

SMPPSession::SMPPSession(const std::string& systemId, smsc::core_ax::network::Socket& connectedSocket)
  : _shared_session_info(new shared_session_info(OPENED, connectedSocket, systemId)),
    _log(smsc::logger::Logger::getInstance("smppsess"))
{}

SMPPSession::SMPPSession(const SMPPSession& rhs)
  : _shared_session_info(rhs._shared_session_info),
    _log(smsc::logger::Logger::getInstance("smppsess"))
{
  _shared_session_info->incrementRefCount();
}

SMPPSession&
SMPPSession::operator=(const SMPPSession& rhs)
{
  if ( this != &rhs ) {
    _shared_session_info->decrementRefCount();
    if ( _shared_session_info->getRefCount() == 0 )
      delete _shared_session_info;

    _shared_session_info = rhs._shared_session_info;
    _shared_session_info->incrementRefCount();

  }
  return *this;
}

SMPPSession::~SMPPSession()
{
  _shared_session_info->decrementRefCount();
  if ( _shared_session_info->getRefCount() == 0 )
    delete _shared_session_info;
}

SMPPSession::auth_result_t
SMPPSession::authenticate(const std::string& password)
{
  Configuration& configuration = Configuration::getInstance();

  Configuration::systemid_to_passwd_map_t::iterator iter =
    configuration.sysid_Passwd_map.find(getSystemId());
  if ( iter != configuration.sysid_Passwd_map.end() &&
       iter->second == password ) {
    smsc_log_info(_log,"SMPPSession::authenticate::: SME authentication with id=[%s] is successful", getSystemId().c_str());
    return AUTH_SUCCESS;
  } else {
    smsc_log_info(_log,"SMPPSession::authenticate::: SME authentication with id=%s is falied", getSystemId().c_str());
    return AUTH_FAILED;
  }
}

void
SMPPSession::connectSession()
{
  _shared_session_info->_socketToPeer = smsc::core_ax::network::Socket(Configuration::getInstance().smscHost, Configuration::getInstance().smscPort);
  _shared_session_info->_socketToPeer.connect();
  _shared_session_info->_socketToPeer.setNonBlocking();
  _shared_session_info->_state = OPENED;
}

void
SMPPSession::closeSession()
{
  _shared_session_info->_state = CLOSED;
}


smsc::core_ax::network::Socket&
SMPPSession::getSocketToPeer() const
{
  return _shared_session_info->_socketToPeer;
}

const std::string&
SMPPSession::getSystemId() const
{
  return _shared_session_info->_systemId;
}

SMPPSession::operation_result_t
SMPPSession::updateSessionState(event_t event)
{
  session_state_t newSessionState = _transition_table[_shared_session_info->_state][event];

  if ( newSessionState != _shared_session_info->_state )
    smsc_log_info(_log,"SMPPSession::updateSessionState::: update session=[%s]. event=[%s], session state transition [%s]==>[%s]", toString().c_str(), eventToString(event), stateToString(_shared_session_info->_state), stateToString(newSessionState));
  
  _shared_session_info->_state = newSessionState;
    
  if ( _shared_session_info->_state == INVALID )
    return OPERATION_FAILED;
  else
    return OPERATION_SUCCESS;
}

bool
SMPPSession::bindInProgress() const
{
  return _shared_session_info->_state == BIND_IN_PROGRESS;
}

bool
SMPPSession::operator<(const SMPPSession& rhs) const
{
  return _shared_session_info < rhs._shared_session_info;
}

bool
SMPPSession::operator==(const SMPPSession& rhs) const
{
  return _shared_session_info == rhs._shared_session_info;
}

bool
SMPPSession::operator!=(const SMPPSession& rhs) const
{
  return !(*this == rhs);
}

const std::string
SMPPSession::toString() const
{
  std::ostringstream obuf;

  obuf << "systemId=[" << getSystemId() 
       << "] socket=[" << getSocketToPeer().toString()
       << "]";
  return obuf.str();
}

const char*
SMPPSession::stateToString(session_state_t state) const
{
  switch(state) {
  case INVALID:
    return "INVALID";
  case CLOSED:
    return "CLOSED";
  case OPENED:
    return "OPENED";
  case BIND_IN_PROGRESS:
    return "BIND_IN_PROGRESS";
  case BOUND:
    return "BOUND";
  case UNINITIALIZED:
    return "UNINITIALIZED";
  default:
    throw smsc::util::Exception("SMPPSession::stateToString::: invalid session_state value=[%d]", state);
  }
}

const char*
SMPPSession::eventToString(event_t event) const
{
  switch(event) {
  case GOT_BIND_REQ:
    return "GOT_BIND_REQ";
  case GOT_BIND_RESP:
    return "GOT_BIND_RESP";
  case GOT_APP_MESSAGE:
    return "GOT_APP_MESSAGE";
  case GOT_UNBIND:
    return "GOT_UNBIND";
  case GOT_NEGATIVE_BIND_RESP:
    return "GOT_NEGATIVE_BIND_RESP";
  default:
    throw smsc::util::Exception("SMPPSession::eventToString::: invalid event value=[%d]", event);
  }
}

}
