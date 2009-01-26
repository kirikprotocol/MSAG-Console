#ifndef __SCAG_PVSS_PVAP_EXCEPTIONS_H__
#define __SCAG_PVSS_PVAP_EXCEPTIONS_H__

#include <string>
#include <stdio.h>
#include "scag/exc/SCAGExceptions.h"

namespace scag {
namespace pvss {
namespace pvap {

class PvapException : public scag::exceptions::SCAGException
{
public:
    virtual ~PvapException() throw () {}
};

class FieldIsNullException: public PvapException
{
public:
    FieldIsNullException(const char* field)
    {
        message = "Attempt to get field ";
        message += field;
        message += " which wasn't set.";
    }
    virtual ~FieldIsNullException() throw() {}
};

class MandatoryFieldMissingException : public PvapException {
public:
    MandatoryFieldMissingException(const char* field)
    {
        message = "Mandatory field ";
        message += field;
        message += " is missing.";
    }
    virtual ~MandatoryFieldMissingException() throw(){}
};

class DuplicateFieldException : public PvapException
{
public:
  DuplicateFieldException(const char* field)
  {
    message="Duplicate field ";
    message+=field;
    message+" found.";
  }
  ~DuplicateFieldException()throw(){}
};

class IncompatibleVersionException:public PvapException{
public:
  IncompatibleVersionException(const char* messageName)
  {
    message="Incompatible versions for message:";
    message+=messageName;
  }
};

class UnexpectedTag:public PvapException{
public:
  UnexpectedTag(const char* messageName,uint32_t tag)
  {
    char buf[32];
    sprintf(buf,"%u",tag);
    message="Unexpected tag in message '";
    message+=messageName;
    message+="':";
    message+=buf;
  }
};

class NetworkErrorException:public PvapException{
public:
  NetworkErrorException()
  {
    message="Network Error";
  }
  ~NetworkErrorException()throw(){}
};

class NetworkInitException:public PvapException{
public:
  NetworkInitException(const char* host,int port)
  {
    char buf[32];
    sprintf(buf,"%d",port);
    message="Failed to init server at ";
    message+=host;
    message+=':';
    message+=buf;
  }
};

class UnhandledMessage:public PvapException{
public:
  UnhandledMessage(uint32_t tag)
  {
    char buf[32];
    sprintf(buf,"%u",tag);
    message+="Message with tag ";
    message+=buf;
    message+=" not handled.";
  }
};

class ReadBeyondEof:public PvapException
{
public:
    ReadBeyondEof()
    {
        message = "Attempt to read data beyond end.";
    }
};

class InvalidValueLength : public PvapException 
{
public:
    InvalidValueLength(const char* valueType,int len)
    {
        message = "Invalid value length for type ";
        message += valueType;
        message += ":";
        char buf[32];
        sprintf(buf,"%d",len);
        message += buf;
    }
};

}
}
}

namespace scag2 {
namespace pvss {
namespace pvap = scag::pvss::pvap;
} // namespace pvss
} // namespace scag2

#endif
