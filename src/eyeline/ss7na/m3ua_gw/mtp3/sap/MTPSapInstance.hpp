#ifndef __EYELINE_SS7NA_M3UAGW_MTP3_SAP_MTPSAPINSTANCE_HPP__
# define __EYELINE_SS7NA_M3UAGW_MTP3_SAP_MTPSAPINSTANCE_HPP__

# include "logger/Logger.h"
# include "eyeline/utilx/Singleton.hpp"

# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/m3ua_gw/io_dispatcher/ConnectMgr.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/DATAMessage.hpp"

# include "eyeline/ss7na/m3ua_gw/mtp3/primitives/MTP_Pause_Ind.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/primitives/MTP_Resume_Ind.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/primitives/MTP_Status_Ind.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/primitives/MTP_Transfer.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace sap {

class MTPSapInstance : public utilx::Singleton<MTPSapInstance> {
public:
  void handle(const primitives::MTP_Pause_Ind& mtp_primitive);
  void handle(const primitives::MTP_Resume_Ind& mtp_primitive);
  void handle(const primitives::MTP_Status_Ind& mtp_primitive);
  void handle(const primitives::MTP_Transfer_Req& mtp_primitive);
  void handle(const primitives::MTP_Transfer_Ind& mtp_primitive);

protected:
  MTPSapInstance()
    : _logger(smsc::logger::Logger::getInstance("mtp3_sap")),
      _cMgr(io_dispatcher::ConnectMgr::getInstance())
  {}
  friend class utilx::Singleton<MTPSapInstance>;

  void sendMessage(const m3ua_stack::messages::DATAMessage& message,
                   const common::LinkId& out_link_id);
private:
  smsc::logger::Logger* _logger;
  common::io_dispatcher::ConnectMgr& _cMgr;
};

}}}}}

#endif
