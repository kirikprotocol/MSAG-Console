#ifndef SCAG_SYSTEM_SMSCSIGNALHANDLERS
#define SCAG_SYSTEM_SMSCSIGNALHANDLERS

#include <signal.h>

//#include <admin/smsc_service//SmscComponent.h>
#include "admin/service/ServiceSocketListener.h"
#include "scag/admin/SCAGCommandDispatcher.h"
#include "scag/admin/SCAGSocketListener.h"

namespace scag {

using namespace scag::admin;

class Scag;

static const int SHUTDOWN_SIGNAL = SIGTERM;


void registerScagSignalHandlers(Scag * scag);
void registerScagSignalHandlers(SCAGSocketListener * socketListener);
void clearThreadSignalMask();

}

#endif //#ifndef SMSC_SYSTEM_SMSCSIGNALHANDLERS
