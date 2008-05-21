#ifndef __SUA_LIBSUA_SUAAPI_HPP__
# define __SUA_LIBSUA_SUAAPI_HPP__ 1

# include <sys/types.h>
# include <util/config/ConfigView.h>
# include <sua/communication/sua_messages/PointCode.hpp>

# include <sua/libsua/types.hpp>
# include <sua/libsua/MessageInfo.hpp>
# include <sua/libsua/MessageProperties.hpp>

namespace libsua {

enum { OK = 0, NOT_CONNECTED = 128, NOT_BINDED = 129, NOT_INITIALIZED = 130, WRONG_CONNECT_NUM = 131, GOT_TOO_LONG_MESSAGE = 132 };

class SuaApi {
public:
  virtual ~SuaApi() {}

  virtual void sua_init(smsc::util::config::ConfigView* config) = 0;

  virtual void sua_close() = 0;

  virtual void sua_connect(unsigned int suaConnectNum) = 0;

  virtual void sua_disconnect(unsigned int suaConnectNum) = 0;

  virtual unsigned int bind(unsigned int suaConnectNum) = 0;

  virtual void unbind(unsigned int suaConnectNum) = 0;

  virtual unsigned int unitdata_req(const uint8_t* message,
                                    uint16_t messageSize,
                                    const uint8_t* calledAddr,
                                    uint8_t calledAddrLen,
                                    const uint8_t* callingAddr,
                                    uint8_t callingAddrLen,
                                    const MessageProperties& msgProperties,
                                    unsigned int suaConnectNum) = 0;

  virtual void msgRecv(MessageInfo* msgInfo) = 0;

  virtual unsigned int sua_getConnectsCount() const = 0;
};

}

#endif
