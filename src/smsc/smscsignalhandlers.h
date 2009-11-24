#ifndef SMSC_SYSTEM_SMSCSIGNALHANDLERS
#define SMSC_SYSTEM_SMSCSIGNALHANDLERS

#include <signal.h>


namespace smsc {

class Smsc;

static const int SHUTDOWN_SIGNAL = SIGTERM;

void registerSmscSignalHandlers(Smsc * smsc);
void clearThreadSignalMask();

}

#endif //#ifndef SMSC_SYSTEM_SMSCSIGNALHANDLERS
