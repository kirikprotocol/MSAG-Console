/* ************************************************************************** *
 * libSUA API interface definition.
 * ************************************************************************** */
#ifndef __EYELINE_SUA_LIBSUA_SUAAPI_HPP__
#ident "@(#)$Id$"
# define __EYELINE_SUA_LIBSUA_SUAAPI_HPP__

# include "util/config/ConfigView.h"

# include "eyeline/sua/libsua/MessageInfo.hpp"
# include "eyeline/sua/libsua/MessageProperties.hpp"

namespace eyeline {
namespace sua {
namespace libsua {

class SuaApi {
public:
  enum ErrorCode_e { OK = 0,
    //NOTE: bind result related values of SuaApi::ErrorCode_e biuniquely conform
    //to corresponding BindConfirmMessage::BindResult_e values !!!
    BIND_CONFIRM_UNKNOWN_APP_ID_VALUE = 1, BIND_CONFIRM_UNSUPPORTED_PROTOCOL_VERSION = 2,
    BIND_CONFIRM_COMPONENT_IS_INACTIVE = 3, SUA_NOT_INITIALIZED = 4, NOT_CONNECTED = 5,
    NOT_BINDED = 6, WRONG_CONNECT_NUM = 7, GOT_TOO_LONG_MESSAGE = 8, SOCKET_TIMEOUT = 9,
    ALREADY_BINDED = 10, SYSTEM_MALFUNCTION = 255, SYSTEM_ERROR = -1
  };

  virtual ~SuaApi() {}

  virtual ErrorCode_e sua_init(smsc::util::config::ConfigView* config) = 0;

  virtual ErrorCode_e sua_close() = 0;

  virtual ErrorCode_e sua_connect(unsigned int suaConnectNum) = 0;

  virtual ErrorCode_e sua_disconnect(unsigned int suaConnectNum) = 0;

  virtual ErrorCode_e bind(unsigned int suaConnectNum) = 0;

  virtual ErrorCode_e unbind(unsigned int suaConnectNum) = 0;

  struct CallResult {
    CallResult(ErrorCode_e anOperationResult, unsigned int aSuaConnectNum)
      : operationResult(anOperationResult), suaConnectNum(aSuaConnectNum)
    {}

    ErrorCode_e operationResult;
    unsigned int suaConnectNum;
  };

  virtual CallResult unitdata_req(const uint8_t* message,
                                  uint16_t messageSize,
                                  const uint8_t* calledAddr,
                                  uint8_t calledAddrLen,
                                  const uint8_t* callingAddr,
                                  uint8_t callingAddrLen,
                                  const MessageProperties& msgProperties) = 0;

  virtual CallResult unitdata_req(const uint8_t* message,
                                  uint16_t messageSize,
                                  const uint8_t* calledAddr,
                                  uint8_t calledAddrLen,
                                  const uint8_t* callingAddr,
                                  uint8_t callingAddrLen,
                                  const MessageProperties& msgProperties,
                                  unsigned int suaConnectNum) = 0;

  virtual ErrorCode_e msgRecv(MessageInfo* msgInfo, uint32_t timeout=0) = 0;

  virtual size_t sua_getConnectsCount() const = 0;
};

}}}

#endif /* __EYELINE_SUA_LIBSUA_SUAAPI_HPP__ */

