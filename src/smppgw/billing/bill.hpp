#ifndef __SMSC_SMPPGW_BILLING_BILLING__
#define __SMSC_SMPPGW_BILLING_BILLING__

#include "interface.hpp"

namespace smsc{
namespace smppgw{
namespace billing{

void InitBillingInterface(const char* billingModule);
IBillingInterface* GetBillingInterface();

}//billing
}//smppgw
}//smsc


#endif
