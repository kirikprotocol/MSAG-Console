#ifndef __SCAG_PVSS_PVAP_EXCEPTIONS_H__
#define __SCAG_PVSS_PVAP_EXCEPTIONS_H__

#include <string>
#include <stdio.h>
#include "util/int.h"
#include "scag/exc/SCAGExceptions.h"

namespace scag {
namespace pvss {
namespace pvap {

class PvapException : public scag::exceptions::SCAGException
{
protected:
    PvapException( uint32_t seqNum = (uint32_t)-1 ) : seqNum_(seqNum), isRequest_(true) {}
    PvapException( bool isRequest, uint32_t seqNum, int tag = 0 ) : seqNum_(seqNum), isRequest_(isRequest), tag_(tag) {}

public:
    virtual ~PvapException() throw () {}

    uint32_t getSeqNum() const { return seqNum_; }
    bool isRequest() const { return isRequest_; }
    int getTag() const { return tag_; }

private:
    uint32_t seqNum_;
    bool     isRequest_;
    int      tag_;
};

class FieldIsNullException: public PvapException
{
public:
    FieldIsNullException(const char* field, uint32_t seqNum) :
    PvapException(seqNum)
    {
        message = "Attempt to get field ";
        message += field;
        message += " which wasn't set.";
    }
    virtual ~FieldIsNullException() throw() {}
};

class NotImplementedException : public PvapException
{
public:
    NotImplementedException(const char* field)
    {
        message = "Processing of ";
        message += field;
        message += " field is not implemented yet";
    }
    virtual ~NotImplementedException() throw() {}
};

class MandatoryFieldMissingException : public PvapException {
public:
    MandatoryFieldMissingException(bool isRequest, const char* field, uint32_t seqNum ) :
    PvapException(isRequest, seqNum)
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
    DuplicateFieldException( bool isRequest, const char* field, int seqNum ) :
    PvapException(isRequest,seqNum)
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

class InvalidMessageTypeException : public PvapException 
{
public:
    InvalidMessageTypeException( int tag, const char* messageName = "")
    {
        char buf[32];
        sprintf(buf,"%u",tag);
        message="Unexpected tag in message '";
        message+=messageName;
        message+="':";
        message+=buf;
    }
};

class InvalidFieldTypeException : public PvapException 
{
public:
    InvalidFieldTypeException(bool isRequest, const char* messageName, uint32_t seqNum, int tag ) :
    PvapException(isRequest,seqNum,tag)
    {
        char buf[32];
        sprintf(buf,"%u",tag);
        message="Unexpected tag in message '";
        message+=messageName;
        message+="':";
        message+=buf;
    }
};

class UnexpectedSeqNumException : public PvapException
{
public:
    UnexpectedSeqNumException( int tag, const char* messageName = "")
    {
        char buf[32];
        sprintf(buf,"%u",tag);
        message="Unexpected seqNum in message '";
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
