#ifndef _CDR_BILLING_MACHINE_
#define _CDR_BILLING_MACHINE_

#include "scag/bill/BillingMachine.h"
#include "CDRActionFactory.h"

extern "C" scag::bill::BillingMachine* initBillingMachine(uint8_t machine_id, const std::string& cfg_dir);
//extern "C" initBillingMachineFn initBillingMachine;

namespace scag { namespace bill {


class CDRBillingMachine : public BillingMachine
{
    IntHash<Transaction> TransactionHash;

    CDRActionFactory actionFactory;
    unsigned int m_key;
public:
    virtual void rollback(const Bill &bill);
    virtual const ActionFactory& getActionFactory() const { return actionFactory;}
    static CDRBillingMachine * Instance();
    Bill OpenTransaction();
    CDRBillingMachine() : m_key(0) {}
};


}}


#endif
