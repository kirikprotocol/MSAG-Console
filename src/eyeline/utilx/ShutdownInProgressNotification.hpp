#ifndef __EYELINE_UTILX_SHUTDOWNINPROGRESSNOTIFICATION_HPP__
# define __EYELINE_UTILX_SHUTDOWNINPROGRESSNOTIFICATION_HPP__

namespace eyeline {
namespace utilx {

class ShutdownInProgressNotification {
public:
  virtual ~ShutdownInProgressNotification() {}
  virtual void notifyShutdownInProgess() = 0;
};

}}

#endif
