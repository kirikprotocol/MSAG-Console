#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_MTP3INDICATIONSPROCESSOR_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_MTP3INDICATIONSPROCESSOR_HPP__

# include <sys/types.h>
# include "logger/Logger.h"
# include "eyeline/ss7na/m3ua_gw/io_dispatcher/ConnectMgr.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/primitives/MTP_Pause_Ind.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/primitives/MTP_Resume_Ind.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/primitives/MTP_Status_Ind.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/primitives/MTP_Transfer.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/sap/MTPSapInstance.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/SCCPMessage.hpp"
# include "eyeline/ss7na/libsccp/messages/LibsccpMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {

class MTP3IndicationsProcessor : public utilx::Singleton<MTP3IndicationsProcessor> {
public:
  MTP3IndicationsProcessor()
  : _logger(smsc::logger::Logger::getInstance("sccp")),
    _cMgr(io_dispatcher::ConnectMgr::getInstance())
  {}

  //  void inititialize(mtp3::sap::MTPSapInstance* mtp_instance) { _mtp3SapInstance = mtp_instance; }
  void handle(const mtp3::primitives::MTP_Pause_Ind& mtp_primitive);
  void handle(const mtp3::primitives::MTP_Resume_Ind& mtp_primitive);
  void handle(const mtp3::primitives::MTP_Status_Ind& mtp_primitive);
  void handle(const mtp3::primitives::MTP_Transfer_Ind& mtp_primitive);

protected:
  void formMtpTransferReq(const mtp3::primitives::MTP_Transfer_Ind& mtp_ind_primitive,
                          const messages::SCCPMessage& negative_message);
  void broadcastIndication(libsccp::LibsccpMessage& libsccp_msg);

private:
  smsc::logger::Logger* _logger;
  common::io_dispatcher::ConnectMgr& _cMgr;
};

}}}}

#endif
