#ifndef STATEMENT_DECLARATIONS
#define STATEMENT_DECLARATIONS

#include <oci.h>
#include <orl.h>

#include <sms/sms.h>
#include "StoreExceptions.h"

namespace smsc { namespace store 
{
    class Connection;
    class Statement
    {
    protected:

        Connection  *owner;

        OCIEnv      *envhp;
        OCISvcCtx   *svchp;
        OCIError    *errhp;
        OCIStmt     *stmt;
        
        Statement(Connection* connection, const char* sql) 
            throw(StorageException);
        
        void checkErr(sword status) 
            throw(StorageException);
        
        void convertDateToOCIDate(time_t* sms_date, OCIDate* oci_date);
        void convertOCIDateToDate(OCIDate* oci_date, time_t* sms_date);

    public:
        
        virtual ~Statement();

        void bind(ub4 pos, ub2 type, 
                  dvoid* placeholder, sb4 size) 
            throw(StorageException);
        void bind(CONST text* name, sb4 name_len, ub2 type,
                  dvoid* placeholder, sb4 size)
            throw(StorageException);
        
        void define(ub4 pos, ub2 type, 
                    dvoid* placeholder, sb4 size)
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
        
        IdStatement(Connection* connection, const char* sql) 
            throw(StorageException)
                : Statement(connection, sql) {};
    public:
        
        virtual ~IdStatement() {};

        void setSMSId(const SMSId smsId);
        void getSMSId(SMSId &smsId);
    };

    class MessageStatement : public IdStatement
    {
    protected:

        SMS     sms;
        
        OCIDate waitTime;
        OCIDate validTime;
        OCIDate submitTime;
        OCIDate deliveryTime;
        
        uint8_t         uState;
        char            bStatusReport;
        char            bRejectDuplicates;
        char            bHeaderIndicator;
        char            bNeedArchivate;
        
        MessageStatement(Connection* connection, const char* sql) 
            throw(StorageException)
                : IdStatement(connection, sql) {};
    public:
        
        virtual ~MessageStatement() {};

        void setSMS(const SMS &sms);
        void getSMS(SMS &sms);
    };

    class StoreStatement : public MessageStatement
    {
    static const char* sql;
    public:
        
        StoreStatement(Connection* connection)
            throw(StorageException);
        virtual ~StoreStatement() {};
    };
    
    class IsRejectedStatement : public MessageStatement
    {
    static const char* sql;
    protected:

        ub4 count;
    
    public:
        
        IsRejectedStatement(Connection* connection)
            throw(StorageException);
        virtual ~IsRejectedStatement() {};

        bool isRejected();
    };
    
    class RetriveStatement : public MessageStatement
    {
    static const char* sql;
    public:
        
        RetriveStatement(Connection* connection)
            throw(StorageException);
        virtual ~RetriveStatement() {};
    };
    
    class ReplaceStatement : public MessageStatement
    {
    static const char* sql;
    public:
        
        ReplaceStatement(Connection* connection)
            throw(StorageException);
        virtual ~ReplaceStatement() {};

        inline bool wasReplaced() {
            return (getRowsAffectedCount() ? true:false); 
        };
    };
    
    class RemoveStatement : public IdStatement
    {
    static const char* sql;
    public:
        
        RemoveStatement(Connection* connection)
            throw(StorageException);
        virtual ~RemoveStatement() {};

        inline bool wasRemoved() {
            return (getRowsAffectedCount() ? true:false);
        };
    };
    
    class GetMaxIdStatement : public IdStatement
    {
    static const char* sql;
    public:
        
        GetMaxIdStatement(Connection* connection)
            throw(StorageException);
        virtual ~GetMaxIdStatement() {};
    };

    class StateUpdateStatement : public IdStatement
    {
    static const char* sql;
    protected:
        
        uint8_t uState;

        StateUpdateStatement(Connection* connection, const char* sql)
            throw(StorageException) 
                : IdStatement(connection, sql) {};
    
    public:

        StateUpdateStatement(Connection* connection)
            throw(StorageException);
        virtual ~StateUpdateStatement() {};

        inline bool wasUpdated() {
            return (getRowsAffectedCount() ? true:false);
        };
        inline void setState(State state) {
            uState = (uint8_t)state;
        };
    };
    
    class StateDateUpdateStatement : public StateUpdateStatement
    {
    static const char* sql;
    protected:

        OCIDate     operationDate;
    
        StateDateUpdateStatement(Connection* connection, const char* sql)
            throw(StorageException) 
                : StateUpdateStatement(connection, sql) {};
    public:

        StateDateUpdateStatement(Connection* connection)
            throw(StorageException);
        virtual ~StateDateUpdateStatement() {};

        inline void setOpTime(time_t time) {
            convertDateToOCIDate(&(time), &operationDate);
        };
    };
    
    class StateDateFcsUpdateStatement : public StateDateUpdateStatement
    {
    static const char* sql;
    protected:

        uint8_t     fcs;

    public:

        StateDateFcsUpdateStatement(Connection* connection)
            throw(StorageException);
        virtual ~StateDateFcsUpdateStatement() {};

        inline void setFcs(uint8_t fcs) {
            this->fcs = fcs;
        };
    };

}}

#endif
