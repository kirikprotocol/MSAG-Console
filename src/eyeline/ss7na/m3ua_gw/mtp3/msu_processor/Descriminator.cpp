#include "Descriminator.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/PointsDefinitionRegistry.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace msu_processor {

bool
Descriminator::checkRouteTermination(uint32_t dpc) const
{
  return PointsDefinitionRegistry::getInstance().exists(dpc);
}

}}}}}
