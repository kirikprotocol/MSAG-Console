#ifndef _CDR_BILLING_MACHINE_
#define _CDR_BILLING_MACHINE_

#include "scag/bill/BillingMachine.h"
#include "CDRActionFactory.h"
#include "core/buffers/IntHash.hpp"
#include "scag/util/sms/HashUtil.h"
#include <core/buffers/XHash.hpp>


extern "C" scag::bill::BillingMachine* initBillingMachine(uint8_t machine_id, const std::string& cfg_dir);
//extern "C" initBillingMachineFn initBillingMachine;

namespace scag { namespace bill {

using namespace smsc::core::buffers;
using namespace scag::util::sms;

class CDRBillingMachine : public BillingMachine
{

    class XBillKeyHashFunc {
    public:
        static uint32_t CalcHash(const BillKey& key)
        {
            Address addr(key.OA.c_str());
            
            return XAddrHashFunc::CalcHash(addr);
        }
    };


    IntHash<Transaction> TransactionHash;
    XHash<BillKey,int,XBillKeyHashFunc> BillMap;

    CDRActionFactory actionFactory;
    unsigned int m_Key;
public:
    virtual void rollback(const Bill& bill);
    void commit(const Bill& bill);

    virtual const ActionFactory& getActionFactory() const { return actionFactory;}
    static CDRBillingMachine * Instance();
    Bill OpenTransaction();
    CDRBillingMachine() : m_Key(0) {}
    Bill GetBill(BillKey& billKey);
};


}}


#endif
