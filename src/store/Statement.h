#ifndef STATEMENT_DECLARATIONS
#define STATEMENT_DECLARATIONS

#include <oci.h>
#include <orl.h>

#include <sms/sms.h>
#include "StoreExceptions.h"

namespace smsc { namespace store 
{
    using namespace smsc::sms;
    
    class Connection;
    class Statement
    {
    protected:

        Connection  *owner;

        OCIEnv      *envhp;
        OCISvcCtx   *svchp;
        OCIError    *errhp;
        OCIStmt     *stmt;
        
        void convertDateToOCIDate(time_t* sms_date, OCIDate* oci_date);
        void convertOCIDateToDate(OCIDate* oci_date, time_t* sms_date);

    public:
        
        Statement(Connection* connection, const char* sql, 
                  bool assign=false) 
            throw(StorageException);
        virtual ~Statement();

        void check(sword status) 
            throw(StorageException);
    
        void bind(ub4 pos, ub2 type, 
                  dvoid* placeholder, sb4 size, dvoid* indp = 0) 
            throw(StorageException);
        void bind(CONST text* name, sb4 name_len, ub2 type,
                  dvoid* placeholder, sb4 size, dvoid* indp = 0)
            throw(StorageException);
        
        void define(ub4 pos, ub2 type, 
                    dvoid* placeholder, sb4 size, dvoid* indp = 0)
            throw(StorageException);

        sword execute(ub4 mode=OCI_DEFAULT,
                     ub4 iters=1, ub4 rowoff=0);
        sword fetch();

        ub4 getRowsAffectedCount();
    };
    
    using namespace smsc::sms;

    class IdStatement : public Statement
    {
    protected:

        SMSId   smsId;
        
        IdStatement(Connection* connection, const char* sql, 
                    bool assign=false) 
            throw(StorageException)
                : Statement(connection, sql) {};
    public:
        
        virtual ~IdStatement() {};

        void setSMSId(const SMSId smsId);
        void getSMSId(SMSId &smsId);
    };
    
    class GetIdStatement : public IdStatement
    {
    public:

        GetIdStatement(Connection* connection, const char* sql, 
                       bool assign=false)
            throw(StorageException);
        virtual ~GetIdStatement() {};
    };
    
    class SetIdStatement : public IdStatement
    {
    public:

        SetIdStatement(Connection* connection, const char* sql, 
                       bool assign=false)
            throw(StorageException);
        virtual ~SetIdStatement() {};
    };

    class StoreStatement : public IdStatement
    {
    static const char* sql;
    protected:
        
        OCIDate waitTime;
        OCIDate validTime;
        OCIDate submitTime;

        char    bHeaderIndicator;
        char    bNeedArchivate;

        sb2     indWaitTime, indSvcType;
        
    public:
        
        StoreStatement(Connection* connection, bool assign=true)
            throw(StorageException);
        virtual ~StoreStatement() {};

        void bindId(SMSId id)
            throw(StorageException);
        void bindSms(SMS& sms)
            throw(StorageException);
    };
    
    class NeedRejectStatement : public Statement
    {
    static const char* sql;
    protected:

        ub4 count;
    
    public:
        
        NeedRejectStatement(Connection* connection, bool assign=true)
            throw(StorageException);
        virtual ~NeedRejectStatement() {};

        void bindMr(dvoid* mr, sb4 size)
            throw(StorageException);
        void bindOriginatingAddress(Address& oa)
            throw(StorageException);
        void bindDestinationAddress(Address& da)
            throw(StorageException);
        
        bool needReject();
    };
    
    class NeedOverwriteStatement : public IdStatement
    {
    static const char* sql;
    protected:
        
        NeedOverwriteStatement(Connection* connection, const char* sql,
                               bool assign=true)
            throw(StorageException);

    public:
        
        NeedOverwriteStatement(Connection* connection, bool assign=true)
            throw(StorageException);
        virtual ~NeedOverwriteStatement() {};

        void bindOriginatingAddress(Address& oa)
            throw(StorageException);
        void bindDestinationAddress(Address& da)
            throw(StorageException);
        
        void getId(SMSId& id)
            throw(StorageException);
    };
    
    class NeedOverwriteSvcStatement : public NeedOverwriteStatement
    {
    static const char* sql;
    public:
        
        NeedOverwriteSvcStatement(Connection* connection, bool assign=true)
            throw(StorageException);
        virtual ~NeedOverwriteSvcStatement() {};

        void bindEServiceType(dvoid* type, sb4 size)
            throw(StorageException);
    };
    
    class OverwriteStatement : public IdStatement
    {
    static const char* sql;
    protected:
        
        OCIDate waitTime;
        OCIDate validTime;
        OCIDate submitTime;

        char    bHeaderIndicator;
        char    bNeedArchivate;

        sb2     indWaitTime, indSvcType;
    
    public:

        OverwriteStatement(Connection* connection, bool assign=true)
            throw(StorageException);
        virtual ~OverwriteStatement() {};
        
        void bindId(SMSId id)
            throw(StorageException);
        void bindSms(SMS& sms)
            throw(StorageException);
        
        inline bool wasOverwrited() {
            return (getRowsAffectedCount() ? true:false);
        };
    };

    class RetriveStatement : public IdStatement
    {
    static const char* sql;
    protected:

        OCIDate waitTime;
        OCIDate validTime;
        OCIDate submitTime;
        OCIDate lastTime;
        OCIDate nextTime;
        
        uint8_t uState;
        char    bHeaderIndicator;
        char    bNeedArchivate;

