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


// --- params data
class BillOpenCallParamsData
{
public:
    BillingInfoStruct billingInfoStruct;
    TariffRec         tariffRec;
};

class BillTransitParamsData
{
public:
    BillTransitParamsData() : transId(0) {}
    uint32_t                              transId;
    std::auto_ptr<BillOpenCallParamsData> data;
};


// --- interfaces for different types of calls


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
    virtual billid_type billId() const = 0;
    virtual bool isTimeout() const = 0;
    virtual BillTransitParamsData* getTransitData() { return 0; }
};


class BillCheckCallParams
{
public:
    virtual ~BillCheckCallParams() {}
    virtual BillTransitParamsData* getTransitData() { return 0; }
};


class BillTransferCallParams
{
public:
    virtual ~BillTransferCallParams() {}

    virtual int getAgentId() const = 0;
    virtual const std::string& getUserId() const = 0;
    virtual const std::string& getSrcWalletType() const = 0;
    virtual const std::string& getDstWalletType() const = 0;
    virtual const std::string& getExternalId() const = 0;
    virtual int getAmount() const = 0;
    virtual const std::string& getDescription() const = 0;
};

// --- actual hierarchy of call params (inherited from lcm::LongCallParams)


class BillCallParams : public lcm::LongCallParams
{
public:
    BillCallParams();
    virtual BillOpenCallParams* getOpen() = 0;
    virtual BillCloseCallParams* getClose() = 0;
    virtual BillCheckCallParams* getCheck() = 0;
    virtual BillTransferCallParams* getTransfer() { return 0; }
    virtual int getStatus() const = 0;
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
    virtual BillCheckCallParams* getCheck() { return 0; }
    virtual int getStatus() const {
        return exception.empty() ? ewallet::Status::OK : ewallet::Status::UNKNOWN;
    }
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
    virtual int getStatus() const { return int(status_); }

    void setRegistrator( TransactionRegistrator* reg ) { registrator_ = reg; }
    virtual void setResponse( ewallet::Response& resp ) = 0;
    virtual void continueExecution();

    /// identifies a transaction on Ewallet server
    uint32_t getTransId() const { return transId_; }
    void setTransId( uint32_t ti ) { transId_ = ti; }

    bool isTransit() const { return transit_; }

private:
    EwalletCallParams();

protected:
    lcm::LongCallContext*   lcmCtx_;
private:
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
    EwalletCallParams(transit, lcmCtx), data_(data), resultAmount_(0), chargeThreshold_(0) {}
    virtual void setResponse( ewallet::Response& resp );
    virtual TariffRec* tariffRec() const { return data_.get() ? &(data_->tariffRec) : 0; }
    virtual BillingInfoStruct* billingInfoStruct() const { return data_.get() ? &(data_->billingInfoStruct) : 0; }
    virtual billid_type billId() const { return data_.get() ? data_->billingInfoStruct.billId : 0; }
    virtual void setBillId( billid_type bi ) { if (data_.get()) data_->billingInfoStruct.billId = bi; }
    virtual EwalletOpenCallParams* getOpen() { return this; }
    virtual BillCloseCallParams* getClose() { return 0; }
    virtual BillCheckCallParams* getCheck() { return 0; }

    int32_t getResultAmount() const { return resultAmount_; }
    void setResultAmount( int32_t v ) { resultAmount_ = v; }
    int32_t getChargeThreshold() const { return chargeThreshold_; }
    void setChargeThreshold( int32_t v ) { chargeThreshold_ = v; }

private:
    std::auto_ptr<BillOpenCallParamsData> data_;
    int32_t                 resultAmount_;
    int32_t                 chargeThreshold_;
};


class EwalletCloseCallParams : public EwalletCallParams, public BillCloseCallParams
{
public:
    // non-transit ctor
    EwalletCloseCallParams( billid_type billid, bool timeout, lcm::LongCallContext* lcmCtx ) :
    EwalletCallParams(false,lcmCtx), billId_(billid), timeout_(timeout) {}

    // transit ctor
    EwalletCloseCallParams( BillTransitParamsData* data, lcm::LongCallContext* lcmCtx ) :
    EwalletCallParams(true,lcmCtx), billId_(0), timeout_(false), data_(data) {}

    void setTransitData( BillTransitParamsData* data ) {
        data_.reset(data);
    }

    virtual void setResponse( ewallet::Response& resp );
    virtual BillOpenCallParams* getOpen() { return 0; }
    virtual EwalletCloseCallParams* getClose() { return this; }
    virtual BillCheckCallParams* getCheck() { return 0; }
    virtual billid_type billId() const { return billId_; }
    virtual BillTransitParamsData* getTransitData() { return data_.get(); }
    bool isCommit() const;
    bool isTimeout() const { return timeout_; }
private:
    billid_type billId_;
    bool timeout_;
    std::auto_ptr<BillTransitParamsData> data_;
};


class EwalletCheckCallParams : public EwalletCallParams, public BillCheckCallParams
{
public:
    // transit ctor
    EwalletCheckCallParams( BillTransitParamsData* data, lcm::LongCallContext* lcmCtx ) :
    EwalletCallParams(true,lcmCtx), data_(data),
    transStatus_(0), txAmount_(0), txEndDate_(0) {}

