#ifndef STRORE_EXCEPTIONS_DECLARATIONS
#define STRORE_EXCEPTIONS_DECLARATIONS

#include <util/Exception.hpp>
#include <sms/sms.h>

namespace smsc { namespace store
{
    using smsc::sms::SMSId;
    using smsc::util::Exception;
    
    class StoreException : public Exception
    {
    public:
        
        StoreException() 
            : Exception() {};
        StoreException(const char* fmt,...)
            : Exception() 
        {
            SMSC_UTIL_EX_FILL(fmt);
        };
        virtual ~StoreException() throw() {};
    };
    
    class StorageException : public StoreException
    {
    protected:
        
        int   code;
        int   status;
        
    public:
        
        StorageException(const StorageException& exc) 
            : StoreException(exc.what()),
                code(exc.getErrorCode()), status(exc.getErrorStatus())
        {};
        
        StorageException(const char* _cause, int _code=-1, int _status=0) 
            : StoreException("Code: %d, Status: %d, Failure cause: %s",
                             _code, _status, _cause), 
                    code(_code), status(_status) 
        {};
        virtual ~StorageException() throw() {};
        
        inline int getErrorCode() const {
            return code;
        };
        inline int getErrorStatus() const {
            return status;
        };
    };
    
    class ConnectionFailedException : public StorageException
    {
    public:
        
        ConnectionFailedException(StorageException& exc)
            : StorageException(exc) {};
        virtual ~ConnectionFailedException() throw() {};
    };
    
    class NoSuchMessageException : public StoreException
    {
    public:
        
        NoSuchMessageException() 
            : StoreException("Unable to find such message in DB !") 
        {};
        NoSuchMessageException(SMSId id) 
            : StoreException("Unable to find message with id = %llu in DB !", id)
        {};
        virtual ~NoSuchMessageException() throw() {};
    };
    
    class DuplicateMessageException : public StoreException
    {
    public:

        DuplicateMessageException() 
            : StoreException("Message was rejected by MessageStore ! "
                             "Storage already contains such message") {};
        DuplicateMessageException(SMSId id) 
            : StoreException("Message #%llu was rejected by MessageStore ! "
                             "Storage already contains such message", id) {};
        
        virtual ~DuplicateMessageException() throw() {};
    };

}}

#endif

