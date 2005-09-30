#ifndef _SCAG_EXCEPTIONS_
#define _SCAG_EXCEPTIONS_


#include <util/Exception.hpp>

namespace scag { namespace exceptions {

using smsc::util::Exception;

class SCAGException : public Exception
{
public:

    SCAGException(const Exception& exc) : Exception(exc.what()) {};
    SCAGException() : Exception() {};
    SCAGException(const char* fmt,...);
    virtual ~SCAGException() throw() {};

};

class RuleEngineException : public SCAGException
{
    int m_nLineNumber;
public:
    int getLineNumber() const {return m_nLineNumber;};
    RuleEngineException(int nLine, const char* fmt,...);
    virtual ~RuleEngineException() throw() {};
};


class InvalidPropertyException : public SCAGException
{
public:
    InvalidPropertyException(const char* fmt,...);
};


class ConvertException : public SCAGException
{
    static const char* message;
public:
    ConvertException(const char* val, const char* type);
    virtual ~ConvertException() throw() {};
};

class CompareException : public SCAGException
{
    static const char* message;
public:
    CompareException(const char * _message);
    virtual ~CompareException() throw() {};
};

class ConstantSetException : public SCAGException
{
    static const char* message;
public:
    ConstantSetException() : SCAGException(message) {};
    ConstantSetException(const char* fmt,...);
    virtual ~ConstantSetException() throw() {};
};


}}

#endif 

