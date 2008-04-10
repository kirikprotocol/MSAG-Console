#include "initializer.hpp"
#include "EncapsulatedSuaMessage.hpp"
#include "BindMessage.hpp"
#include "UnbindMessage.hpp"
#include "MErrorMessage.hpp"

#include <sua/sua_layer/io_dispatcher/SuaLayerMessagesFactory.hpp>

namespace libsua_messages {

bool
initialize ()
{
  LibsuaMessage::registerMessageCode(BindMessage().getMsgCode());

  for(uint32_t msgCode=EncapsulatedSuaMessage::_MIN_MSG_CODE; msgCode <= EncapsulatedSuaMessage::_MAX_MSG_CODE; ++msgCode)
    LibsuaMessage::registerMessageCode(msgCode);

  LibsuaMessage::registerMessageCode(MErrorMessage().getMsgCode());

  LibsuaMessage::registerMessageCode(UnbindMessage().getMsgCode());

  return true;
}

}
