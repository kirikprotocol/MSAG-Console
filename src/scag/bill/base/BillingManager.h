/* "$Id$" */
#ifndef _BILLING_MANAGER_BASE_H_
#define _BILLING_MANAGER_BASE_H_

#include "sms/sms.h"
#include "scag/re/base/LongCallContext.h"
#include "Infrastructure.h"
#include "BillingInfoStruct.h"

namespace scag2 {
namespace bill {

using namespace smsc::sms;
using namespace infrastruct;

class BillOpenCallParams : public lcm::LongCallParams
{
public:
    BillingInfoStruct billingInfoStruct;
    TariffRec tariffRec;
    int BillId;
};

class BillCloseCallParams : public lcm::LongCallParams
{
public:
    int BillId;
};

class BillingManager
{
public:
    virtual unsigned int Open( BillingInfoStruct& billingInfoStruct,
                               TariffRec& tariffRec,
                               lcm::LongCallContext* lcmCtx = NULL) = 0;
    virtual void Commit( int billId, lcm::LongCallContext* lcmCtx = NULL ) = 0;
    virtual void Rollback( int billId, bool timeout = false,
                           lcm::LongCallContext* lcmCtx = NULL) = 0;
    virtual void Info(int billId, BillingInfoStruct& bis,
                      TariffRec& tariffRec) = 0;
    virtual void Stop() = 0;

    static BillingManager& Instance();
    // static void Init(BillingManagerConfig& cfg);

    virtual Infrastructure& getInfrastructure() = 0;

    virtual ~BillingManager();

protected:
    BillingManager();

private:
    BillingManager(const BillingManager& bm);
    BillingManager& operator=(const BillingManager& bm);
};


}}

#endif
