#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMMTP3ROUTINGTABLEREMOVEROUTE_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMMTP3ROUTINGTABLEREMOVEROUTE_HPP__

# include "eyeline/ss7na/common/types.hpp"
# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"
# include "eyeline/utilx/runtime_cfg/CompositeParameter.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_MTP3RoutingTable_RemoveRoute : public common::lm_subsystem::LM_Command {
public:
  LM_MTP3RoutingTable_RemoveRoute(const std::string& route_name,
                                  common::lm_subsystem::CmdsTransactionMgr& trn_mgr)
  : _trnMgr(trn_mgr), _routeName(route_name)
  {
    setId(composeCommandId(_routeName));
  }

  virtual std::string executeCommand();

  virtual void updateConfiguration();

  static std::string composeCommandId(const std::string& route_name);

private:
  bool checkNoTranslationTableRef(common::point_code_t dpc);
  void removeRoute(utilx::runtime_cfg::CompositeParameter* entry_param,
                   common::point_code_t lpc);

  common::lm_subsystem::CmdsTransactionMgr& _trnMgr;
  std::string _routeName;
};

}}}}}

#endif
