#include "SCAGExceptions.h"

namespace scag { namespace exceptions {

const char* ConvertException::message     = "Failed to convert '%s' to type %s";
const char* ConstantSetException::message = "Failed to assign value to constant";


SCAGException::SCAGException(const char* fmt,...) : Exception() 
{ 
    SMSC_UTIL_EX_FILL(fmt); 
}


InvalidPropertyException::InvalidPropertyException(const char* fmt,...) : SCAGException() 
{
    SMSC_UTIL_EX_FILL(fmt); 
}



ConvertException::ConvertException(const char* val, const char* type) : SCAGException(message, val, type) 
{ 
    /*SMSC_UTIL_EX_FILL(fmt);*/ 
}

CompareException::CompareException(const char * _message) : SCAGException(_message) 
{
    /*SMSC_UTIL_EX_FILL(_message);*/ 
}


ConstantSetException::ConstantSetException(const char* fmt,...) : SCAGException() 
{ 
    SMSC_UTIL_EX_FILL(fmt); 
}

}}
