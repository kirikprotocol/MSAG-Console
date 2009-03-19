#include "initializer.hpp"
#include "BindMessage.hpp"
#include "UnbindMessage.hpp"
#include "N_UNITDATA_REQ_Message.hpp"

namespace eyeline {
namespace sua {
namespace communication {
namespace libsua_messages {

bool
initialize ()
{
  LibsuaMessage::registerMessageCode(BindMessage().getMsgCode());
  LibsuaMessage::registerMessageCode(N_UNITDATA_REQ_Message().getMsgCode());
  LibsuaMessage::registerMessageCode(UnbindMessage().getMsgCode());

  return true;
}

}}}}
