#ifndef SMSC_DBSME_DATASOURCE_EXCEPTIONS
#define SMSC_DBSME_DATASOURCE_EXCEPTIONS

#include <util/Exception.hpp>

namespace smsc { namespace db
{
    using smsc::util::Exception;

    class SQLException : public Exception
    {
    protected:
        
        int   code;
        int   status;
        
        SQLException(int _code=-1, int _status=0) 
            : Exception(""), code(_code) {};

    public:
        
        SQLException(const SQLException& exc) 
            : Exception(exc.what()),
                code(exc.getErrorCode()), status(exc.getErrorStatus())
        {};
        
        SQLException(const char* _cause, int _code=-1, int _status=0) 
            : Exception("Code: %d Status: %d, Failure cause: %s", 
                        _code, _status, _cause), code(_code), status(_status) 
        {};
        virtual ~SQLException() throw() {};
        
        inline int getErrorCode() const {
            return code;
        };
        inline int getErrorStatus() const {
            return status;
        };
    };

    class InvalidArgumentException : public Exception
    {
    public:
        
        InvalidArgumentException() 
            : Exception("Argument is invalid !") {};
        
        virtual ~InvalidArgumentException() throw() {};
    };
    
    class LoadupException : public Exception
    {
    public:
        
        LoadupException() 
            : Exception("Loadup failed !") {};
        
        virtual ~LoadupException() throw() {};
    };

}}

#endif

