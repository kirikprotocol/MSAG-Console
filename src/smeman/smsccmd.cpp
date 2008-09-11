#include "smeman/smsccmd.h"
#include "util/config/Manager.h"

namespace smsc{
namespace smeman{

bool SmscCommand::standardErrorCodes=false;

uint32_t SmscCommand::makeSmppStatus(uint32_t status)
{
  if(!standardErrorCodes)
  {
    return status;
  }
  if(status<=260)
  {
    return status;
  }
  using namespace smsc::system;
  switch(status)
  {
    case Status::NOROUTE:return SmppStatusSet::ESME_RINVDSTADR;
    case Status::MSMEMCAPACITYFULL:return SmppStatusSet::ESME_RMSGQFUL;
    default:
      return Status::isErrorPermanent(status)?SmppStatusSet::ESME_RX_P_APPN:SmppStatusSet::ESME_RX_T_APPN;
  }
}


}
}

