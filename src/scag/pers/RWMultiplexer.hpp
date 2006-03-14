/* $Id$ */

#ifndef __SCAG_PERS_RWMULTIPLEXER_HPP__
#define __SCAG_PERS_RWMULTIPLEXER_HPP__

#include "core/network/Socket.hpp"
#include "core/buffers/Array.hpp"
#include "core/network/Multiplexer.hpp"
#include <poll.h>

namespace scag{ namespace pers{

using smsc::core::buffers::Array;
using smsc::core::network::Socket;
using smsc::core::network::Multiplexer;

class RWMultiplexer: public Multiplexer{
protected:
  int add(Socket* sock, int type);

public:
  RWMultiplexer(){}
  virtual ~RWMultiplexer(){}

  int addR(Socket* sock) { return add(sock, POLLIN); }
  int addW(Socket* sock) { return add(sock, POLLOUT); }
  int addRW(Socket* sock) { return add(sock, POLLIN|POLLOUT); }

  int canReadWrite(SockArray& read, SockArray& write, SockArray& error, int timeout = -1);
};

}}

#endif
