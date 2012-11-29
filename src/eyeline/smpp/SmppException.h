#ifndef _EYELINE_SMPP_SMPPEXCEPTION_H
#define _EYELINE_SMPP_SMPPEXCEPTION_H

#include "util/Exception.hpp"
#include "informer/io/Strerror.h"
#include "Status.h"

namespace eyeline {
namespace smpp {

/// base exception
class SmppException : public smsc::util::Exception
{
public:
    SmppException(const char* fmt, ...) : status_(Status::OK) {
        SMSC_UTIL_EX_FILL(fmt);
    }
    SmppException(int status, const char* fmt, ...) : status_(status) {
        SMSC_UTIL_EX_FILL(fmt);
    }
    ~SmppException() throw () {
//        fprintf(stderr,"destroying exc %p %s\n",this,what());
    }
    inline int getStatus() const { return status_; }
protected:
    explicit SmppException( int status = Status::OK ) : status_(status) {}

private:
    int status_;
};


/// thrown at decode()/encode()/read() when pdu/buffer len is too small
class SmppPduBadLenException : public SmppException
{
public:
    SmppPduBadLenException(const char* fmt, ...) : SmppException() {
        SMSC_UTIL_EX_FILL(fmt);
    }
protected:
    SmppPduBadLenException() {}
};


class SmppTLVBadLenException : public SmppPduBadLenException
{
public:
    SmppTLVBadLenException(const char* fmt,...) : SmppPduBadLenException() {
        SMSC_UTIL_EX_FILL(fmt);
    }
};


class SmppTLVNotFoundException : public SmppException
{
public:
    SmppTLVNotFoundException(uint16_t tag, const char* fmt, ...) :
    SmppException(), tag_(tag) {
        SMSC_UTIL_EX_FILL(fmt);
    }
    uint16_t getTag() const { return tag_; }
private:
    uint16_t tag_;
};

}
}

#endif
