#include <vector>
#include "ClusterControllerMsgsHndlr.hpp"

#include "eyeline/protogen/framework/SerializerBuffer.hpp"
#include "eyeline/load_balancer/cluster/messages/GetServicesStatus.hpp"
#include "eyeline/load_balancer/cluster/messages/GetServicesStatusResp.hpp"
#include "eyeline/load_balancer/cluster/messages/DisconnectService.hpp"
#include "eyeline/load_balancer/cluster/messages/DisconnectServiceResp.hpp"
#include "eyeline/load_balancer/cluster/ClusterLoadBalancerProtocolClnt.hpp"
#include "eyeline/load_balancer/protocols/smpp/SmeRegistry.hpp"
#include "eyeline/load_balancer/io_subsystem/IOProcessorMgrRegistry.hpp"

#include "ClusterControllerMgr.hpp"

namespace eyeline {
namespace load_balancer {
namespace cluster {

void
ClusterControllerMsgsHndlr::handle(const messages::GetServicesStatus& msg)
{
  smsc_log_info(_logger, "ClusterControllerMsgsHndlr::handle::: processing GetServicesStatus message [%s]",msg.toString().c_str());
  messages::GetServicesStatusResp responseMsg;
  messages::Response responseResult;

  try {
    std::vector<messages::ServiceStatus> servicesStatusArray;
    getAllSmeStatus(&servicesStatusArray);
    responseMsg.setStatus(servicesStatusArray);
    responseResult.setStatus(RESPONSE_OK);
  } catch (std::exception& ex) {
    responseResult.setStatus(RESPONSE_FAILED);
  }
  responseMsg.setResp(responseResult);

  protogen::framework::SerializerBuffer bufferForSend(512);
  _protocol.encodeMessage(responseMsg, &bufferForSend);

  _mgr.writeBuffer(bufferForSend.getBuffer(), bufferForSend.getDataWritten());
}

void
ClusterControllerMsgsHndlr::handle(const messages::DisconnectService& msg)
{
  smsc_log_info(_logger, "ClusterControllerMsgsHndlr::handle::: processing DisconnectService message [%s]",msg.toString().c_str());
  messages::DisconnectServiceResp responseMsg;
  messages::Response responseResult;

  try {
    disconnectSme(msg.getServiceId());
    responseResult.setStatus(RESPONSE_OK);
  } catch (std::exception& ex) {
    responseResult.setStatus(RESPONSE_FAILED);
  }

  responseMsg.setResp(responseResult);
  protogen::framework::SerializerBuffer bufferForSend(64);
  _protocol.encodeMessage(responseMsg, &bufferForSend);

  _mgr.writeBuffer(bufferForSend.getBuffer(), bufferForSend.getDataWritten());
}

void
ClusterControllerMsgsHndlr::getAllSmeStatus(std::vector<messages::ServiceStatus>* services_status_array)
{
  protocols::smpp::active_sme_list_t activeSmeList;
  protocols::smpp::SmeRegistry::getInstance().getActiveSmeList(&activeSmeList);
  for (protocols::smpp::active_sme_list_t::iterator iter = activeSmeList.begin(), end_iter = activeSmeList.end();
       iter != end_iter; ++iter) {
    messages::ServiceStatus serviceStatus;
    serviceStatus.setBindMode((*iter).bindMode); // "TRX" | "TX" | "RX"
    serviceStatus.setPeerAddress((*iter).peerAddr); // e.g. "192.168.1.200.3333"
    serviceStatus.setServiceName((*iter).systemId);
    serviceStatus.setBoundSmsc(io_subsystem::IOProcessorMgrRegistry::getInstance().getIOProcessorMgr((*iter).ioProcMgrId)->getIOProcessor((*iter).ioProcId)->getLinkSet((*iter).dstLinksetId)->getActivityIndicators());

    //(*iter).dstLinksetId;
//    std::vector<bool> boundSmsc;
//    serviceStatus.setBoundSmsc();

    services_status_array->push_back(serviceStatus);
  }
}

void
ClusterControllerMsgsHndlr::disconnectSme(const std::string& service_id)
{
  protocols::smpp::SmeInfoRef smeInfo = protocols::smpp::SmeRegistry::getInstance().getSmeInfo(service_id);
  smsc_log_debug(_logger, "ClusterControllerMsgsHndlr::disconnectSme::: got smeInfo [bind_type='%s',linkId='%s',systemId='%s', ioProcMgrId=%u] for service_id='%s'",
                 smeInfo->bindRequest->getBindType().c_str(), smeInfo->linkIdToSme.toString().c_str(),
                 smeInfo->systemId.c_str(), smeInfo->ioProcMgrId, service_id.c_str());

  io_subsystem::IOProcessorMgr* ioProcMgr =
      io_subsystem::IOProcessorMgrRegistry::getInstance().getIOProcessorMgr(smeInfo->ioProcMgrId);
  io_subsystem::IOProcessorRefPtr ioProcessor = ioProcMgr->getIOProcessor(smeInfo->ioProcId);
  if ( ioProcessor.Get() )
    ioProcessor->cleanUpConnection(smeInfo->linkIdToSme);
}

}}}
