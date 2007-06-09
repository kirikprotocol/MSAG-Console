#ifndef __SMSC_BDB_AGENT_APPLICATIONREQUESTSHANDLER_HPP__
# define __SMSC_BDB_AGENT_APPLICATIONREQUESTSHANDLER_HPP__ 1

# include <mmbox/app_protocol/messages/AppRequest_QueryMmsRecordsByAbonentAddress.hpp>
# include <mmbox/app_protocol/messages/AppRequest_InsertMmsRecord.hpp>
# include <mmbox/app_protocol/messages/AppRequest_QueryMmsRecordsByAbonentAddressAndStatus.hpp>

namespace mmbox {
namespace app_pck_handler {

class ApplicationRequestsHandler : public mmbox::app_protocol::AppRequest_InsertMmsRecord::Handler,
                                   public mmbox::app_protocol::AppRequest_QueryMmsRecordsByAbonentAddress::Handler,
                                   public mmbox::app_protocol::AppRequest_QueryMmsRecordsByAbonentAddressAndStatus::Handler {
public:
  void handleEvent(const mmbox::app_protocol::AppRequest_QueryMmsRecordsByAbonentAddress* appReq,
                   const std::string& sourceConnectId);
  void handleEvent(const mmbox::app_protocol::AppRequest_InsertMmsRecord* appReq,
                   const std::string& sourceConnectId);
  void handleEvent(const mmbox::app_protocol::AppRequest_QueryMmsRecordsByAbonentAddressAndStatus* appReq,
                   const std::string& sourceConnectId);
};

}}

#endif
