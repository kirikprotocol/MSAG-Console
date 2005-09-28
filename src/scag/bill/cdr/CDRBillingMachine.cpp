#include "CDRBillingMachine.h"
#include "scag/exc/SCAGExceptions.h"

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

using namespace scag::exceptions;

CDRBillingMachine::CDRBillingMachine() : m_Key(0) 
{ 
    actionFactory = new CDRActionFactory();
}

CDRBillingMachine * CDRBillingMachine::Instance()
{
    if (!bHasInstance) return 0;
    return billingMachine.get();
}


ActionFactory * CDRBillingMachine::getActionFactory() const 
{ 
    return actionFactory;
}


void CDRBillingMachine::commit(const Bill& bill)
{

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

    TransactionHash.Insert(m_Key,transaction);
    return bill;
}

Bill CDRBillingMachine::GetBill(BillKey& billKey)
{
    if (!BillMap.Exists(billKey)) throw SCAGException("Billing machine: cannot find bill '%d'",billKey.key);
    Bill bill;

    bill.bill_id = BillMap.Get(billKey);
    bill.machine_id = machine_id;
    return bill;
}


}}
