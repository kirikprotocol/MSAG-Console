/* $Id$ */

#ifndef __SCAG_PERS_RWMULTIPLEXER_HPP__
#define __SCAG_PERS_RWMULTIPLEXER_HPP__

#include <logger/Logger.h>
#include "core/network/Socket.hpp"
#include "core/buffers/Array.hpp"
#include "core/network/Multiplexer.hpp"
#include <poll.h>

namespace scag{ namespace pers{

using smsc::core::buffers::Array;
using smsc::core::network::Socket;
using smsc::core::network::Multiplexer;
using smsc::logger::Logger;

class RWMultiplexer: public Multiplexer{
public:
  RWMultiplexer() {
    logger = Logger::getInstance("mplexer");
  }
  virtual ~RWMultiplexer(){}

  int addR(Socket* sock) { return add(sock, POLLIN); }
  int addW(Socket* sock) { return add(sock, POLLOUT); }
  int addRW(Socket* sock) { return add(sock, POLLIN|POLLOUT); }

  int canReadWrite(SockArray& read, SockArray& write, SockArray& error, int timeout = -1);
  
  int count() {return sockets.Count(); };
  Socket* get(int i) {return sockets[i];};
  
  void _remove(int i)
  {
	if(i >= count()) return;
	sockets.Delete(i);
    fds.Delete(i);
  }

protected:
  int add(Socket* sock, int type);

private:
  Logger* logger;
  
};

}}

#endif
