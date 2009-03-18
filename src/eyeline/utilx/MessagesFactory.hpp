#ifndef __EYELINE_UTILX_MESSAGESFACTORY_HPP__
# define __EYELINE_UTILX_MESSAGESFACTORY_HPP__

# include <string.h>

namespace eyeline {
namespace utilx {

template <class MESSAGE>
class MessageCreator {
public:
  virtual MESSAGE* instance() = 0;
};

template <class MESSAGE, unsigned int MSG_NUMS>
class MessagesFactoryTmpl {
public:
  MessagesFactoryTmpl() {
    memset(reinterpret_cast<uint8_t*>(_registredInstancers), 0, sizeof(MessageCreator<MESSAGE>*) * MSG_NUMS);
  }
  virtual MESSAGE* instanceMessage(unsigned int msgIdx) {
    if ( msgIdx >= MSG_NUMS ) 
      smsc::util::Exception("MessagesFactoryTmpl::instanceMessage::: invalid msgIdx=%d value, accceptable value is from interval [0..%d]", msgIdx, MSG_NUMS-1);

    MessageCreator<MESSAGE>* msgCreator = _registredInstancers[msgIdx];
    if ( msgCreator ) return msgCreator->instance();
    else throw smsc::util::Exception("MessagesFactoryTmpl::instanceMessage::: can't find message creator for message with idx=%d", msgIdx);
  }
  virtual void registerMessageCreator(unsigned int msgIdx, MessageCreator<MESSAGE>* msgCreator) {
    if ( !_registredInstancers[msgIdx] ) _registredInstancers[msgIdx] = msgCreator;
  }
protected:
  MessageCreator<MESSAGE>* _registredInstancers[MSG_NUMS];
};

}}

#endif
