#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMMTP3ROUTINGTABLESHOW_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMMTP3ROUTINGTABLESHOW_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/utilx/runtime_cfg/CompositeParameter.hpp"
# include "eyeline/ss7na/m3ua_gw/runtime_cfg/RuntimeConfig.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_MTP3RoutingTable_Show : public common::lm_subsystem::LM_Command {
public:
  virtual std::string executeCommand() {
    std::string resultString;

    utilx::runtime_cfg::CompositeParameter& mtp3RoutingTables =
      runtime_cfg::RuntimeConfig::getInstance().find<utilx::runtime_cfg::CompositeParameter>("config.mtp3-routing-tables");

    utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter>
      tableIterator = mtp3RoutingTables.getIterator<utilx::runtime_cfg::CompositeParameter>("table");
    while ( tableIterator.hasElement() ) {
      utilx::runtime_cfg::CompositeParameter* table = tableIterator.getCurrentElement();
      resultString += table->getValue() + ": lpc=" +
          table->getParameter<utilx::runtime_cfg::Parameter>("lpc")->getValue() + "\n";

      utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter>
        entryIterator = table->getIterator<utilx::runtime_cfg::CompositeParameter>("entry");
      while ( entryIterator.hasElement() ) {
        resultString += "\t" +
            entryIterator.getCurrentElement()->getParameter<utilx::runtime_cfg::Parameter>("dpc")->getValue() +
            " ==> " +
            entryIterator.getCurrentElement()->getParameter<utilx::runtime_cfg::Parameter>("sgp_link")->getValue();
        entryIterator.next();
        resultString += "\n";
      }

      tableIterator.next();
    }
    delete this;
    return resultString + "OK";
  }
};

}}}}}

#endif
