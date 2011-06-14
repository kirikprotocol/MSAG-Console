/* ************************************************************************** *
 * TCP Server configuration params.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_TCP_SERVER_CFG_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_TCP_SERVER_CFG_HPP

#include <string>

namespace smsc  {
namespace inman {
namespace interaction  {

struct TcpListenerCFG {
  static const unsigned int _MIN_SHUTDOWN_TMO_MS = 100;  //millisecs
  static const unsigned int _MIN_POLL_TMO_MS = 100;      //millisecs

  unsigned int    _pollTmo; //sockets listening timeout, millisecs
  unsigned int    _shtdTmo; //connection establishing/shutdown timeout, millisecs

  TcpListenerCFG() : _pollTmo(0), _shtdTmo(0)
  { }
  TcpListenerCFG(unsigned poll_tmo, unsigned shtd_tmo)
    : _pollTmo(poll_tmo), _shtdTmo(shtd_tmo)
  { }
};

struct TcpServerCFG : public TcpListenerCFG {
  std::string     _host;
  unsigned int    _port;
  unsigned int    _maxConn; //maximum number of client's connections to accept

  TcpServerCFG() : TcpListenerCFG(), _port(0), _maxConn(0)
  { }
  ~TcpServerCFG()
  { }

  TcpServerCFG & operator=(const TcpListenerCFG & use_val)
  {
    *(TcpListenerCFG*)this = use_val;
    return *this;
  }
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_TCP_SERVER_CFG_HPP */

