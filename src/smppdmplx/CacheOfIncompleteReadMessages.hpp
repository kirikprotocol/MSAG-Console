#ifndef __SMPPDMPLX_CACHEOFINCOMPLETEREADMESSAGE_HPP__
# define __SMPPDMPLX_CACHEOFINCOMPLETEREADMESSAGE_HPP__ 1

# include <core_ax/network/Socket.hpp>
# include "RawMessage.hpp"

# include <map>

namespace smpp_dmplx {

/*
** Class responsibility is grant API tor cache incomplete reading SMPP message.
*/
class CacheOfIncompleteReadMessages
{
public:
  smpp_dmplx::RawMessage& getIncompleteMessageForSocket(const smsc::core_ax::network::Socket& socket);
  void removeCompleteMessageForSocket(const smsc::core_ax::network::Socket& socket);
private:
  typedef std::map<smsc::core_ax::network::Socket, smpp_dmplx::RawMessage> SocketMessageCache_t;
  SocketMessageCache_t _socketMessageCache;
};

}

#endif
