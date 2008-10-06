/* "$Id$" */
#ifndef _BILLING_MANAGER_H_
#define _BILLING_MANAGER_H_

#include "sms/sms.h"
#include <core/synchronization/EventMonitor.hpp>
#include "scag/config/bill/BillingManagerConfig.h"
#include "scag/lcm/LongCallManager.h"

#include "infrastruct/Infrastructure.h"

namespace scag { namespace bill {

using namespace smsc::sms;
using namespace smsc::core::synchronization;
using namespace scag::config;
using namespace scag::bill::infrastruct;
using namespace scag::lcm;

enum TransactionStatus
{
    TRANSACTION_NOT_STARTED = 0,
    TRANSACTION_WAIT_ANSWER = 1,
    TRANSACTION_VALID       = 2,
    TRANSACTION_INVALID     = 3
};

enum BillingTransactionEvent
{
    TRANSACTION_OPEN            = 1,
    TRANSACTION_COMMITED        = 2,
    TRANSACTION_CALL_ROLLBACK   = 3,
    TRANSACTION_TIME_OUT        = 4
    //TRANSACTION_REFUSED         = 5
};

enum BillingCommandStatus
{
    COMMAND_SUCCESSFULL         =0,
    SERVER_NOT_RESPONSE         =1,
    REJECTED_BY_SERVER          =2,
    EXTERNAL_ERROR              =3,
    INVALID_TRANSACTION         =4
};
/*
1-open
2-commit
3-rollback
4-rollback_by_timeout */


struct BillingInfoStruct
{
    std::string mediaType, category;
    std::string AbonentNumber;
    int serviceId;
    int protocol;
    int providerId;
    int operatorId;
    int msgRef;

    timeval SessionBornMicrotime;

    BillingInfoStruct():keywords_(0) {};

    const BillingInfoStruct& operator=(const BillingInfoStruct& cp)
    {
        if (this == &cp) {
          return *this;
        }
        mediaType = cp.mediaType;
        category = cp.category;
        AbonentNumber = cp.AbonentNumber;
        serviceId = cp.serviceId;
        protocol = cp.protocol;
        providerId = cp.providerId;
        operatorId = cp.operatorId;
        msgRef = cp.msgRef;
        SessionBornMicrotime = cp.SessionBornMicrotime;
        const std::string* keywords = cp.getKeywords();
        if (keywords) {
          setKeywords(*keywords);
        }
        return *this;
    }

    const std::string* getKeywords() const {
        return keywords_;
    }

    void setKeywords( const std::string& kw ) {
        if ( keywords_ ) delete keywords_;
        if ( kw.empty() ) keywords_ = 0; 
        else keywords_ = new std::string( kw );
    }

private:
    std::string* keywords_;
};

class BillOpenCallParams : public LongCallParams
{
public:
    BillingInfoStruct billingInfoStruct;
    TariffRec tariffRec;
    int BillId;
};

class BillCloseCallParams : public LongCallParams
{
public:
    int BillId;
};

class BillingManager
{
    BillingManager(const BillingManager& bm);
    BillingManager& operator=(const BillingManager& bm);
protected:
    virtual ~BillingManager() {};
    BillingManager() {};

public:
    virtual unsigned int Open(BillingInfoStruct& billingInfoStruct, TariffRec& tariffRec, LongCallContext* lcmCtx = NULL) = 0;
    virtual void Commit(int billId, LongCallContext* lcmCtx = NULL) = 0;
    virtual void Rollback(int billId, bool timeout = false, LongCallContext* lcmCtx = NULL) = 0;
    virtual void Info(int billId, BillingInfoStruct& bis, TariffRec& tariffRec) = 0;
    virtual void Stop() = 0;

    static BillingManager& Instance();
    static void Init(BillingManagerConfig& cfg);

    virtual Infrastructure& getInfrastructure() = 0;
};


}}

#endif
