/* ************************************************************************** *
 * libSUA message housekeeping structure.
 * ************************************************************************** */
#ifndef __EYELINE_LIBSCCP_MESSAGEINFO_HPP__
#ident "@(#)$Id$"
# define __EYELINE_LIBSCCP_MESSAGEINFO_HPP__

# include "eyeline/ss7na/libsccp/types.hpp"
# include "core/buffers/ExtendingBuf.hpp"

namespace eyeline {
namespace ss7na {
namespace libsccp {

struct MessageInfo
{
  enum MsgSize_e { DFLT_MSG_SIZE = 8*1024 };
  typedef smsc::core::buffers::ExtendingBuffer<uint8_t, MessageInfo::DFLT_MSG_SIZE>
    msg_buffer_t;

  message_type_t  messageType;
  unsigned int    connectNum;
  msg_buffer_t    msgData;

  MessageInfo(unsigned buf_sz = MessageInfo::DFLT_MSG_SIZE)
    : messageType(0), connectNum(0), msgData(buf_sz)
  { }

  SccpMessageId::MsgCode_e getMsgId(void) const
  {
    return static_cast<SccpMessageId::MsgCode_e>(messageType);
  }
};

}}}

#endif

