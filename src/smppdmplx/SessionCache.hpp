#ifndef __SMPPDMPLX_SESSIONCACHE_HPP__
# define __SMPPDMPLX_SESSIONCACHE_HPP__

# include <map>
# include <logger/Logger.h>
# include <util/Singleton.hpp>
# include <smppdmplx/core_ax/network/Socket.hpp>
# include <smppdmplx/SMPPSession.hpp>

namespace smpp_dmplx {

/*
** ����� ��� �������� ������ �����_�_SME -- ������_�_SME,
** �����_�_SMSC -- ������_�_SMSC.
*/
class SessionCache : public smsc::util::Singleton<SessionCache> {
public:
  SessionCache();
  void makeSocketSessionEntry(smsc::core_ax::network::Socket& socket,
                              SMPPSession& session);

  typedef std::pair<bool,SMPPSession> search_result_t;

  search_result_t getSession(smsc::core_ax::network::Socket& socket);
  void removeSession(smsc::core_ax::network::Socket& socket);
private:
  typedef std::map<smsc::core_ax::network::Socket, SMPPSession> socket_to_session_map_t;

  smsc::logger::Logger* _log;
  socket_to_session_map_t _socketToSessionMap;
};

}

#endif
