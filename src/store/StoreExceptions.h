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

    const int MAX_MESSAGE_LENGTH = 1024;

    class StoreException : public exception
    {
    protected:
        
        char  cause[MAX_MESSAGE_LENGTH];
        
        StoreException() {};

    public:
        
        StoreException(const char* _cause) {
            sprintf(cause, "Failure cause - %s", _cause);
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
    
    class ConnectionFailedException : public StorageException
    {
    public:
        
        ConnectionFailedException(StorageException& exc) 
            : StorageException(exc.getErrorCode())
        {
            strcpy(cause, exc.what());
        };
        virtual ~ConnectionFailedException() throw() {};
    };
    
    class NoSuchMessageException : public StoreException
    {
    public:
        
        NoSuchMessageException() 
            : StoreException("Unable to find such message in DB !") {};
        
        NoSuchMessageException(SMSId id) : StoreException() 
        {
            sprintf(cause, "Failure cause - "
                    "Unable to find message with id = %d in DB !", id);
        };
        virtual ~NoSuchMessageException() throw() {};
    };

}}

#endif

