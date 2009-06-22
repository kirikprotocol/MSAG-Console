/* "$Id$" */
#ifndef _BILLING_MANAGER_BASE_H_
#define _BILLING_MANAGER_BASE_H_

#include "sms/sms.h"
#include "scag/re/base/LongCallContext.h"
#include "Infrastructure.h"
#include "BillingInfoStruct.h"
#include "scag/bill/ewallet/Client.h"
#include "scag/bill/ewallet/Status.h"

namespace scag2 {
namespace bill {

using namespace smsc::sms;
using namespace infrastruct;

class BillOpenCallParamsData
{
public:
    BillingInfoStruct billingInfoStruct;
    TariffRec         tariffRec;
    int               BillId;            // returned
};


class BillOpenCallParams
{
public:
    virtual ~BillOpenCallParams() {}
    virtual TariffRec* tariffRec() const = 0;
    virtual BillingInfoStruct* billingInfoStruct() const = 0;
    virtual int billId() const = 0;
    virtual void setBillId( int bi ) = 0;
};


class BillCloseCallParams
{
public:
    virtual ~BillCloseCallParams() {}
    virtual int getBillId() const = 0;
private:
    int billId;
};


class BillCallParams : public lcm::LongCallParams
{
public:
    virtual BillOpenCallParams* getOpen() = 0;
    virtual BillCloseCallParams* getClose() = 0;
};


class InmanOpenCallParams : public BillCallParams, public BillOpenCallParams
{
public:
    InmanOpenCallParams( BillOpenCallParamsData* data ) : data_(data) {}
    virtual TariffRec* tariffRec() const { return data_.get() ? &(data_->tariffRec) : 0; }
    virtual BillingInfoStruct* billingInfoStruct() const { return data_.get() ? &(data_->billingInfoStruct) : 0; }
    virtual int billId() const { return data_.get() ? data_->BillId : 0; }
    virtual void setBillId( int bi ) { if (data_.get()) data_->BillId = bi; }
    virtual BillOpenCallParams* getOpen() { return this; }
    virtual BillCloseCallParams* getClose() { return 0; }
private:
    std::auto_ptr<BillOpenCallParamsData> data_;
};


class EwalletCallParams : public BillCallParams, public ewallet::Client::ResponseHandler
{
public:
    EwalletCallParams( lcm::LongCallContext* lcmCtx );
    virtual void handleResponse( std::auto_ptr< ewallet::Request > request, 
                                 std::auto_ptr< ewallet::Response > response );
    virtual void handleError( std::auto_ptr< ewallet::Request > request,
                              const ewallet::Exception& error );
    virtual void setStatus( uint8_t stat, const char* msg ) {
        status_ = stat;
        exception = msg;
    }
    virtual void setResponse( ewallet::Response& resp ) = 0;
    virtual void continueExecution();
private:
    lcm::LongCallContext* lcmCtx_;
    uint8_t               status_;
};


class EwalletOpenCallParams : public EwalletCallParams, public BillOpenCallParams
{
public:
    EwalletOpenCallParams( BillOpenCallParamsData* data, lcm::LongCallContext* lcmCtx ) :
    EwalletCallParams(lcmCtx), data_(data) {}
    virtual void setResponse( ewallet::Response& resp ) {
        // fixme;
    }
    virtual TariffRec* tariffRec() const { return data_.get() ? &(data_->tariffRec) : 0; }
    virtual BillingInfoStruct* billingInfoStruct() const { return data_.get() ? &(data_->billingInfoStruct) : 0; }
    virtual int billId() const { return data_.get() ? data_->BillId : 0; }
    virtual void setBillId( int bi ) { if (data_.get()) data_->BillId = bi; }
    virtual BillOpenCallParams* getOpen() { return this; }
    virtual BillCloseCallParams* getClose() { return 0; }
private:
    std::auto_ptr<BillOpenCallParamsData> data_;
};


class InmanCloseCallParams : public BillCallParams, public BillCloseCallParams
{
public:
    InmanCloseCallParams( int billid ) : billId_(billid) {}
    virtual BillOpenCallParams* getOpen() { return 0; }
    virtual BillCloseCallParams* getClose() { return this; }
    virtual int getBillId() const { return billId_; }
private:
    int billId_;
};


class BillingManager
{
public:
    virtual unsigned int Open( BillOpenCallParams& openCallParams,
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
