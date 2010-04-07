#include "MTPSapInstance.hpp"

#include "eyeline/ss7na/common/Exception.hpp"
#include "eyeline/ss7na/common/io_dispatcher/Reconnector.hpp"
#include "eyeline/ss7na/m3ua_gw/types.hpp"
#include "eyeline/ss7na/m3ua_gw/sccp/MTP3IndicationsProcessor.hpp"
#include "eyeline/ss7na/m3ua_gw/point_status_mgmt/PointStatusTable.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/msu_processor/Router.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace sap {

void
MTPSapInstance::handle(const primitives::MTP_Pause_Ind& mtp_primitive)
{
  smsc_log_info(_logger, "MTPSapInstance::handle::: got MTP_Pause_Ind='%s'",
                mtp_primitive.toString().c_str());
  point_status_mgmt::PointStatusTable::getInstance().updateStatus(mtp_primitive.getAffectedDPC(),
                                                                  point_status_mgmt::POINT_PROHIBITED);
  sccp::MTP3IndicationsProcessor::getInstance().handle(mtp_primitive);
}

void
MTPSapInstance::handle(const primitives::MTP_Resume_Ind& mtp_primitive)
{
  smsc_log_info(_logger, "MTPSapInstance::handle::: got MTP_Resume_Ind='%s'",
                mtp_primitive.toString().c_str());
  point_status_mgmt::PointStatusTable::getInstance().updateStatus(mtp_primitive.getAffectedDPC(),
                                                                  point_status_mgmt::POINT_ALLOWED);
  sccp::MTP3IndicationsProcessor::getInstance().handle(mtp_primitive);
}

void
MTPSapInstance::handle(const primitives::MTP_Status_Ind& mtp_primitive)
{
  smsc_log_info(_logger, "MTPSapInstance::handle::: got MTP_Status_Ind='%s'",
                mtp_primitive.toString().c_str());

  point_status_mgmt::PointStatusTable::getInstance().updateStatus(mtp_primitive.getAffectedDPC(),
                                                                  point_status_mgmt::POINT_PROHIBITED);
  sccp::MTP3IndicationsProcessor::getInstance().handle(mtp_primitive);
}

void
MTPSapInstance::handle(const primitives::MTP_Transfer_Req& mtp_primitive)
{
  smsc_log_info(_logger, "MTPSapInstance::handle::: got MTP_Transfer_Req='%s'",
                mtp_primitive.toString().c_str());
  common::point_code_t opc = mtp_primitive.getOPC(), dpc = mtp_primitive.getDPC();
  point_status_mgmt::point_status_e poinStatus =
      point_status_mgmt::PointStatusTable::getInstance().checkStatus(dpc);
  if ( poinStatus != point_status_mgmt::POINT_ALLOWED )
    throw common::MTP3RouteNotFound("MTPSapInstance::handle::: traffic to point with code=[%u] is not allowed, status=[%u]",
                                    dpc, poinStatus);

  const common::LinkId& outLinkId =
      msu_processor::Router::getInstance().route(opc, dpc);

  m3ua_stack::messages::DATAMessage message;
  m3ua_stack::messages::TLV_ProtocolData protocolData;
  protocolData.setOPC(opc);
  protocolData.setDPC(dpc);
  protocolData.setMessagePriority(mtp_primitive.getMsgPrio());
  protocolData.setNetworkIndicator(mtp_primitive.getNetworkIndicator());
  protocolData.setServiceIndicator(mtp_primitive.getServiceIndicator());
  protocolData.setSLS(mtp_primitive.getSLS());
  utilx::variable_data_t data = mtp_primitive.getData();
  protocolData.setUserProtocolData(data.data, data.dataLen);
  message.setProtocolData(protocolData);
  sendMessage(message, outLinkId);
}

void
MTPSapInstance::handle(const primitives::MTP_Transfer_Ind& mtp_primitive)
{
  sccp::MTP3IndicationsProcessor::getInstance().handle(mtp_primitive);
}

void
MTPSapInstance::sendMessage(const m3ua_stack::messages::DATAMessage& message,
                            const common::LinkId& out_link_id)
{
  try {
    _cMgr.send(out_link_id, message);
  } catch (corex::io::BrokenPipe& ex) {
    smsc_log_error(_logger, "MTPSapInstance::handle::: caught BrokenPipe exception when processing MTP_Transfer_Req primitive");
    primitives::MTP_Pause_Ind mtpPauseInd(message.getProtocolData().getDPC());
    sccp::MTP3IndicationsProcessor::getInstance().handle(mtpPauseInd);
    common::io_dispatcher::Reconnector::getInstance().schedule(_cMgr.removeLink(out_link_id));
    throw;
  } catch (utilx::CongestionException& ex) {
    primitives::MTP_Status_Ind mtpStatusInd(message.getProtocolData().getDPC(), SignallingNetworkCongestion);

    smsc_log_error(_logger, "MTPSapInstance::handle::: caught CongestionException='%s' when processing MTP_Transfer_Req primitive",
                   ex.what());
    sccp::MTP3IndicationsProcessor::getInstance().handle(mtpStatusInd);
    throw;
  } catch (common::io_dispatcher::ProtocolException& ex) {
    smsc_log_error(_logger, "MTPSapInstance::handle::: caught ProtocolException='%s' when processing MTP_Transfer_Req primitive",
                   ex.what());
    primitives::MTP_Pause_Ind mtpPauseInd(message.getProtocolData().getDPC());
    sccp::MTP3IndicationsProcessor::getInstance().handle(mtpPauseInd);
    throw;
  } catch (common::RouteNotFound& ex) {
    smsc_log_error(_logger, "MTPSapInstance::handle::: caught RouteNotFound exception when processing MTP_Transfer_Req primitive",
                       ex.what());
    primitives::MTP_Pause_Ind mtpPauseInd(message.getProtocolData().getDPC());
    sccp::MTP3IndicationsProcessor::getInstance().handle(mtpPauseInd);
    throw;
  }

}

}}}}}
