#ifndef _SCAG_EXCEPTIONS_
#define _SCAG_EXCEPTIONS_


#include <util/Exception.hpp>

namespace scag { namespace re { namespace exceptions {

using smsc::util::Exception;

class RuleEngineException : public Exception
{
public:

    RuleEngineException() : Exception() {};
    RuleEngineException(const char* fmt,...);
    virtual ~RuleEngineException() throw() {};

};


class InvalidPropertyException : public RuleEngineException
{
public:
    InvalidPropertyException(const char* fmt,...);
};


class ConvertException : public RuleEngineException
{
    static const char* message;
public:
    ConvertException(const char* val, const char* type);
    virtual ~ConvertException() throw() {};
};

class CompareException : public RuleEngineException
{
    static const char* message;
public:
    CompareException(const char * _message);
    virtual ~CompareException() throw() {};
};

class ConstantSetException : public RuleEngineException
{
    static const char* message;
public:
    ConstantSetException() : RuleEngineException(message) {};
    ConstantSetException(const char* fmt,...);
    virtual ~ConstantSetException() throw() {};
};


}}}

#endif 