    virtual void setResponse( ewallet::Response& resp );
    virtual BillOpenCallParams* getOpen() { return 0; }
    virtual BillCloseCallParams* getClose() { return 0; }
    virtual EwalletCheckCallParams* getCheck() { return this; }
    virtual BillTransitParamsData* getTransitData() { return data_.get(); }
    int getTransStatus() const { return transStatus_; }
    int getTxAmount() const { return txAmount_; }
    int64_t getTxEndDate() const { return txEndDate_; }
private:
    std::auto_ptr<BillTransitParamsData> data_;
    // returned values
    int  transStatus_;
    int  txAmount_;
    int64_t txEndDate_;
};


class EwalletTransferCallParams : public EwalletCallParams, public BillTransferCallParams
{
public:
    EwalletTransferCallParams( lcm::LongCallContext* ctx ) :
    EwalletCallParams(true,ctx) {}

    virtual void setResponse( ewallet::Response& resp );
    virtual BillOpenCallParams* getOpen() { return 0; }
    virtual BillCloseCallParams* getClose() { return 0; }
    virtual BillCheckCallParams* getCheck() { return 0; }
    virtual EwalletTransferCallParams* getTransfer() { return this; }

    void setAgentId( int v ) { agentId_ = v; }
    void setUserId( const std::string& v ) { userId_ = v; }
    void setSrcWalletType( const std::string& v ) { srcWalletType_ = v; }
    void setDstWalletType( const std::string& v ) { dstWalletType_ = v; }
    void setExternalId( const std::string& v ) { externalId_ = v; }
    void setAmount( int v ) { amount_ = v; }
    void setDescription( const std::string& v ) { description_ = v; }

    virtual int getAgentId() const { return agentId_; }
    virtual const std::string& getUserId() const { return userId_; }
    virtual const std::string& getSrcWalletType() const { return srcWalletType_; }
    virtual const std::string& getDstWalletType() const { return dstWalletType_; }
    virtual const std::string& getExternalId() const { return externalId_; }
    virtual int getAmount() const { return amount_; }
    virtual const std::string& getDescription() const { return description_; }

private:
    int         agentId_;
    std::string userId_;
    std::string srcWalletType_;
    std::string dstWalletType_;
    std::string externalId_;
    int         amount_;
    std::string description_;
};


class EwalletInfoCallParams : public EwalletCallParams
{
public:
    EwalletInfoCallParams( BillOpenCallParamsData* data,
                           lcm::LongCallContext*   lcmCtx ) :
    EwalletCallParams(true,lcmCtx),
    data_(data) {}

    virtual BillOpenCallParams* getOpen() { return 0; }
    virtual BillCloseCallParams* getClose() { return 0; }
    virtual BillCheckCallParams* getCheck() { return 0; }
    virtual void setResponse( ewallet::Response& resp );

    virtual BillOpenCallParamsData* getInfoData() { return data_.get(); }

    int32_t getResultAmount() const { return resultAmount_; }
    void setResultAmount( int32_t v ) { resultAmount_ = v; }
    int32_t getChargeThreshold() const { return chargeThreshold_; }
    void setChargeThreshold( int32_t v ) { chargeThreshold_ = v; }

private:
    std::auto_ptr< BillOpenCallParamsData > data_;
    int32_t                 resultAmount_;
    int32_t                 chargeThreshold_;
};


class InmanCloseCallParams : public BillCallParams, public BillCloseCallParams
{
public:
    InmanCloseCallParams( billid_type billid, bool timeout ) : billId_(billid), timeout_(timeout) {}
    virtual BillOpenCallParams* getOpen() { return 0; }
    virtual InmanCloseCallParams* getClose() { return this; }
    virtual BillCheckCallParams* getCheck() { return 0; }
    virtual billid_type billId() const { return billId_; }
    virtual bool isTimeout() const { return timeout_; }
    virtual int getStatus() const {
        return exception.empty() ? ewallet::Status::OK : ewallet::Status::UNKNOWN;
    }
private:
    billid_type billId_;
    bool        timeout_;
};



class BillingManager
{
public:
    virtual billid_type Open( BillOpenCallParams& openCallParams,
                              lcm::LongCallContext* lcmCtx = NULL) = 0;
    virtual void Commit( billid_type billId,
                         lcm::LongCallContext* lcmCtx = NULL ) = 0;
    virtual void Rollback( billid_type billId,
                           bool timeout,
                           lcm::LongCallContext* lcmCtx = NULL) = 0;
    virtual void CommitTransit( BillCloseCallParams& closeCallParams,
                                lcm::LongCallContext* lcmCtx = NULL ) = 0;
    virtual void RollbackTransit( BillCloseCallParams& closeCallParams,
                                  lcm::LongCallContext* lcmCtx = NULL ) = 0;
    virtual void Check( BillCheckCallParams& checkCallParams,
                        lcm::LongCallContext* lcmCtx ) = 0;
    virtual void Info(billid_type billId, BillingInfoStruct& bis,
                      TariffRec& tariffRec) = 0;
    virtual void Info( EwalletInfoCallParams& infoParams,
                       lcm::LongCallContext* lcmCtx ) = 0;
    virtual void Transfer( BillTransferCallParams& transferParams,
                           lcm::LongCallContext* lcmCtx ) = 0;
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
