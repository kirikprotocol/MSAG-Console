#ifndef __SMPPDMPLX_SESSIONCACHE_HPP__
# define __SMPPDMPLX_SESSIONCACHE_HPP__ 1

# include <util/Singleton.hpp>
# include <core_ax/network/Socket.hpp>
# include "SMPPSession.hpp"
# include <map>

namespace smpp_dmplx {

/*
** Класс для хранения связок сокет_с_SME -- сессия_с_SME,
** сокет_с_SMSC -- сессия_с_SMSC.
*/
class SessionCache : public smsc::util::Singleton<SessionCache> {
public:
  void makeSocketSessionEntry(smsc::core_ax::network::Socket& socket,
                              SMPPSession& session);

  typedef std::pair<bool,SMPPSession> search_result_t;

  search_result_t getSession(smsc::core_ax::network::Socket& socket);
  void removeSession(smsc::core_ax::network::Socket& socket);
private:
  typedef std::map<smsc::core_ax::network::Socket, SMPPSession> socket_to_session_map_t;

  socket_to_session_map_t _socketToSessionMap;
};

}

#endif
