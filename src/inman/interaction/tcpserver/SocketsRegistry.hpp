/* ************************************************************************** *
 * TCP Connects registry, implemented as a sorted vector.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_CONNECTS_REGISTRY_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_CONNECTS_REGISTRY_HPP

#include <poll.h>
#include <map>

#include "core/buffers/Array.hpp"
#include "inman/interaction/tcpserver/TcpServerDefs.hpp"

namespace smsc  {
namespace inman {
namespace interaction  {

class SocketInfo {
protected:
  struct Flags_s {
    bool _inUse;
    bool _toClose;
    bool _doAbort;
    bool _doNotify;

    Flags_s() : _inUse(false), _toClose(false)
      , _doAbort(false), _doNotify(false)
    { }

    void clear(void) { _inUse = _toClose = _doAbort = _doNotify = false; }
  };

  Flags_s               _flags;
  SocketListenerIface * _sockHdl;     //Socket events handler
  bool                  _isMonitored; //Connection is monitored by TcpServerListeners

public:
  SocketInfo() : _sockHdl(NULL), _isMonitored(false)
  { }
  SocketInfo(SocketListenerIface & sock_hdl, bool monitoring_on)
    : _sockHdl(&sock_hdl), _isMonitored(monitoring_on)
  { }
  ~SocketInfo()
  { }

  void clear(void) { _flags.clear(); _sockHdl = NULL; _isMonitored = false; }

  bool empty(void) const { return _sockHdl == NULL; }

  unsigned getUId(void) const { return _sockHdl ? _sockHdl->getUId() : 0; }
  SOCKET   getFd(void) const { return _sockHdl ? _sockHdl->getFd() : INVALID_SOCKET; }

  void setRef(void) { _flags._inUse = true; }
  void unRef(void) { _flags._inUse = false; }

  SocketListenerIface * sockHandler(void) const { return _sockHdl; }
  bool isMonitored(void) const { return _isMonitored; }

  bool isInUse(void) const { return _flags._inUse; }
  //
  bool isToClose(void) const { return _flags._toClose; }
  //
  bool needNotify(void) const { return _flags._doNotify; }
  //
  bool needAbort(void) const { return _flags._doAbort; }
  //
  void markToClose(bool do_abort, bool do_notify)
  {
    _flags._doAbort = do_abort;
    _flags._doNotify = do_notify;
    _flags._toClose = true;
  }
  //
  void closeSocket(void)
  {
    if (_flags._doAbort)
      _sockHdl->getSocket()->Abort(); //imediately close socket, discard all unsent data
    else
      _sockHdl->getSocket()->Close(); //send suspended data and close socket
  }
};

struct pollfd_arr {
  nfds_t    _size;
  pollfd *  _ptr;

  pollfd_arr() : _size(0), _ptr(NULL)
  { }
  pollfd_arr(nfds_t use_size, pollfd * use_ptr)
    : _size(use_size), _ptr(use_ptr)
  { }

  bool find(const int fd_num, nfds_t & at_pos) const
  {
    for (; at_pos < _size; ++at_pos) {
      if (_ptr[at_pos].fd == fd_num)
        return true;
    }
    return false;
  }
};

//NOTE: actually registry's size_type is int, because of fd's dimension.
class SocketsRegistry  {
public:
  typedef std::map<unsigned/*sock_id*/, SocketInfo>  SocketsMap;
  typedef SocketsMap::iterator iterator;
  typedef SocketsMap::const_iterator const_iterator;

  explicit SocketsRegistry(int num_to_rsrv = 0)
    : _isModified(false), _fds(num_to_rsrv)
  { }
  ~SocketsRegistry()
  { }

  void            reserve(int num_to_rsrv) { _fds.SetSize(num_to_rsrv); }
  //
  bool            empty(void) const { return _sockMap.empty(); }
  //
  unsigned        size(void) const { return (unsigned)_sockMap.size(); }
  //
  const_iterator  begin(void) const { return _sockMap.begin(); }
  //
  iterator        begin(void) { return _sockMap.begin(); }
  //
  const_iterator  end(void) const { return _sockMap.end(); }
  //
  iterator        end(void) { return _sockMap.end(); }

  //
  SocketInfo *    insert(const SocketInfo & sock_inf);
  //
  void            erase(iterator use_it);
  //
  void            erase(unsigned conn_id);
  //returns NULL if not found
  const_iterator  find(unsigned conn_id) const { return _sockMap.find(conn_id); }
  //returns NULL if not found
  iterator        find(unsigned conn_id) { return _sockMap.find(conn_id); }

  //
  pollfd_arr      composeFds(void);

private:
  bool        _isModified;

protected:
  typedef smsc::core::buffers::Array<pollfd> FDsArray;

  SocketsMap  _sockMap;
  FDsArray    _fds;
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_CONNECTS_REGISTRY_HPP */

