#ifndef __EYELINE_UTILX_MESSAGESFACTORY_HPP__
# define __EYELINE_UTILX_MESSAGESFACTORY_HPP__

# include <string.h>
# include "util/Exception.hpp"
# include "logger/Logger.h"

namespace eyeline {
namespace utilx {

template <class MESSAGE>
class MessageCreator {
public:
  virtual MESSAGE* instance() = 0;
};

template <class MESSAGE, unsigned int MSG_NUMS, unsigned int PROTOCOL_NUMS=1>
class MessagesFactoryTmpl {
public:
  MessagesFactoryTmpl() {
    memset(_registredInstancers, 0, sizeof(_registredInstancers));
    {
      smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("utilx");
      smsc_log_debug(logger, "MessagesFactoryTmpl::MessagesFactoryTmpl::: MSG_NUMS=%u, PROTOCOL_NUMS=%u, sizeof(_registredInstancers)=%u",
                     MSG_NUMS, PROTOCOL_NUMS, sizeof(_registredInstancers));
    }
  }

  MESSAGE* instanceMessage(unsigned int msg_idx, unsigned int protocol_num=0) {
    if ( protocol_num >= PROTOCOL_NUMS )
      throw smsc::util::Exception("MessagesFactoryTmpl::instanceMessage::: invalid protocolNum=%u value, acceptable value is from interval [0..%u]",
                                  protocol_num, PROTOCOL_NUMS-1);
    if ( msg_idx >= MSG_NUMS )
      throw smsc::util::Exception("MessagesFactoryTmpl::instanceMessage::: invalid msgIdx=%u value, acceptable value is from interval [0..%u]",
                                 msg_idx, MSG_NUMS-1);

    MessageCreator<MESSAGE>* msgCreator = _registredInstancers[protocol_num][msg_idx];
    if ( msgCreator ) return msgCreator->instance();
    else throw smsc::util::Exception("MessagesFactoryTmpl::instanceMessage::: can't find message creator for message with protocol=%d/idx=%u",
                                     protocol_num, msg_idx);
  }

  void registerMessageCreator(unsigned int msg_idx, MessageCreator<MESSAGE>* msg_creator, unsigned int protocol_num=0) {
    if ( protocol_num > PROTOCOL_NUMS - 1 )
      throw smsc::util::Exception("MessagesFactoryTmpl::registerMessageCreator::: invalid protocolNum=%u, max protocolNum value=%u",
                                  protocol_num, PROTOCOL_NUMS - 1);

    if ( msg_idx > MSG_NUMS - 1 )
      throw smsc::util::Exception("MessagesFactoryTmpl::registerMessageCreator::: invalid msgIdx=%u, max msgIdx value=%u",
                                  msg_idx, MSG_NUMS - 1);

    {
      smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("utilx");
      smsc_log_debug(logger, "MessagesFactoryTmpl::registerMessageCreator::: msg_idx=%u, protocol_num=%u, msg_creator=0x%p,_registredInstancers[protocol_num][msg_idx]=%p",
                     msg_idx, protocol_num, msg_creator, _registredInstancers[protocol_num][msg_idx]);
    }

    if ( !_registredInstancers[protocol_num][msg_idx] )
      _registredInstancers[protocol_num][msg_idx] = msg_creator;
  }

protected:
  MessageCreator<MESSAGE>* _registredInstancers[PROTOCOL_NUMS][MSG_NUMS];
};

}}

#endif
