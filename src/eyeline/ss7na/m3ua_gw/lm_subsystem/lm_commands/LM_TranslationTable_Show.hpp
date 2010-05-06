#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONTABLESHOW_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONTABLESHOW_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/utilx/runtime_cfg/CompositeParameter.hpp"
# include "eyeline/ss7na/m3ua_gw/runtime_cfg/RuntimeConfig.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_TranslationTable_Show : public common::lm_subsystem::LM_Command {
public:
  virtual std::string executeCommand() {
    std::string resultString;
    utilx::runtime_cfg::CompositeParameter& translationTable =
        runtime_cfg::RuntimeConfig::getInstance().find<utilx::runtime_cfg::CompositeParameter>("config.translation-table");

    utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter>
      entryIterator = translationTable.getIterator<utilx::runtime_cfg::CompositeParameter>("entry");
    while ( entryIterator.hasElement() ) {
      utilx::runtime_cfg::CompositeParameter* entry = entryIterator.getCurrentElement();
      utilx::runtime_cfg::Parameter* gt =
          entry->getParameter<utilx::runtime_cfg::Parameter>("gt");
      utilx::runtime_cfg::Parameter* ssn =
          entry->getParameter<utilx::runtime_cfg::Parameter>("ssn");
      utilx::runtime_cfg::Parameter* trafficMode =
          entry->getParameter<utilx::runtime_cfg::Parameter>("traffic-mode");
      utilx::runtime_cfg::CompositeParameter* sccpUsers =
          entry->getParameter<utilx::runtime_cfg::CompositeParameter>("sccp_users");
      if ( sccpUsers ) {
        std::string sccpUsersStr;
        utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter> userIdIter =
            sccpUsers->getIterator<utilx::runtime_cfg::Parameter>("userid");
        while ( userIdIter.hasElement() ) {
          sccpUsersStr += userIdIter.getCurrentElement()->getValue();
          userIdIter.next();
          if ( userIdIter.hasElement() )
            sccpUsersStr += ",";
        }
        resultString += entry->getValue() + ": " + gt->getValue();
        if ( ssn )
          resultString += "," + ssn->getValue();
        resultString += " ==> sccp_users=" +
            sccpUsersStr + ",mode=" + trafficMode->getValue();
      } else {
        std::string pcsStr;
        utilx::runtime_cfg::CompositeParameter* dpcs =
            entry->getParameter<utilx::runtime_cfg::CompositeParameter>("dpcs");
        utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter> pcIter =
            dpcs->getIterator<utilx::runtime_cfg::Parameter>("pc");
        while ( pcIter.hasElement() ) {
          pcsStr += pcIter.getCurrentElement()->getValue();
          pcIter.next();
          if ( pcIter.hasElement() )
            pcsStr += ",";
        }
        utilx::runtime_cfg::Parameter* lpc =
            entry->getParameter<utilx::runtime_cfg::Parameter>("lpc");
        resultString += entry->getValue() + ": " + gt->getValue();
        if ( ssn )
          resultString += "," + ssn->getValue();
        resultString += " ==> lpc=" + lpc->getValue() + ",dpcs=" +
            pcsStr + ",mode=" + trafficMode->getValue();
      }
      entryIterator.next();
      resultString += "\n";
    }
    delete this;
    return resultString + "OK";
  }
};

}}}}}

#endif
