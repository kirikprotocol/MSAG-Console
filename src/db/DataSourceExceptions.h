#ifndef SMSC_DBSME_DATASOURCE_EXCEPTIONS
#define SMSC_DBSME_DATASOURCE_EXCEPTIONS

#include <stdio.h>
#include <string.h>
#include <exception>

using std::exception;

namespace smsc { namespace db
{
    const int SMSC_MAX_ERROR_MESSAGE_LENGTH = 2048;

    class BaseException : public exception
    {
    protected:
        
        char  cause[SMSC_MAX_ERROR_MESSAGE_LENGTH];
        
        BaseException() {};

    public:
        
        BaseException(const char* _cause) {
            strcpy(cause, _cause);
        };
        
        virtual ~BaseException() throw() {};
       
        virtual const char* what() const throw() {
            return cause;
        };
        
    };
    
    class SQLException : public BaseException
    {
    protected:
        
        int   code;
        int   status;
        
        SQLException(int _code=-1, int _status=0) 
            : BaseException(), code(_code) {};

    public:
        
        SQLException(const SQLException& exc) 
            : BaseException(exc.what()),
                code(exc.getErrorCode()), status(exc.getErrorStatus())
        {};
        
        SQLException(const char* _cause, int _code=-1, int _status=0) 
            : BaseException(), code(_code), status(_status) 
        {
            sprintf(cause, "Code - %d, Status - %d, Failure cause - %s", 
                    _code, _status, _cause); 
        };
        virtual ~SQLException() throw() {};
        
        inline int getErrorCode() const {
            return code;
        };
        inline int getErrorStatus() const {
            return status;
        };
    };

    class InvalidArgumentException : public BaseException
    {
    public:
        
        InvalidArgumentException() 
            : BaseException("Argument is invalid !") {};
        
        virtual ~InvalidArgumentException() throw() {};
    };
    
    class LoadupException : public BaseException
    {
    public:
        
        LoadupException() 
            : BaseException("Loadup failed !") {};
        
        virtual ~LoadupException() throw() {};
    };

}}

#endif

