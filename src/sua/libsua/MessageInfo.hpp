#ifndef __SUA_LIBSUA_MESSAGEINFO_HPP__
# define __SUA_LIBSUA_MESSAGEINFO_HPP__ 1

# include <sua/libsua/types.hpp>
# include <sua/libsua/MessageProperties.hpp>
# include <sua/communication/TP.hpp>
# include <core/buffers/TmpBuf.hpp>

namespace libsua {

struct MessageInfo
{
  MessageInfo()
    : messageType(0), suaConnectNum(0), msgData(communication::TP::MAX_PACKET_SIZE) {}

  message_type_t messageType;

  unsigned int suaConnectNum;

  typedef smsc::core::buffers::TmpBuf<uint8_t, communication::TP::MAX_PACKET_SIZE> msg_buffer_t;
  msg_buffer_t msgData;

  MessageProperties msgProperties;

};

}

#endif
