#ifndef __SUA_LIBSUA_SUAAPI_HPP__
# define __SUA_LIBSUA_SUAAPI_HPP__ 1

# include <sys/types.h>
# include <util/config/ConfigView.h>
# include <sua/communication/sua_messages/PointCode.hpp>

# include <sua/libsua/types.hpp>
# include <sua/libsua/MessageInfo.hpp>
# include <sua/libsua/MessageProperties.hpp>

namespace libsua {

enum { OK = 0, BIND_CONFIRM_UNKNOWN_APP_ID_VALUE = 1, BIND_CONFIRM_UNSUPPORTED_PROTOCOL_VERSION = 2,
       BIND_CONFIRM_COMPONENT_IS_INACTIVE = 3, SUA_NOT_INITIALIZED = 4, NOT_CONNECTED = 5,
       NOT_BINDED = 6, WRONG_CONNECT_NUM = 7, GOT_TOO_LONG_MESSAGE = 8, SYSTEM_MALFUNCTION = 255,
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

  virtual int sua_getConnectsCount() const = 0;
};

}

#endif
