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
        
        SQLException(int _code=-1, int _status=0);

    public:
        
        SQLException(const SQLException& exc);
        SQLException(const char* _cause, int _code=-1, int _status=0);
        virtual ~SQLException() throw();
        
        inline int getErrorCode() const;
        inline int getErrorStatus() const;
    };

    class InvalidArgumentException : public Exception
    {
    public:
        
        InvalidArgumentException();
        virtual ~InvalidArgumentException() throw();
    };
    
    class LoadupException : public Exception
    {
    public:
        
        LoadupException(); 
        virtual ~LoadupException() throw();
    };

}}

#endif

