/* ************************************************************************** *
 * libSUA message housekeeping structure.
 * ************************************************************************** */
#ifndef __EYELINE_SUA_LIBSUA_MESSAGEINFO_HPP__
#ident "@(#)$Id$"
# define __EYELINE_SUA_LIBSUA_MESSAGEINFO_HPP__

# include "eyeline/sua/libsua/SuaMsgIdent.hpp"
# include "core/buffers/ExtendingBuf.hpp"

namespace eyeline {
namespace sua {
namespace libsua {

struct MessageInfo
{
  enum MsgSize_e { DFLT_MSG_SIZE = 8*1024 };
  typedef smsc::core::buffers::ExtendingBuffer<uint8_t, MessageInfo::DFLT_MSG_SIZE>
    msg_buffer_t;

  message_type_t  messageType;
  unsigned int    suaConnectNum;
  msg_buffer_t    msgData;

  MessageInfo(unsigned buf_sz = MessageInfo::DFLT_MSG_SIZE)
    : messageType(0), suaConnectNum(0), msgData(buf_sz)
  { }

  SUAMessageId::MsgCode_e getMsgId(void) const
  {
    return static_cast<SUAMessageId::MsgCode_e>(messageType);
  }
};

} //libsua
} //sua
} //eyeline

#endif /* __EYELINE_SUA_LIBSUA_MESSAGEINFO_HPP__ */

