#ifndef STRORE_EXCEPTIONS_DECLARATIONS
#define STRORE_EXCEPTIONS_DECLARATIONS

#include <stdio.h>
#include <string.h>
#include <exception>
#include <sms/sms.h>

using std::exception;

namespace smsc { namespace store
{
    using smsc::sms::SMSId;

    const int MAX_MESSAGE_LENGTH = 2048;

    class StoreException : public exception
    {
    protected:
        
        char  cause[MAX_MESSAGE_LENGTH];
        
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
    
    class TooLargeQueueException : public StoreException
    {
    public:

        TooLargeQueueException() 
            : StoreException("Too many pending requests a waiting "
                             "for store connections !") {};
        
        virtual ~TooLargeQueueException() throw() {};
    };

    class StorageException : public StoreException
    {
    protected:
        
        int   code;
        
        StorageException(int _code=-1) 
            : StoreException(), code(_code) {};

    public:
        
        StorageException(const char* _cause, int _code=-1) 
            : StoreException(), code(_code) 
        {
            sprintf(cause, "Code - %d, Failure cause - %s", _code, _cause);
        };
        virtual ~StorageException() throw() {};
        
        inline int getErrorCode() {
            return code;
        };
    };
    
    class ConnectionFailedException : public StoreException
    {
    public:
        
        ConnectionFailedException(StoreException& exc)
            : StoreException(exc.what()) {};
        
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

