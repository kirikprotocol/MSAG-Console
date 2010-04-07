#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_SAP_MESSAGEHANDLERS_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_SAP_MESSAGEHANDLERS_HPP__

# include "logger/Logger.h"
# include "core/synchronization/Mutex.hpp"
# include "eyeline/ss7na/common/sccp_sap/MessageHandlers.hpp"
# include "eyeline/ss7na/m3ua_gw/io_dispatcher/ConnectMgr.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/sap/MTPSapInstance.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/UDT.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/XUDT.hpp"
# include "eyeline/ss7na/libsccp/messages/N_UNITDATA_REQ_Message.hpp"
# include "eyeline/ss7na/libsccp/messages/N_UNITDATA_IND_Message.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace sap {

class MessageHandlers : public common::sccp_sap::MessageHandlers {
public:
  explicit MessageHandlers()
  : _logger(smsc::logger::Logger::getInstance("sccp_sap")),
    _cMgr(io_dispatcher::ConnectMgr::getInstance())
  {}

  virtual void handle(const libsccp::BindMessage& message, const common::LinkId& link_id);
  virtual void handle(const libsccp::UnbindMessage& message, const common::LinkId& link_id);
  virtual void handle(const libsccp::N_UNITDATA_REQ_Message& message, const common::LinkId& link_id);

  enum { MAX_HOP_COUNTER_VALUE = 15 };

protected:
  bool decideIfNeedSegmentation(const libsccp::N_UNITDATA_REQ_Message& message);
  void doSegmentation(const libsccp::N_UNITDATA_REQ_Message& message);
  sccp::messages::XUDT& formXUDT(const libsccp::N_UNITDATA_REQ_Message& message);
  sccp::messages::XUDT& formXUDT(const libsccp::N_UNITDATA_REQ_Message& message,
                                 uint16_t next_data_part_start, uint16_t next_data_part_sz,
                                 uint8_t remaining_segments, uint32_t local_ref);

  sccp::messages::UDT& formUDT(const libsccp::N_UNITDATA_REQ_Message& message);
  void formNegativeResponse(const libsccp::N_UNITDATA_REQ_Message& message,
                            common::return_cause_value_t return_cause,
                            const common::LinkId& link_id);

  enum authentication_res_e {
    AUTHENTICATION_OK = 0, AUTHENTICATION_FAILED = -1
  };

  authentication_res_e makeAuthentication(const std::string& app_id);

  static uint32_t getUniqueLocaReference();
  static smsc::core::synchronization::Mutex _uniqRefLock;
  static uint32_t _uniqueLocaReference;

private:
  smsc::logger::Logger* _logger;
  common::io_dispatcher::ConnectMgr& _cMgr;

  enum {
    XUDT_FIXED_FIELDS_LEN =3,
    IMPORTANCE_VALUE_LEN = 1,
    MAX_XUDT_LEN = 272,
    MAX_USERDATA_SIZE = 3952,
    OPTIONAL_PART_PTR_SIZE = 1,
    OPTIONAL_PARAM_FIXED_PREFIX_LEN = 2, /* one octet for parameter name and one for length indicator*/
    MANDATORY_VARIABLE_PARAM_OVERHEAD = 2 /* one octet for pointer value and one for length field */,
    END_OF_OPTIONAL_PARAM_SUFFIX_LEN = 1, SEGMENTATION_PARAM_LEN = 6
  };
};

}}}}}

#endif
