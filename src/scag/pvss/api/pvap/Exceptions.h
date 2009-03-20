#ifndef __SCAG_PVSS_PVAP_EXCEPTIONS_H__
#define __SCAG_PVSS_PVAP_EXCEPTIONS_H__

#include <string>
#include <stdio.h>
#include "util/int.h"
#include "scag/exc/SCAGExceptions.h"
#include "scag/exc/IOException.h"
#include "scag/pvss/common/PvapException.h"

namespace scag2 {
namespace pvss {
namespace pvap {

class ReadBeyondEof : public exceptions::IOException
{
public:
    ReadBeyondEof() : exceptions::IOException( "Attempt to read data beyond end of buffer" ) {}
};


class InvalidValueLength : public exceptions::IOException
{
public:
    InvalidValueLength( const char* valueType, int len ) :
    exceptions::IOException( "Invalid value length for type %s: len=%d", valueType, len ) {}
};


// ============================

/*
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
 */

class NotImplementedException : public PvapException
{
public:
    NotImplementedException(const char* field) :
    PvapException(PvssException::UNKNOWN, uint32_t(-1), "Processing of field '%s' is not impl yet", field) {}
    virtual ~NotImplementedException() throw() {}
};

/*
class MandatoryFieldMissingException : public PvapException 
{
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
 */

class DuplicateFieldException : public PvapException
{
public:
    DuplicateFieldException( bool isRequest, const char* field, uint32_t seqNum ) :
    PvapException(getExcType(isRequest),seqNum,"Duplicate field '%s' found", field) {}
    ~DuplicateFieldException()throw(){}
};


class PvapSerializationException : public PvapException
{
public:
    PvapSerializationException( uint32_t seqNum, const char* msg, ... ) :
    PvapException(PvssException::UNKNOWN,seqNum) {
        SMSC_UTIL_EX_FILL(msg);
        showSeqNum();
    }
    PvapSerializationException( bool isreq, uint32_t seqNum, const char* msg, ... ) :
    PvapException(getExcType(isreq),seqNum) {
        SMSC_UTIL_EX_FILL(msg);
        showSeqNum();
    }
};


class MessageIsBrokenException : public PvapException
{
public:
    MessageIsBrokenException( bool isreq, uint32_t seqNum, const char* fmt, ... ) :
    PvapException(getExcType(isreq),seqNum) {
        SMSC_UTIL_EX_FILL(fmt);
        showSeqNum();
    }
};


/*
class IncompatibleVersionException:public PvapException
{
public:
  IncompatibleVersionException(const char* messageName)
  {
    message="Incompatible versions for message:";
    message+=messageName;
  }
};
 */

class InvalidMessageTypeException : public PvapException 
{
public:
    InvalidMessageTypeException(uint32_t seqNum, int tag) :
    PvapException(UNKNOWN,seqNum,"Unexpected message type: %d",tag) {}
};

class InvalidFieldTypeException : public PvapException 
{
public:
    InvalidFieldTypeException(bool isRequest, const char* messageName, uint32_t seqNum, int tag ) :
    PvapException(getExcType(isRequest),seqNum,"Invalid tag (%d) in message %s", tag, messageName) {}
};

class UnexpectedSeqNumException : public PvapException
{
public:
    UnexpectedSeqNumException(uint32_t seqNum) :
    PvapException(PvssException::UNKNOWN,seqNum,"Unexpected seqNum in message") {}
};

/*
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
 */

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
