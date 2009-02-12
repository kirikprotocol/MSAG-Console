#ifndef __SCAG_PVSS_PVAP_EXCEPTIONS_H__
#define __SCAG_PVSS_PVAP_EXCEPTIONS_H__

#include <string>
#include <stdio.h>
#include "util/int.h"
#include "scag/exc/SCAGExceptions.h"
#include "scag/pvss/common/PvapException.h"

namespace scag2 {
namespace pvss {
namespace pvap {

/// exception used in Serializer
class IOException : public scag::exceptions::SCAGException
{
protected:
    IOException() {}
public:
    IOException( const char* fmt, ... ) : SCAGException() {
        SMSC_UTIL_EX_FILL(fmt);
    }
};

class ReadBeyondEof : public IOException
{
public:
    ReadBeyondEof() : IOException( "Attempt to read data beyond end of buffer" ) {}
};


class InvalidValueLength : public IOException
{
public:
    InvalidValueLength( const char* valueType, int len ) :
    IOException( "Invalid value length for type %s: len=%d", valueType, len ) {}
};


// ============================

class FieldIsNullException: public PvapException
{
public:
    FieldIsNullException(bool isRequest, const char* field, uint32_t seqNum) :
    PvapException(isRequest, seqNum)
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


class PvapSerializationException : public PvapException
{
public:
    PvapSerializationException( uint32_t seqNum, const char* msg, ... ) : PvapException(seqNum) {
        SMSC_UTIL_EX_FILL(msg);
    }
    PvapSerializationException( bool isreq, uint32_t seqNum, const char* msg, ... ) :
    PvapException(isreq,seqNum) {
        SMSC_UTIL_EX_FILL(msg);
    }
};


class MessageIsBrokenException : public PvapException
{
public:
    MessageIsBrokenException( bool isreq, const std::string& msg, uint32_t seqNum ) :
    PvapException(isreq,seqNum) {
        message = msg;
    }
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
    InvalidMessageTypeException(uint32_t seqNum, int tag, const char* messageName = "") :
    PvapException(true,seqNum,tag)
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

}
}
}

namespace scag {
namespace pvss {
namespace pvap {
using namespace scag2::pvss::pvap;
} // namespace pvap
} // namespace pvss
} // namespace scag2

#endif
