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
        
        void convertDateToOCIDate(time_t* sms_date, OCIDate* oci_date);
        void convertOCIDateToDate(OCIDate* oci_date, time_t* sms_date);
        
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
    private:
        
        static const char* sql;
    
    public:
        
        StoreStatement(Connection* connection)
            throw(StorageException);
        virtual ~StoreStatement() {};
    };
    
    class IsRejectedStatement : public MessageStatement
    {
    private:
        
        static const char* sql;
        
        ub4 count;
    
    public:
        
        IsRejectedStatement(Connection* connection)
            throw(StorageException);
        virtual ~IsRejectedStatement() {};

        bool isRejected();
    };
    
    class RetriveStatement : public MessageStatement
    {
    private:
        
        static const char* sql;
    
    public:
        
        RetriveStatement(Connection* connection)
            throw(StorageException);
        virtual ~RetriveStatement() {};
    };
    
    class ReplaceStatement : public MessageStatement
    {
    private:
        
        static const char* sql;
    
    public:
        
        ReplaceStatement(Connection* connection)
            throw(StorageException);
        virtual ~ReplaceStatement() {};

        bool wasReplaced();
    };
    
    class RemoveStatement : public IdStatement
    {
    private:
        
        static const char* sql;
    
    public:
        
        RemoveStatement(Connection* connection)
            throw(StorageException);
        virtual ~RemoveStatement() {};

        bool wasRemoved();
    };
    
    class GetMaxIdStatement : public IdStatement
    {
    private:
        
        static const char* sql;
    
    public:
        
        GetMaxIdStatement(Connection* connection)
            throw(StorageException);
        virtual ~GetMaxIdStatement() {};
    };

}}

#endif
