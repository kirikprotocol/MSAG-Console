#ifndef __EYELINE_SUA_LIBSUA_MESSAGEINFO_HPP__
# define __EYELINE_SUA_LIBSUA_MESSAGEINFO_HPP__

# include <eyeline/sua/libsua/types.hpp>
# include <eyeline/sua/libsua/MessageProperties.hpp>
# include <eyeline/sua/communication/TP.hpp>
# include <core/buffers/TmpBuf.hpp>

namespace eyeline {
namespace sua {
namespace libsua {

struct MessageInfo
{
  MessageInfo()
    : messageType(0), suaConnectNum(0), msgData(communication::TP::MAX_PACKET_SIZE) {}

  message_type_t messageType;

  unsigned int suaConnectNum;

  typedef smsc::core::buffers::TmpBuf<uint8_t, communication::TP::MAX_PACKET_SIZE> msg_buffer_t;
  msg_buffer_t msgData;
};

}

#endif
