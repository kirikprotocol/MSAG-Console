/* ************************************************************************** *
 * libSCCP API interface definition.
 * ************************************************************************** */
#ifndef __EYELINE_SS7NA_LIBSCCP_SCCPAPI_HPP__
#ident "@(#)$Id$"
# define __EYELINE_SS7NA_LIBSCCP_SCCPAPI_HPP__

# include "eyeline/ss7na/libsccp/MessageInfo.hpp"
# include "eyeline/ss7na/libsccp/MessageProperties.hpp"
# include "eyeline/ss7na/libsccp/SccpConfig.hpp"

namespace eyeline {
namespace ss7na {
namespace libsccp {

class SccpApi {
public:
  enum ErrorCode_e { OK = 0,
    //NOTE: bind result related values of SuaApi::ErrorCode_e biuniquely conform
    //to corresponding BindConfirmMessage::BindResult_e values !!!
    BIND_CONFIRM_UNKNOWN_APP_ID_VALUE = 1, BIND_CONFIRM_UNSUPPORTED_PROTOCOL_VERSION = 2,
    BIND_CONFIRM_COMPONENT_IS_INACTIVE = 3, ACTIVE_APP_ALREADY_EXISTS = 4,
    LIB_NOT_INITIALIZED = 5, NOT_CONNECTED = 6, NOT_BINDED = 7, WRONG_CONNECT_NUM = 8,
    GOT_TOO_LONG_MESSAGE = 9, SOCKET_TIMEOUT = 10, ALREADY_BINDED = 11,
    GOT_UNKOWN_MESSAGE = 12, SYSTEM_MALFUNCTION = 255, SYSTEM_ERROR = -1
    // Note: value ACTIVE_APP_ALREADY_EXISTS is returned when second application with
    // the same appId attempt to connect. Value ALREADY_BINDED is returned when application
    // call bind() second time.
  };

  virtual ~SccpApi() {}

  //Throws in case of unexpected init() call
  virtual ErrorCode_e init(const SccpConfig & sua_cfg) /*throw(std::exception)*/ = 0;

  virtual ErrorCode_e close(void) = 0;

  //NOTE: 'connect_num' is an index of SCSPLink from SccpConfig._links array
  virtual ErrorCode_e connect(unsigned int connect_num) = 0;

  virtual ErrorCode_e disconnect(unsigned int connect_num) = 0;

  //Instructs the SCCP Service Provider that given connect serves (is
  //binded to) specified SubSystems. In case of success, SCCP provider
  //asigns SCCPAddress to this connect.
  virtual ErrorCode_e bind(unsigned int connect_num, const uint8_t * ssn_list, uint8_t ssn_list_sz) = 0;
  //
  virtual ErrorCode_e unbind(unsigned int connect_num) = 0;

  //Returns number of configured connects
  virtual unsigned int getConnectsCount(void) const = 0;
  //Returns non OK in case of unknown 'connect_num'
  virtual ErrorCode_e getConnectState(SCSPLinkState & link_state, unsigned int connect_num) const = 0;
  //Returns non OK in case of unknown 'connect_num'
  virtual ErrorCode_e getConnectInfo(SCSPLink & link_info, unsigned int connect_num) const = 0;


  struct CallResult {
    CallResult(ErrorCode_e operation_result, unsigned int connect_num)
      : operationResult(operation_result), connectNum(connect_num)
    {}

    ErrorCode_e operationResult;
    unsigned int connectNum;
  };

  virtual CallResult unitdata_req(const uint8_t* message,
                                  uint16_t message_size,
                                  const uint8_t* called_addr,
                                  uint8_t called_addr_len,
                                  const uint8_t* calling_addr,
                                  uint8_t calling_addr_len,
                                  const MessageProperties& msg_properties) = 0;

  CallResult unitdata_req(const uint8_t* message,
                          uint16_t message_size,
                          const sccp::SCCPAddress& called_addr,
                          const sccp::SCCPAddress& calling_addr,
                          const MessageProperties& msg_properties);

  virtual ErrorCode_e unitdata_req(const uint8_t* message,
                                  uint16_t message_size,
                                  const uint8_t* called_addr,
                                  uint8_t called_addr_len,
                                  const uint8_t* calling_addr,
                                  uint8_t calling_addr_len,
                                  const MessageProperties& msg_properties,
                                  unsigned int connect_num) = 0;

  ErrorCode_e unitdata_req(const uint8_t* message,
                          uint16_t message_size,
                          const sccp::SCCPAddress& called_addr,
                          const sccp::SCCPAddress& calling_addr,
                          const MessageProperties& msg_properties,
                          unsigned int connect_num);

  virtual ErrorCode_e msgRecv(MessageInfo* msg_info, uint32_t timeout=0) = 0;

};

}}}

#endif /* __EYELINE_SS7NA_LIBSCCP_SCCPAPI_HPP__ */

