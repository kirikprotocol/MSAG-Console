#ifndef STRORE_EXCEPTIONS_DECLARATIONS
#define STRORE_EXCEPTIONS_DECLARATIONS

#include <stdio.h>
#include <string.h>
#include <exception>
#include <sms/sms.h>

using std::exception;

namespace smsc { namespace store
{
    const int SMSC_STORE_QUEUE_TOO_LARGE = -10001;

    using smsc::sms::SMSId;

    const int SMSC_MAX_ERROR_MESSAGE_LENGTH = 2048;

    class StoreException : public exception
    {
    protected:
        
        char  cause[SMSC_MAX_ERROR_MESSAGE_LENGTH];
        
        StoreException() {};

    public:
        
        StoreException(const char* _cause) {
            strcpy(cause, _cause);
        };
        
        virtual ~StoreException() throw() {};
       
        virtual const char* what() const throw() {
            return cause;
        };
        
    };
    
    class StorageException : public StoreException
    {
    protected:
        
        int   code;
        int   status;
        
        StorageException(int _code=-1, int _status=0) 
            : StoreException(), code(_code) {};

    public:
        
        StorageException(const StorageException& exc) 
            : StoreException(exc.what()),
                code(exc.getErrorCode()), status(exc.getErrorStatus())
        {};
        
        StorageException(const char* _cause, int _code=-1, int _status=0) 
            : StoreException(), code(_code), status(_status) 
        {
            sprintf(cause, "Code - %d, Status - %d, Failure cause - %s", 
                    _code, _status, _cause); 
        };
        virtual ~StorageException() throw() {};
        
        inline int getErrorCode() const {
            return code;
        };
        inline int getErrorStatus() const {
            return status;
        };
    };
    
    /*class TooLargeQueueException : public StorageException
    {
    public:
        
        TooLargeQueueException() 
            : StorageException("Too many pending requests are waiting "
                               "for store connections !",
                               SMSC_STORE_QUEUE_TOO_LARGE) {};
        virtual ~TooLargeQueueException() throw() {};
    };*/

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
            : StoreException("Unable to find such message in DB !") {};
        
        NoSuchMessageException(SMSId id) 
            : StoreException() 
        {
            sprintf(cause, "Unable to find message with id = %u in DB !", id);
        };
        virtual ~NoSuchMessageException() throw() {};
    };

}}

#endif

