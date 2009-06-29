/* "$Id$" */
#ifndef _BILLING_MANAGER_BASE_H_
#define _BILLING_MANAGER_BASE_H_

#include "sms/sms.h"
#include "scag/re/base/LongCallContext.h"
#include "Infrastructure.h"
#include "BillingInfoStruct.h"
#include "scag/bill/ewallet/Client.h"
#include "scag/bill/ewallet/Status.h"
// #include "scag/util/Time.h"
#include "logger/Logger.h"

namespace scag2 {
namespace bill {

using namespace smsc::sms;
using namespace infrastruct;

// typedef util::msectime_type billid_type;

class BillOpenCallParamsData
{
public:
    BillingInfoStruct billingInfoStruct;
    TariffRec         tariffRec;
    // billid_type       BillId;            // moved to billingInfoStruct
};


class BillCloseTransitParamsData
{
public:
    uint32_t                              transId;
    std::auto_ptr<BillOpenCallParamsData> data;
};


class BillOpenCallParams
{
public:
    virtual ~BillOpenCallParams() {}
    virtual TariffRec* tariffRec() const = 0;
    virtual BillingInfoStruct* billingInfoStruct() const = 0;
    virtual billid_type billId() const = 0;
    virtual void setBillId( billid_type bi ) = 0;
};


class BillCloseCallParams
{
public:
    virtual ~BillCloseCallParams() {}
    virtual billid_type getBillId() const = 0;
    virtual BillCloseTransitParamsData* getTransitData() { return 0; }
};


class BillCallParams : public lcm::LongCallParams
{
public:
    BillCallParams();
    virtual BillOpenCallParams* getOpen() = 0;
    virtual BillCloseCallParams* getClose() = 0;
protected:
    static smsc::logger::Logger* log_;
};


class InmanOpenCallParams : public BillCallParams, public BillOpenCallParams
{
public:
    InmanOpenCallParams( BillOpenCallParamsData* data ) : data_(data) {}
    virtual TariffRec* tariffRec() const { return data_.get() ? &(data_->tariffRec) : 0; }
    virtual BillingInfoStruct* billingInfoStruct() const { return data_.get() ? &(data_->billingInfoStruct) : 0; }
    virtual billid_type billId() const { return data_.get() ? data_->billingInfoStruct.billId : 0; }
    virtual void setBillId( billid_type bi ) { if (data_.get()) data_->billingInfoStruct.billId = bi; }
    virtual InmanOpenCallParams* getOpen() { return this; }
    virtual BillCloseCallParams* getClose() { return 0; }
private:
    std::auto_ptr<BillOpenCallParamsData> data_;
};


class EwalletCallParams : public BillCallParams, public ewallet::Client::ResponseHandler
{
public:
    class TransactionRegistrator {
    public:
        virtual ~TransactionRegistrator() {}
        virtual void processAsyncResult( EwalletCallParams& params ) = 0;
    };

    EwalletCallParams( bool isTransit, lcm::LongCallContext* lcmCtx );
    virtual void handleResponse( std::auto_ptr< ewallet::Request > request, 
                                 std::auto_ptr< ewallet::Response > response );
    virtual void handleError( std::auto_ptr< ewallet::Request > request,
                              const ewallet::Exception& error );
    virtual void setStatus( uint8_t stat, const char* msg ) {
        status_ = stat;
        exception = msg;
    }
    void setRegistrator( TransactionRegistrator* reg ) { registrator_ = reg; }
    virtual void setResponse( ewallet::Response& resp ) = 0;
    virtual void continueExecution();

    /// identifies a transaction on Ewallet server
    uint32_t getTransId() const { return transId_; }
    void setTransId( uint32_t ti ) { transId_ = ti; }

    bool isTransit() const { return transit_; }

private:
    lcm::LongCallContext*   lcmCtx_;
    TransactionRegistrator* registrator_;
    bool                    transit_;
    uint32_t                transId_;
    uint8_t                 status_;
};


class EwalletOpenCallParams : public EwalletCallParams, public BillOpenCallParams
{
public:
    EwalletOpenCallParams( bool transit,
                           BillOpenCallParamsData* data,
                           lcm::LongCallContext* lcmCtx ) :
    EwalletCallParams(transit, lcmCtx), data_(data) {}
    virtual void setResponse( ewallet::Response& resp );
    virtual TariffRec* tariffRec() const { return data_.get() ? &(data_->tariffRec) : 0; }
    virtual BillingInfoStruct* billingInfoStruct() const { return data_.get() ? &(data_->billingInfoStruct) : 0; }
    virtual billid_type billId() const { return data_.get() ? data_->billingInfoStruct.billId : 0; }
    virtual void setBillId( billid_type bi ) { if (data_.get()) data_->billingInfoStruct.billId = bi; }
    virtual EwalletOpenCallParams* getOpen() { return this; }
    virtual BillCloseCallParams* getClose() { return 0; }
private:
    std::auto_ptr<BillOpenCallParamsData> data_;
};


class EwalletCloseCallParams : public EwalletCallParams, public BillCloseCallParams
{
public:
    // non-transit ctor
    EwalletCloseCallParams( billid_type billid, lcm::LongCallContext* lcmCtx ) :
    EwalletCallParams(false,lcmCtx), billId_(billid) {}

    // transit ctor
    EwalletCloseCallParams( BillCloseTransitParamsData* data, lcm::LongCallContext* lcmCtx ) :
    EwalletCallParams(true,lcmCtx), billId_(0), data_(data) {}

    virtual void setResponse( ewallet::Response& resp );
    virtual BillOpenCallParams* getOpen() { return 0; }
    virtual EwalletCloseCallParams* getClose() { return this; }
    virtual billid_type getBillId() const { return billId_; }
    virtual BillCloseTransitParamsData* getTransitData() { return data_.get(); }
private:
    billid_type billId_;
    std::auto_ptr<BillCloseTransitParamsData> data_;
};


class InmanCloseCallParams : public BillCallParams, public BillCloseCallParams
{
public:
    InmanCloseCallParams( billid_type billid ) : billId_(billid) {}
    virtual BillOpenCallParams* getOpen() { return 0; }
    virtual InmanCloseCallParams* getClose() { return this; }
    virtual billid_type getBillId() const { return billId_; }
private:
    billid_type billId_;
};


class BillingManager
{
public:
    virtual billid_type Open( BillOpenCallParams& openCallParams,
                              lcm::LongCallContext* lcmCtx = NULL) = 0;
    virtual void Commit( billid_type billId,
                         lcm::LongCallContext* lcmCtx = NULL ) = 0;
    virtual void Rollback( billid_type billId,
                           lcm::LongCallContext* lcmCtx = NULL) = 0;
    virtual void CommitTransit( BillCloseCallParams& closeCallParams,
                                lcm::LongCallContext* lcmCtx = NULL ) = 0;
    virtual void RollbackTransit( BillCloseCallParams& closeCallParams,
                                  lcm::LongCallContext* lcmCtx = NULL ) = 0;
    virtual void Info(billid_type billId, BillingInfoStruct& bis,
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
