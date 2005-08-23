#include "RuleEngineExceptions.h"

namespace scag { namespace re { namespace exceptions {

const char* ConvertException::message     = "Failed to convert '%s' to type %s";
const char* ConstantSetException::message = "Failed to assign value to constant";

PropertyException::PropertyException(const char* fmt,...) : Exception() 
{ 
    SMSC_UTIL_EX_FILL(fmt); 
}
  

ConvertException::ConvertException(const char* val, const char* type) : PropertyException(message, val, type) 
{ 
    /*SMSC_UTIL_EX_FILL(fmt);*/ 
}

CompareException::CompareException(const char * _message) : PropertyException(_message) 
{
    /*SMSC_UTIL_EX_FILL(_message);*/ 
}


ConstantSetException::ConstantSetException(const char* fmt,...) : PropertyException() 
{ 
    SMSC_UTIL_EX_FILL(fmt); 
}

}}}
