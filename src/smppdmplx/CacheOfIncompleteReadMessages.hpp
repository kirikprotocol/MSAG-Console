#ifndef __SMPPDMPLX_CACHEOFINCOMPLETEREADMESSAGE_HPP__
# define __SMPPDMPLX_CACHEOFINCOMPLETEREADMESSAGE_HPP__

# include <map>
# include <logger/Logger.h>
# include <util/Singleton.hpp>
# include <smppdmplx/RawMessage.hpp>
# include <smppdmplx/core_ax/network/Socket.hpp>

namespace smpp_dmplx {

/*
** Class responsibility is to grant API to cache incomplete incoming SMPP messages.
*/
class CacheOfIncompleteReadMessages : public smsc::util::Singleton<CacheOfIncompleteReadMessages>
{
public:
  CacheOfIncompleteReadMessages();
  smpp_dmplx::RawMessage& getIncompleteMessageForSocket(const smsc::core_ax::network::Socket& socket);
  void removeCompleteMessageForSocket(const smsc::core_ax::network::Socket& socket);
private:
  smsc::logger::Logger* _log;
  typedef std::map<smsc::core_ax::network::Socket, smpp_dmplx::RawMessage> SocketMessageCache_t;
  SocketMessageCache_t _socketMessageCache;
};

}

#endif
