#ifndef __EYELINE_SS7NA_SUAGW_SCCPSAP_MESSAGES_NNOTICEINDMESSAGE_HPP__
# define __EYELINE_SS7NA_SUAGW_SCCPSAP_MESSAGES_NNOTICEINDMESSAGE_HPP__

# include <sys/types.h>
# include <string>

# include "eyeline/utilx/types.hpp"
# include "eyeline/ss7na/libsccp/messages/N_NOTICE_IND_Message.hpp"
# include "eyeline/ss7na/libsccp/messages/N_UNITDATA_REQ_Message.hpp"

# include "eyeline/ss7na/sua_gw/sua_stack/messages/SuaTLV.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/CLDRMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/SCCPAddress.hpp"
# include "eyeline/utilx/Exception.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sccp_sap {
namespace messages {

class N_NOTICE_IND_Message : public libsccp::N_NOTICE_IND_Message {
public:
  N_NOTICE_IND_Message(const libsccp::N_UNITDATA_REQ_Message& original_message,
                       uint8_t reason_for_return) {
    utilx::variable_data_t dataParam = original_message.getCalledAddress();
    libsccp::N_NOTICE_IND_Message::setCalledAddress(dataParam.data, static_cast<uint8_t>(dataParam.dataLen));

    dataParam = original_message.getCallingAddress();
    libsccp::N_NOTICE_IND_Message::setCallingAddress(dataParam.data, static_cast<uint8_t>(dataParam.dataLen));

    setReasonForReturn(reason_for_return);

    dataParam = original_message.getUserData();
    setUserData(dataParam.data, dataParam.dataLen);

    try {
      setImportance(original_message.getImportance());
    } catch (utilx::FieldNotSetException& ex) {}
  }

  N_NOTICE_IND_Message(const sua_stack::messages::CLDRMessage& cldr_message) {
    const sua_stack::messages::TLV_DestinationAddress& dstAddress = cldr_message.getDestinationAddress();

    if ( dstAddress.getGlobalTitle().isSetValue() ) {
      if ( dstAddress.getSSN().isSetValue() ) {
        setCalledAddress(sua_stack::messages::SCCPAddress(dstAddress.getGlobalTitle(), dstAddress.getSSN()));
      } else {
        setCalledAddress(sua_stack::messages::SCCPAddress(dstAddress.getGlobalTitle()));
      }
    } else if ( dstAddress.getPointCode().isSetValue() && dstAddress.getSSN().isSetValue() ) {
      setCalledAddress(sua_stack::messages::SCCPAddress(dstAddress.getPointCode(), dstAddress.getSSN()));
    } else {
      throw smsc::util::Exception("N_NOTICE_IND_Message::N_NOTICE_IND_Message::: wrong dstAddress in incoming CLDR message");
    }

    const sua_stack::messages::TLV_SourceAddress& srcAddress = cldr_message.getSourceAddress();

    if ( srcAddress.getGlobalTitle().isSetValue() ) {
      if ( srcAddress.getSSN().isSetValue() ) {
        setCallingAddress(sua_stack::messages::SCCPAddress(srcAddress.getGlobalTitle(), srcAddress.getSSN()));
      } else {
        setCallingAddress(sua_stack::messages::SCCPAddress(srcAddress.getGlobalTitle()));
      }
    } else if ( srcAddress.getPointCode().isSetValue() && srcAddress.getSSN().isSetValue() ) {
      setCallingAddress(sua_stack::messages::SCCPAddress(srcAddress.getPointCode(), srcAddress.getSSN()));
    } else {
      throw smsc::util::Exception("N_NOTICE_IND_Message::N_NOTICE_IND_Message::: wrong srcAddress in incoming CLDR message");
    }

    const sua_stack::messages::TLV_SCCP_Cause& sccpCause = cldr_message.getSCCPCause();
    common::return_cause_type_t sccpCauseType = sccpCause.getCauseType();

    if ( sccpCauseType != common::RETURN_CAUSE )
      throw smsc::util::Exception("N_NOTICE_IND_Message::N_NOTICE_IND_Message::: invalid sccp cause type value = [%d] in incoming CLDR message", sccpCauseType);

    setReasonForReturn(sccpCause.getCauseValue());
    const sua_stack::messages::TLV_Data& userData = cldr_message.getData();
    setUserData(userData.getValue(), userData.getValueLength());
  }

  using libsccp::N_NOTICE_IND_Message::setCalledAddress;
  using libsccp::N_NOTICE_IND_Message::setCallingAddress;

  void setCalledAddress(const sua_stack::messages::SCCPAddress& address)
  {
    _calledAddrLen = static_cast<uint8_t>(address.serialize(_calledAddr, sizeof(_calledAddr)));
  }

  void setCallingAddress(const sua_stack::messages::SCCPAddress& address)
  {
    _callingAddrLen = static_cast<uint8_t>(address.serialize(_callingAddr, sizeof(_callingAddr)));
  }

};

}}}}}

#endif
