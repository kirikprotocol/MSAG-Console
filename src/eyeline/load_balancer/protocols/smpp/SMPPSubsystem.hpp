#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SMPPSUBSYSTEM_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SMPPSUBSYSTEM_HPP__

# include <sys/types.h>
# include <string>

# include "logger/Logger.h"
# include "util/config/ConfigView.h"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class SMPPSubsystem {
public:
  SMPPSubsystem()
  : _logger(smsc::logger::Logger::getInstance("smpp"))
  {}
  void initialize();
private:
  smsc::logger::Logger* _logger;
};

}}}}

#endif

