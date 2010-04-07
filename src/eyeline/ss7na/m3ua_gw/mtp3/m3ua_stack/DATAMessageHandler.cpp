#include "DATAMessageHandler.hpp"
#include "eyeline/ss7na/common/io_dispatcher/Exceptions.hpp"
#include "eyeline/ss7na/m3ua_gw/io_dispatcher/ConnectMgr.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/msu_processor/Descriminator.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/msu_processor/Distributor.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/msu_processor/Router.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {

DATAMessageHandler::DATAMessageHandler()
  : _logger(smsc::logger::Logger::getInstance("m3ua_stack")),
    _cMgr(io_dispatcher::ConnectMgr::getInstance()) {}

void
DATAMessageHandler::handle(const messages::DATAMessage& message,
                           const common::LinkId& link_id)
{
  smsc_log_debug(_logger, "DATAMessageHandler::handle::: handle DATAMessage [%s]", message.toString().c_str());
  const messages::TLV_ProtocolData& protocolData = message.getProtocolData();
  uint32_t dpc = protocolData.getDPC();
  if ( msu_processor::Descriminator::getInstance().checkRouteTermination(dpc) )
    msu_processor::Distributor::getInstance().dispatch(protocolData);
  else {
    const common::LinkId dstLinkId =
        msu_processor::Router::getInstance().route(protocolData.getOPC(), dpc, link_id);
    _cMgr.send(dstLinkId, message);
  }
}

}}}}}
