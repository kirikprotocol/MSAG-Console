#ifndef SMSC_SYSTEM_SMSCSIGNALHANDLERS
#define SMSC_SYSTEM_SMSCSIGNALHANDLERS

#include <signal.h>

//#include <admin/smsc_service//SmscComponent.h>
#include "admin/service/ServiceSocketListener.h"
#include "smppgw/admin/SmppGwCommandDispatcher.h"
#include "smppgw/admin/SmppGwSocketListener.h"

namespace smsc {
namespace smppgw {

using namespace smsc::smppgw::admin;

class Smsc;

static const int SHUTDOWN_SIGNAL = SIGTERM;


void registerSmscSignalHandlers(Smsc * smsc);
void registerSmscSignalHandlers(SmppGwSocketListener * socketListener);
void clearThreadSignalMask();

}
}

#endif //#ifndef SMSC_SYSTEM_SMSCSIGNALHANDLERS
