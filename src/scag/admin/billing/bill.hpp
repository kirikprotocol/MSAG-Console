#ifndef __SMSC_SCAG_BILLING_BILLING__
#define __SMSC_SCAG_BILLING_BILLING__

#include "interface.hpp"

namespace smsc{
namespace scag{
namespace billing{

void InitBillingInterface(const char* billingModule);
IBillingInterface* GetBillingInterface();

}//billing
}//scag
}//smsc


#endif
