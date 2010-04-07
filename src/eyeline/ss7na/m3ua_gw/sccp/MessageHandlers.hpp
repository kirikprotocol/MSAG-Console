#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGEHANDLERS_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGEHANDLERS_HPP__

# include "logger/Logger.h"
# include "eyeline/ss7na/m3ua_gw/io_dispatcher/ConnectMgr.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/UDT.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/UDTS.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/XUDT.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/XUDTS.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/sap/MTPSapInstance.hpp"
# include "eyeline/ss7na/libsccp/messages/N_NOTICE_IND_Message.hpp"
# include "eyeline/ss7na/libsccp/messages/N_UNITDATA_IND_Message.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {

class MessageHandlers {
public:
  MessageHandlers()
  : _logger(smsc::logger::Logger::getInstance("sccp")),
    _cMgr(io_dispatcher::ConnectMgr::getInstance()),
    _mtp3SapInstance(mtp3::sap::MTPSapInstance::getInstance()),
    _sequenceControl(0), _isSetSequenceControl(false)
  {}

  void handle(messages::UDT& message);
  void handle(messages::UDTS& message);
  void handle(messages::XUDT& message);
  void handle(messages::XUDTS& message);

  void setSequenceControl(uint8_t seq) {
    _sequenceControl = seq; _isSetSequenceControl = true;
  }

protected:
  bool checkMessageIsSCMG(const messages::ProtocolClass& protocol_class,
                          const eyeline::sccp::SCCPAddress& calling_address,
                          const eyeline::sccp::SCCPAddress& called_address);
  bool isSCMGAddrPredicateTrue(const eyeline::sccp::SCCPAddress& address);

  const libsccp::N_UNITDATA_IND_Message* createIndicationMessage(const messages::UDT& message);
  const libsccp::N_UNITDATA_IND_Message* createIndicationMessage(const messages::XUDT& message);
  const libsccp::N_NOTICE_IND_Message* createIndicationMessage(const messages::UDTS& message);
  const libsccp::N_NOTICE_IND_Message* createIndicationMessage(const messages::XUDTS& message);
  const libsccp::N_UNITDATA_IND_Message* createIndicationMessage(const messages::XUDT& message,
                                                                 const uint8_t* assembled_msg_data,
                                                                 uint16_t assembled_msg_data_sz);
  const libsccp::N_NOTICE_IND_Message* createIndicationMessage(const messages::XUDTS& message,
                                                               const uint8_t* assembled_msg_data,
                                                               uint16_t assembled_msg_data_sz);

  bool isNotSegmentedOrIsFirstSegment(const messages::XUDT& message) {
    return !message.isSetSegmentation() ||
            message.getSegmentation().isFirstSegment();
  }

  bool needReassembling(const messages::XUDT& message) { return message.isSetSegmentation(); }
  bool needReassembling(const messages::XUDTS& message) { return message.isSetSegmentation(); }

  template <class MSG>
  void handleRouteOnGT(MSG& message);

  template <class MSG>
  inline
  void forwardMessageToApplication(const std::string& route_id,
                                   const MSG& message);
  template <class MSG>
  inline
  void reassembleAndSend(const std::string& route_id,
                         const MSG& message);

  template <class MSG>
  inline
  void handleRouteOnSSN(const MSG& message);

  void sendNegativeResponse(const messages::UDT& message,
                            common::return_cause_value_t cause);

  void sendNegativeResponse(const messages::XUDT& message,
                            common::return_cause_value_t cause);

  void forwardMessageToMTP3(const std::string& route_id,
                            const messages::SCCPMessage& message);

  enum hop_counter_check_res_e { HOP_COUNTER_CHECK_OK = 0, HOP_COUNTER_VIOLATED = 1 };
  template <class MSG>
  inline
  hop_counter_check_res_e decrementHopCounterAndCheckItViolated(MSG& mssage);

  static uint8_t getNextSsl();

private:
  smsc::logger::Logger* _logger;
  common::io_dispatcher::ConnectMgr& _cMgr;
  mtp3::sap::MTPSapInstance& _mtp3SapInstance;
  uint8_t _sequenceControl;
  bool _isSetSequenceControl;
};

}}}}

# include "eyeline/ss7na/m3ua_gw/sccp/MessageHandlers_impl.hpp"

#endif
