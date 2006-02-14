#ifndef _BILLING_MANAGER_H_
#define _BILLING_MANAGER_H_

#include "sms/sms.h"
#include <core/synchronization/EventMonitor.hpp>


namespace scag { namespace bill {

using namespace smsc::sms;
using namespace smsc::core::synchronization;

struct CTransportId
{
    Address OA;
    Address DA;
};

class BillingManager
{
    BillingManager(const BillingManager& bm);
    BillingManager& operator=(const BillingManager& bm);
protected:
    virtual ~BillingManager() {};
    BillingManager() {};

public:
    virtual int ChargeBill(CTransportId& transportId) = 0;
    virtual bool CheckBill(int billId, EventMonitor& eventMonitor) = 0;
    virtual void CommitBill(int billId) = 0;

    static BillingManager& Instance();
    static void Init();




};


}}

#endif
