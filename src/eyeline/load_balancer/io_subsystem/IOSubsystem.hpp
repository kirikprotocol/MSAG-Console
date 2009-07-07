#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_IOSUBSYSTEM_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_IOSUBSYSTEM_HPP__

# include <sys/types.h>
# include <string>

# include "logger/Logger.h"
# include "util/config/ConfigView.h"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class IOSubsystem {
public:
  IOSubsystem();
  void initialize(const smsc::util::config::ConfigView& smpp_cfg_entry);
  void start();
  void stop();
  void waitForCompletion();
private:
  smsc::logger::Logger* _logger;
};

}}}

#endif
