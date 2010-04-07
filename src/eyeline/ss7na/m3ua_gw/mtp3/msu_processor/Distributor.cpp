#include "Distributor.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/primitives/MTP_Transfer.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/sap/MTPSapInstance.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace msu_processor {

void
Distributor::dispatch(const m3ua_stack::messages::TLV_ProtocolData& protocol_data) const
{
  uint8_t si = protocol_data.getServiceIndicator();
  if ( si == common::SCCP ) {
    primitives::MTP_Transfer_Ind mtpPrimitive(protocol_data.getOPC(), protocol_data.getDPC(),
                                              protocol_data.getSLS(), protocol_data.getServiceIndicator(),
                                              protocol_data.getNetworkIndicator(), protocol_data.getMessagePriority(),
                                              protocol_data.getUserProtocolData());

    _mtp3SapInstance.handle(mtpPrimitive);
  } else if ( si == common::ISUP ) {
    smsc_log_error(_logger, "Distributor::dispatch::: ISUP isn't supported now. Drop MSU",
                   protocol_data.toString().c_str(), si);
  } else
    smsc_log_error(_logger, "Distributor::dispatch::: got msu ='%s' with unsupported service indicator value=%u. Drop MSU",
                   protocol_data.toString().c_str(), si);
}

}}}}}