        sb2     indOA, indSrcMsc, indSrcImsi, indSrcSme;
        sb2     indDA, indDstMsc, indDstImsi, indDstSme;
        sb2     indSvc, indWaitTime, indLastTime, indNextTime;
    
    public:
        
        RetriveStatement(Connection* connection, bool assign=true)
            throw(StorageException);
        virtual ~RetriveStatement() {};

        void bindId(SMSId id)
            throw(StorageException);
        void defineSms(SMS& sms)
            throw(StorageException);
        void getSms(SMS& sms);
    };
    
    class DestroyStatement : public IdStatement
    {
    static const char* sql;
    public:
        
        DestroyStatement(Connection* connection, bool assign=true)
            throw(StorageException);
        virtual ~DestroyStatement() {};

        void bindId(SMSId id)
            throw(StorageException);
        
        inline bool wasDestroyed() {
            return (getRowsAffectedCount() ? true:false);
        };
    };
    
    class ReplaceStatement : public IdStatement
    {
    static const char* sql;
    protected:
        
        OCIDate wTime;
        OCIDate vTime;

        ReplaceStatement(Connection* connection, const char* sql,
                         bool assign=true) throw(StorageException);
    public:
        
        ReplaceStatement(Connection* connection, bool assign=true)
            throw(StorageException);
        virtual ~ReplaceStatement() {};
        
        void bindId(SMSId id)
            throw(StorageException);
        void bindOriginatingAddress(Address& oa)
            throw(StorageException);
        void bindBody(Body& body)
            throw(StorageException);
        void bindDeliveryReport(dvoid* dr, sb4 size)
            throw(StorageException);
        void bindValidTime(time_t validTime)
            throw(StorageException);
        void bindWaitTime(time_t waitTime)
            throw(StorageException);
        
        inline bool wasReplaced() {
            return (getRowsAffectedCount() ? true:false); 
        };
    };
    
    class ReplaceVTStatement : public ReplaceStatement
    {
    static const char* sql;
    public:
        
        ReplaceVTStatement(Connection* connection, bool assign=true)
            throw(StorageException);
        virtual ~ReplaceVTStatement() {};
    };
    
    class ReplaceWTStatement : public ReplaceStatement
    {
    static const char* sql;
    public:
        
        ReplaceWTStatement(Connection* connection, bool assign=true)
            throw(StorageException);
        virtual ~ReplaceWTStatement() {};
    };
    
    class ReplaceVWTStatement : public ReplaceStatement
    {
    static const char* sql;
    public:
        
        ReplaceVWTStatement(Connection* connection, bool assign=true)
            throw(StorageException);
        virtual ~ReplaceVWTStatement() {};
    };
    
    class ToEnrouteStatement : public IdStatement
    {
    static const char* sql;
    protected:
        
        OCIDate nextTime, currTime;
        
        sb2     indDstMsc, indDstImsi;

    public:

        ToEnrouteStatement(Connection* connection, bool assign=true)
            throw(StorageException);
        virtual ~ToEnrouteStatement() {};

        void bindId(SMSId id)
            throw(StorageException);
        void bindNextTime(time_t nextTryTime)
            throw(StorageException);
        void bindFailureCause(dvoid* cause, sb4 size)
            throw(StorageException);
        void bindDestinationDescriptor(Descriptor& dst)
            throw(StorageException);

        inline bool wasUpdated() {
            return (getRowsAffectedCount() ? true:false);
        };
    };
    
    class ToDeliveredStatement : public IdStatement
    {
    static const char* sql;
    protected:

        OCIDate currTime;
        sb2     indDstMsc, indDstImsi;

    public:

        ToDeliveredStatement(Connection* connection, bool assign=true)
            throw(StorageException);
        virtual ~ToDeliveredStatement() {};

        void bindId(SMSId id)
            throw(StorageException);
        void bindDestinationDescriptor(Descriptor& dst)
            throw(StorageException);

        inline bool wasUpdated() {
            return (getRowsAffectedCount() ? true:false);
        };
    };
    
    class ToUndeliverableStatement : public IdStatement
    {
    static const char* sql;
    protected:

        OCIDate currTime;
        sb2     indDstMsc, indDstImsi;

    public:

        ToUndeliverableStatement(Connection* connection, bool assign=true)
            throw(StorageException);
        virtual ~ToUndeliverableStatement() {};

        void bindId(SMSId id)
            throw(StorageException);
        void bindFailureCause(dvoid* cause, sb4 size)
            throw(StorageException);
        void bindDestinationDescriptor(Descriptor& dst)
            throw(StorageException);

        inline bool wasUpdated() {
            return (getRowsAffectedCount() ? true:false);
        };
    };
    
    class ToExpiredStatement : public IdStatement
    {
    static const char* sql;
    public:

        ToExpiredStatement(Connection* connection, bool assign=true)
            throw(StorageException);
        virtual ~ToExpiredStatement() {};

        void bindId(SMSId id)
            throw(StorageException);

        inline bool wasUpdated() {
            return (getRowsAffectedCount() ? true:false);
        };
    };
    
    class ToDeletedStatement : public IdStatement
    {
    static const char* sql;
    public:

        ToDeletedStatement(Connection* connection, bool assign=true)
            throw(StorageException);
        virtual ~ToDeletedStatement() {};

        void bindId(SMSId id)
            throw(StorageException);

        inline bool wasUpdated() {
            return (getRowsAffectedCount() ? true:false);
        };
    };
    
   
}}

#endif
