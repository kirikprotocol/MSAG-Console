#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMSGPLINKSSHOW_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMSGPLINKSSHOW_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/utilx/runtime_cfg/CompositeParameter.hpp"
# include "eyeline/ss7na/m3ua_gw/runtime_cfg/RuntimeConfig.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_SGPLinks_Show : public common::lm_subsystem::LM_Command {
public:
  virtual std::string executeCommand() {
    std::string resultString;

    utilx::runtime_cfg::CompositeParameter& sgpLinks =
      runtime_cfg::RuntimeConfig::getInstance().find<utilx::runtime_cfg::CompositeParameter>("config.sgp_links");

    utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter>
      linkIterator = sgpLinks.getIterator<utilx::runtime_cfg::CompositeParameter>("link");
    while ( linkIterator.hasElement() ) {
      utilx::runtime_cfg::CompositeParameter* link = linkIterator.getCurrentElement();
      resultString += link->getValue();
      utilx::runtime_cfg::CompositeParameter* localAddresses =
          link->getParameter<utilx::runtime_cfg::CompositeParameter>("local_addresses");
      resultString += "\n\t";
      if ( localAddresses ) {
        utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter>
          lAddrIterator = localAddresses->getIterator<utilx::runtime_cfg::Parameter>("address");
        while ( lAddrIterator.hasElement() ) {
          resultString += lAddrIterator.getCurrentElement()->getValue() + ",";
          lAddrIterator.next();
        }
        resultString += link->getParameter<utilx::runtime_cfg::Parameter>("local_port")->getValue();
      }
      utilx::runtime_cfg::CompositeParameter* remoteAddresses =
          link->getParameter<utilx::runtime_cfg::CompositeParameter>("remote_addresses");
      resultString += " ==> ";
      utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter>
        rAddrIterator = remoteAddresses->getIterator<utilx::runtime_cfg::Parameter>("address");
      while ( rAddrIterator.hasElement() ) {
        resultString += rAddrIterator.getCurrentElement()->getValue() + ",";
        rAddrIterator.next();
      }
      resultString += link->getParameter<utilx::runtime_cfg::Parameter>("remote_port")->getValue() + "\n";

      linkIterator.next();
    }
    delete this;
    return resultString + "OK";
  }
};

}}}}}

#endif
