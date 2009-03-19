#ifndef __EYELINE_SUA_LIBSUA_SUAAPI_HPP__
# define __EYELINE_SUA_LIBSUA_SUAAPI_HPP__

# include <sys/types.h>
# include <util/config/ConfigView.h>

# include <eyeline/sua/libsua/types.hpp>
# include <eyeline/sua/libsua/MessageInfo.hpp>
# include <eyeline/sua/libsua/MessageProperties.hpp>
# include <eyeline/sua/communication/sua_messages/PointCode.hpp>

namespace eyeline {
namespace sua {
namespace libsua {

enum { OK = 0, BIND_CONFIRM_UNKNOWN_APP_ID_VALUE = 1, BIND_CONFIRM_UNSUPPORTED_PROTOCOL_VERSION = 2,
       BIND_CONFIRM_COMPONENT_IS_INACTIVE = 3, SUA_NOT_INITIALIZED = 4, NOT_CONNECTED = 5,
       NOT_BINDED = 6, WRONG_CONNECT_NUM = 7, GOT_TOO_LONG_MESSAGE = 8, SOCKET_TIMEOUT = 9,
       ALREADY_BINDED = 10, SYSTEM_MALFUNCTION = 255,
       SYSTEM_ERROR = -1 };

class SuaApi {
public:
  virtual ~SuaApi() {}

  virtual int sua_init(smsc::util::config::ConfigView* config) = 0;

  virtual int sua_close() = 0;

  virtual int sua_connect(unsigned int suaConnectNum) = 0;

  virtual int sua_disconnect(unsigned int suaConnectNum) = 0;

  virtual int bind(unsigned int suaConnectNum) = 0;

  virtual int unbind(unsigned int suaConnectNum) = 0;

  virtual int unitdata_req(const uint8_t* message,
                           uint16_t messageSize,
                           const uint8_t* calledAddr,
                           uint8_t calledAddrLen,
                           const uint8_t* callingAddr,
                           uint8_t callingAddrLen,
                           const MessageProperties& msgProperties,
                           unsigned int suaConnectNum) = 0;

  virtual int msgRecv(MessageInfo* msgInfo, uint32_t timeout=0) = 0;

  virtual size_t sua_getConnectsCount() const = 0;
};

}}}

#endif
