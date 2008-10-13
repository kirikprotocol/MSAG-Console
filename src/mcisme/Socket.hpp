#include <core/network/Socket.hpp>

namespace smsc {
namespace mcisme {

class Socket : public core::network::Socket {
public:
  int Connect();
  void setNonBlocking(int mode /* 0 - blocking, 1 - non blocking*/);
};

}}
