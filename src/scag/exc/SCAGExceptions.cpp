#include "SCAGExceptions.h"

namespace scag { namespace re { namespace exceptions {

const char* ConvertException::message     = "Failed to convert '%s' to type %s";
const char* ConstantSetException::message = "Failed to assign value to constant";


RuleEngineException::RuleEngineException(const char* fmt,...) : Exception() 
{ 
    SMSC_UTIL_EX_FILL(fmt); 
}


InvalidPropertyException::InvalidPropertyException(const char* fmt,...) : RuleEngineException() 
{
}



ConvertException::ConvertException(const char* val, const char* type) : RuleEngineException(message, val, type) 
{ 
    /*SMSC_UTIL_EX_FILL(fmt);*/ 
}

CompareException::CompareException(const char * _message) : RuleEngineException(_message) 
{
    /*SMSC_UTIL_EX_FILL(_message);*/ 
}


ConstantSetException::ConstantSetException(const char* fmt,...) : RuleEngineException() 
{ 
    SMSC_UTIL_EX_FILL(fmt); 
}

}}}
