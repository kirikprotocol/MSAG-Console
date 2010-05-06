#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMPOINTSSHOW_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMPOINTSSHOW_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/utilx/runtime_cfg/CompositeParameter.hpp"
# include "eyeline/ss7na/m3ua_gw/runtime_cfg/RuntimeConfig.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_Points_Show : public common::lm_subsystem::LM_Command {
public:
  virtual std::string executeCommand() {
    std::string resultString;

    utilx::runtime_cfg::CompositeParameter& points =
      runtime_cfg::RuntimeConfig::getInstance().find<utilx::runtime_cfg::CompositeParameter>("config.points-definition");

    utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter>
      pointIterator = points.getIterator<utilx::runtime_cfg::CompositeParameter>("point");
    while ( pointIterator.hasElement() ) {
      utilx::runtime_cfg::CompositeParameter* point = pointIterator.getCurrentElement();
      resultString += point->getValue() + ":" +
          " lpc=" + point->getParameter<utilx::runtime_cfg::Parameter>("lpc")->getValue() +
          " ni=" + point->getParameter<utilx::runtime_cfg::Parameter>("ni")->getValue() +
          " standard=" + point->getParameter<utilx::runtime_cfg::Parameter>("standard")->getValue() +
          "\n";

      pointIterator.next();
    }
    delete this;
    return resultString + "OK";
  }
};

}}}}}

#endif
