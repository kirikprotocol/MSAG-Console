#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_SCMG_SCMGMESSAGEPROCESSOR_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_SCMG_SCMGMESSAGEPROCESSOR_HPP__

# include <sys/types.h>
# include "logger/Logger.h"
# include "eyeline/utilx/types.hpp"
# include "eyeline/utilx/Singleton.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace scmg {

class SCMGMessageProcessor : public utilx::Singleton<SCMGMessageProcessor> {
public:
  SCMGMessageProcessor()
  : _logger(smsc::logger::Logger::getInstance("sccp"))
  {}

  void handle(const utilx::variable_data_t& scmg_msg_data);

private:
  smsc::logger::Logger* _logger;
  static const unsigned MIN_MESSAGE_SIZE = 5;
};

}}}}}

#endif
