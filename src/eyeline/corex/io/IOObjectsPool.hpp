#ifndef __EYELINE_COREX_NETWORK_IOOBJECTSPOOL_HPP__
# define __EYELINE_COREX_NETWORK_IOOBJECTSPOOL_HPP__

# include <deque>
# include <map>
# include <poll.h>
# include <limits.h>

# include "eyeline/corex/io/network/Socket.hpp"
# include "eyeline/corex/io/network/ServerSocket.hpp"
# include "eyeline/corex/io/IOStreams.hpp"

namespace eyeline {
namespace corex {
namespace io {

template <class LOCK>
class IOObjectsPool_tmpl {
public:
  typedef enum {OK_NO_EVENTS = 0, OK_READ_READY = 0x01, OK_WRITE_READY=0x02, OK_ACCEPT_READY=0x04, TIMEOUT = -1} status_t;
  typedef enum {WAIT_READ=1, WAIT_WRITE=2} wait_type_t;

  explicit IOObjectsPool_tmpl(int maxPoolSize);
  int listen(uint32_t timeout=0);

  void insert(InputStream* iStream);
  void insert(OutputStream* oStream);
  void remove(InputStream* iStream);
  void remove(OutputStream* oStream);
  void remove(IOObject* streamsOwner);

  void insert(corex::io::network::ServerSocket* socket);
  void remove(corex::io::network::ServerSocket* socket);

  // next methods returns NULL if there is no ready io stream
  InputStream* getNextReadyInputStream();
  OutputStream* getNextReadyOutputStream();

  corex::io::network::ServerSocket* getNextReadyServerSocket();
private:
  int calcPolledFds(int* sockets_count);
  bool updatePollIndexes(int fd, short event=0);

  struct pollfd *_fds, *_snaphots_fds;
  int* _used_fds;
  int _maxPoolSize, _socketsCount;
  int _signallingPipe[2];
  bool _pollingIsActive;
  enum { INFTIM = -1 };

  typedef std::map<int, InputStream*> in_mask_t;
  typedef std::map<int, OutputStream*> out_mask_t;
  typedef std::map<int, corex::io::network::ServerSocket*> accept_mask_t;

  in_mask_t _inMask;
  out_mask_t _outMask;
  accept_mask_t _acceptMask;

  typedef std::map<int, InputStream*> in_events_t;
  typedef std::map<int, OutputStream*> out_events_t;
  typedef std::deque<corex::io::network::ServerSocket*> accept_events_t;

  in_events_t _inputEventsReady;
  out_events_t _outputEventsReady;
  accept_events_t _newConnectionEventsReady;

  LOCK _lock;
};

typedef IOObjectsPool_tmpl<smsc::core::synchronization::Mutex> IOObjectsPool;

}}}

# include <eyeline/corex/io/IOObjectsPool_impl.hpp>

#endif
