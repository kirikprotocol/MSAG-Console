#ifndef SMSC_SYSTEM_SMSCSIGNALHANDLERS
#define SMSC_SYSTEM_SMSCSIGNALHANDLERS

#include <system/smsc.hpp>
#include <admin/smsc_service//SmscComponent.h>
#include <admin/service/ServiceSocketListener.h>

namespace smsc {
namespace system {

using smsc::system::Smsc;
using smsc::admin::smsc_service::SmscComponent;
using smsc::admin::service::ServiceSocketListener;

void registerSmscSignalHandlers(Smsc * smsc);
void registerSmscSignalHandlers(SmscComponent * smscComponent, ServiceSocketListener* socketListener);
void clearThreadSignalMask();

}
}

#endif //#ifndef SMSC_SYSTEM_SMSCSIGNALHANDLERS
