
#include "DataSourceExceptions.h"

namespace smsc { namespace db
{

SQLException::SQLException(int _code, int _status) 
    : Exception(""), code(_code) 
{
}
SQLException::SQLException(const SQLException& exc) 
    : Exception(exc.what()),
        code(exc.getErrorCode()), status(exc.getErrorStatus())
{
}
SQLException::SQLException(const char* _cause, int _code, int _status) 
    : Exception("Code: %d Status: %d, Failure cause: %s", 
            _code, _status, _cause), code(_code), status(_status) 
{
}
SQLException::~SQLException() throw() 
{
}
        
inline int SQLException::getErrorCode() const 
{
    return code;
}
inline int SQLException::getErrorStatus() const 
{
    return status;
}
   
InvalidArgumentException::InvalidArgumentException() 
    : Exception("Argument is invalid !") 
{
}
InvalidArgumentException::~InvalidArgumentException() throw() 
{
}

LoadupException::LoadupException() 
    : Exception("Loadup failed !") 
{
}
LoadupException::~LoadupException() throw() 
{
}

}}


