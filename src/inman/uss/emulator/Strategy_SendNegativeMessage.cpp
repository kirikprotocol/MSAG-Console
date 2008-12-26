#include "Strategy_SendNegativeMessage.hpp"

namespace smsc  {
namespace inman {
namespace uss  {

void
Strategy_SendNegativeMessage::handle()
{
  smsc::inman::interaction::SPckUSSResult* resultPacket = new interaction::SPckUSSResult();

  resultPacket->Cmd().setStatus(interaction::USS2CMD::STATUS_USS_REQUEST_FAILED);
  resultPacket->setDialogId(_dialogId);

  _responseWriter->scheduleResponse(resultPacket, _conn);
}

}}}
