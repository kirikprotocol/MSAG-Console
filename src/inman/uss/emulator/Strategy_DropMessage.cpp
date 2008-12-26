#include "Strategy_DropMessage.hpp"

namespace smsc  {
namespace inman {
namespace uss  {

// int
// Strategy_DropMessage::Execute()
// {
//   smsc_log_debug(_logger, "Strategy_DropMessage::Execute::: reject message [%s]", _requestObject.toString().c_str());

//   return 0;
// }

void
Strategy_DropMessage::handle()
{
  smsc_log_debug(_logger, "Strategy_DropMessage::handle::: reject message [%s]", _requestObject.toString().c_str());
}

}}}
