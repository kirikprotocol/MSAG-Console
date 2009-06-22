#ifndef _SCAG_EXCEPTIONS_
#define _SCAG_EXCEPTIONS_

#include <util/Exception.hpp>

namespace scag {
namespace exceptions {

class SCAGException : public smsc::util::Exception
{
public:

    SCAGException(const smsc::util::Exception& exc) : smsc::util::Exception(exc) {}
    SCAGException() : smsc::util::Exception() {};
    SCAGException(const char* fmt,...);
    SCAGException(const std::string& exc) : smsc::util::Exception() {
        message = exc;
    }
    virtual ~SCAGException() throw() {};
    inline const std::string& getMessage() const { return message; }
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

namespace scag2 {
namespace exceptions {
using namespace scag::exceptions;
}
}

#endif 

