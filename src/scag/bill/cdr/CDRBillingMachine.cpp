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

CDRBillingMachine * CDRBillingMachine::Instance()
{
    if (!bHasInstance) return 0;
    return billingMachine.get();
}


void CDRBillingMachine::rollback(const Bill &bill)
{
}

Bill CDRBillingMachine::OpenTransaction()
{
    Bill bill;
    Transaction transaction;

    bill.machine_id = machine_id;
    bill.bill_id = ++m_Key;

    TransactionHash.Insert(key,transaction);
    return bill;
}


}}
