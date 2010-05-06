#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMMTP3ROUTINGTABLEADDROUTE_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMMTP3ROUTINGTABLEADDROUTE_HPP__

# include <sys/errno.h>

# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"
# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/ss7na/common/lm_subsystem/Exception.hpp"
# include "eyeline/utilx/strtol.hpp"
# include "eyeline/utilx/runtime_cfg/CompositeParameter.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_MTP3RoutingTable_AddRoute : public common::lm_subsystem::LM_Command {
public:
  LM_MTP3RoutingTable_AddRoute(const std::string& route_name, const std::string& lpc,
                               const std::string& dpc, const std::string& sgp_link_id,
                               common::lm_subsystem::CmdsTransactionMgr& trn_mgr)
  : _trnMgr(trn_mgr), _routeName(route_name), _sgpLinkId(sgp_link_id)
  {
    setId(composeCommandId(_routeName));
    _lpc = static_cast<unsigned>(utilx::strtol(lpc.c_str(), NULL, 10));
    if ( !_lpc && errno )
      throw common::lm_subsystem::InvalidCommandLineException("LM_MTP3RoutingTable_AddRoute::LM_MTP3RoutingTable_AddRoute::: invalid lpc value='%s'",
                                                              lpc.c_str());

    _dpc = static_cast<unsigned>(utilx::strtol(dpc.c_str(), NULL, 10));
    if ( !_dpc && errno )
      throw common::lm_subsystem::InvalidCommandLineException("LM_MTP3RoutingTable_AddRoute::LM_MTP3RoutingTable_AddRoute::: invalid dpc value='%s'",
                                                              dpc.c_str());
  }

  virtual std::string executeCommand();

  virtual void updateConfiguration();

  static std::string composeCommandId(const std::string& route_name);

private:
  bool checkAndAddEntry(utilx::runtime_cfg::CompositeParameter* table_param);

  common::lm_subsystem::CmdsTransactionMgr& _trnMgr;
  std::string _routeName, _sgpLinkId;
  unsigned _lpc, _dpc;
};

}}}}}

#endif
