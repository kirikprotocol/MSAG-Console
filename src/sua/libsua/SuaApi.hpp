#ifndef __SUA_LIBSUA_SUAAPI_HPP__
# define __SUA_LIBSUA_SUAAPI_HPP__ 1

# include <sys/types.h>
# include <util/config/ConfigView.h>
# include <sua/communication/sua_messages/PointCode.hpp>

# include <sua/libsua/types.hpp>
# include <sua/libsua/MessageInfo.hpp>
# include <sua/libsua/MessageProperties.hpp>
# include <sua/libsua/SCCPAddress.hpp>

namespace libsua {

class SuaApi {
public:
  virtual ~SuaApi() {}

  virtual void sua_init(smsc::util::config::ConfigView* config) = 0;

  virtual void sua_close() = 0;

  virtual void sua_connect(unsigned int suaConnectNum) = 0;

  virtual void sua_disconnect(unsigned int suaConnectNum) = 0;

  virtual void sua_bind(unsigned int suaConnectNum) = 0;

  virtual void sua_unbind(unsigned int suaConnectNum) = 0;

  virtual void sua_send_cldt(const uint8_t* message,
                             uint16_t messageSize,
                             const SCCPAddress& srcAddress,
                             const SCCPAddress& dstAddress,
                             const MessageProperties& msgProperties,
                             unsigned int suaConnectNum) = 0;

  virtual void sua_send_scon(const sua_messages::PointCode& pc,
                             uint8_t congestionLevel,
                             unsigned int suaConnectNum) = 0;

  virtual void sua_recvmsg(MessageInfo* msgInfo) = 0;

  virtual unsigned int sua_getConnectsCount() const = 0;
};

}

#endif
