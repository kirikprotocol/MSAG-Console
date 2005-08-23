#ifndef _RULE_ENGINE_EXCEPTIONS_
#define _RULE_ENGINE_EXCEPTIONS_


#include <util/Exception.hpp>

namespace scag { namespace re { namespace exceptions {

using smsc::util::Exception;


class PropertyException : public Exception
{
public:

    PropertyException() : Exception() {};
    PropertyException(const char* fmt,...);
    virtual ~PropertyException() throw() {};

};
class ConvertException : public PropertyException
{
    static const char* message;
public:
    ConvertException(const char* val, const char* type);
    virtual ~ConvertException() throw() {};
};

class CompareException : public PropertyException
{
    static const char* message;
public:
    CompareException(const char * _message);
    virtual ~CompareException() throw() {};
};

class ConstantSetException : public PropertyException
{
    static const char* message;
public:
    ConstantSetException() : PropertyException(message) {};
    ConstantSetException(const char* fmt,...);
    virtual ~ConstantSetException() throw() {};
};


}}}

#endif 

