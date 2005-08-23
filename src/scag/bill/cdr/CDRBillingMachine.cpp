#include "CDRBillingMachine.h"

std::auto_ptr<scag::bill::CDRBillingMachine> billingMachine;
bool bHasInstance = false;

extern "C" scag::bill::BillingMachine* initBillingMachine(uint8_t machine_id, const std::string& cfg_dir)
{
    if (!bHasInstance) 
    {
        billingMachine = std::auto_ptr<scag::bill::CDRBillingMachine>(new scag::bill::CDRBillingMachine());
        bHasInstance = true;
    }
    
    return billingMachine.get();
}


namespace scag { namespace bill {

void CDRBillingMachine::rollback(const Bill &bill)
{
}


}}
