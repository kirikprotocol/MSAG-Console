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
//  for(unsigned i = 0; i < _totalNumOfLPCs; ++i) {
//    if ( _localPointCodes[i] == dpc )
//      return true;
//  }
//  return false;
}

/*void
Descriminator::addLocalPointCode(uint32_t pc)
{
  if ( _totalNumOfLPCs == _maxNumOfLPCs ) {
    _maxNumOfLPCs <<= 2;
    uint32_t* lpcMem = new uint32_t[_maxNumOfLPCs];
    std::copy(_localPointCodes, _localPointCodes + _totalNumOfLPCs, lpcMem);
    if (_isBufferOnHeap )
      delete [] _localPointCodes;
    _localPointCodes = lpcMem; _isBufferOnHeap = true;
  }
  _localPointCodes[_totalNumOfLPCs++] = pc;
}
*/
}}}}}
