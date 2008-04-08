#ifndef __SUA_UTILX_SHUTDOWNINPROGRESSNOTIFICATION_HPP__
# define __SUA_UTILX_SHUTDOWNINPROGRESSNOTIFICATION_HPP__ 1

namespace utilx {

class ShutdownInProgressNotification {
public:
  virtual ~ShutdownInProgressNotification() {}
  virtual void notifyShutdownInProgess() = 0;
};

}

#endif
