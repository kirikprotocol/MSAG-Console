#ifndef __EYELINE_SS7NA_SUAGW_SCCPSAP_MESSAGES_NUNITDATAINDMESSAGE_HPP__
# define __EYELINE_SS7NA_SUAGW_SCCPSAP_MESSAGES_NUNITDATAINDMESSAGE_HPP__

# include <sys/types.h>
# include <string>

# include "eyeline/utilx/types.hpp"
# include "eyeline/ss7na/libsccp/messages/N_UNITDATA_IND_Message.hpp"
# include "eyeline/ss7na/libsccp/messages/N_UNITDATA_REQ_Message.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/CLDTMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/SCCPAddress.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/SuaTLV.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sccp_sap {
namespace messages {

class N_UNITDATA_IND_Message : public libsccp::N_UNITDATA_IND_Message {
public:
  N_UNITDATA_IND_Message(const sua_stack::messages::CLDTMessage& cldt_message) {
    setSequenceControl(cldt_message.getSequenceControl().getValue());

    const sua_stack::messages::TLV_DestinationAddress& dstAddress = cldt_message.getDestinationAddress();

    if ( dstAddress.getGlobalTitle().isSetValue() ) {
      if ( dstAddress.getSSN().isSetValue() ) {
        setCalledAddress(sua_stack::messages::SCCPAddress(dstAddress.getGlobalTitle(), dstAddress.getSSN()));
      } else {
        setCalledAddress(sua_stack::messages::SCCPAddress(dstAddress.getGlobalTitle()));
      }
    } else if ( dstAddress.getPointCode().isSetValue() && dstAddress.getSSN().isSetValue() ) {
      setCalledAddress(sua_stack::messages::SCCPAddress(dstAddress.getPointCode(), dstAddress.getSSN()));
    } else {
      throw smsc::util::Exception("N_UNITDATA_IND_Message::N_UNITDATA_IND_Message::: wrong dstAddress in incoming CLDT message");
    }

    const sua_stack::messages::TLV_SourceAddress& srcAddress = cldt_message.getSourceAddress();

    if ( srcAddress.getGlobalTitle().isSetValue() ) {
      if ( srcAddress.getSSN().isSetValue() ) {
        setCallingAddress(sua_stack::messages::SCCPAddress(srcAddress.getGlobalTitle(), srcAddress.getSSN()));
      } else {
        setCallingAddress(sua_stack::messages::SCCPAddress(srcAddress.getGlobalTitle()));
      }
    } else if ( srcAddress.getPointCode().isSetValue() && srcAddress.getSSN().isSetValue() ) {
      setCallingAddress(sua_stack::messages::SCCPAddress(srcAddress.getPointCode(), srcAddress.getSSN()));
    } else {
      throw smsc::util::Exception("N_UNITDATA_IND_Message::N_UNITDATA_IND_Message::: wrong srcAddress in incoming CLDT message");
    }
    const sua_stack::messages::TLV_Data& userData = cldt_message.getData();
    setUserData(userData.getValue(), userData.getValueLength());
  }

  N_UNITDATA_IND_Message(const libsccp::N_UNITDATA_REQ_Message& n_unitdata_req_message) {
    if ( n_unitdata_req_message.isSetSequenceControl() )
      setSequenceControl(n_unitdata_req_message.getSequenceControl());

    utilx::variable_data_t v_data = n_unitdata_req_message.getCalledAddress();
    libsccp::N_UNITDATA_IND_Message::setCalledAddress(v_data.data, v_data.dataLen);

    v_data = n_unitdata_req_message.getCallingAddress();
    libsccp::N_UNITDATA_IND_Message::setCallingAddress(v_data.data, v_data.dataLen);

    v_data = n_unitdata_req_message.getUserData();
    setUserData(v_data.data, v_data.dataLen);
  }

  using libsccp::N_UNITDATA_IND_Message::setCalledAddress;
  using libsccp::N_UNITDATA_IND_Message::setCallingAddress;

  void setCalledAddress(const sua_stack::messages::SCCPAddress& address) {
    _calledAddrLen = static_cast<uint8_t>(address.serialize(_calledAddr, sizeof(_calledAddr)));
  }

  void setCallingAddress(const sua_stack::messages::SCCPAddress& address) {
    _callingAddrLen = static_cast<uint8_t>(address.serialize(_callingAddr, sizeof(_callingAddr)));
  }
};

}}}}}

#endif
