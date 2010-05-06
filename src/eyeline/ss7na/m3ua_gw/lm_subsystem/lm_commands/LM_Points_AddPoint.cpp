#include "LM_Points_AddPoint.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/PointsDefinitionRegistry.hpp"
#include "eyeline/ss7na/m3ua_gw/runtime_cfg/RuntimeConfig.hpp"
#include "eyeline/ss7na/m3ua_gw/lm_subsystem/RootLayerCommandsInterpreter.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_Points_AddPoint::executeCommand() {
  _trnMgr.addOperation(this);
  return "OK";
}

void
LM_Points_AddPoint::updateConfiguration()
{
  utilx::runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();
  utilx::runtime_cfg::CompositeParameter& pointDefs = runtimeConfig.find<utilx::runtime_cfg::CompositeParameter>("config.points-definition");

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter> iterator =
      pointDefs.getIterator<utilx::runtime_cfg::CompositeParameter>("point");

  while(iterator.hasElement()) {
    const utilx::runtime_cfg::CompositeParameter* parameter = iterator.getCurrentElement();
    if ( parameter->getValue() == _pointName )
      return;
    iterator.next();
  }

  utilx::runtime_cfg::CompositeParameter* pointParam =
      pointDefs.addParameter(new utilx::runtime_cfg::CompositeParameter("point", _pointName));

  pointParam->addParameter(new utilx::runtime_cfg::Parameter("lpc", _pointCode));
  pointParam->addParameter(new utilx::runtime_cfg::Parameter("ni", _ni));
  pointParam->addParameter(new utilx::runtime_cfg::Parameter("standard", _standard));

  mtp3::PointsDefinitionRegistry::getInstance().insert(mtp3::PointInfo(_ni, _standard),
                                                       _pointCode);

  RootLayerCommandsInterpreter::configurationWasModified = true;
}

std::string
LM_Points_AddPoint::composeCommandId(const std::string& point_name)
{
  return "points_addPoint_" + point_name;
}

}}}}}
