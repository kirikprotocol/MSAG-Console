#include "SMPPBinder.hpp"
#include "SmeRegistry.hpp"
#include "util/Exception.hpp"
#include "eyeline/load_balancer/io_subsystem/LinkId.hpp"
#include "eyeline/load_balancer/io_subsystem/SwitchingTable.hpp"
#include "eyeline/load_balancer/io_subsystem/IOProcessor.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

void
SMPPBinder::rebind(const io_subsystem::LinkId& link_set_id_to_smsc)
{
  io_subsystem::LinkId linkIdToSme;
  if ( !io_subsystem::SwitchingTable::getInstance().getSwitching(link_set_id_to_smsc, &linkIdToSme) )
    throw smsc::util::Exception("SMPPBinder::rebind::: there isn't related sme link for linkset with id=%s",
                                link_set_id_to_smsc.toString().c_str());

  SmeInfoRef smeInfo = SmeRegistry::getInstance().getSmeInfo(linkIdToSme);
  bind(link_set_id_to_smsc, *smeInfo->bindRequest);
}

}}}}
