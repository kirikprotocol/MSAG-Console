/* $Id$ */

#ifndef SCAG_PERS_CLIENT_H
#define SCAG_PERS_CLIENT_H

#include "logger/Logger.h"
#include "SerialBuffer.h"
#include "Property.h"
#include "core/network/Socket.hpp"
#include "Types.h"
#include "scag/config/pers/PersClientConfig.h"

namespace scag { namespace pers { namespace client {

using smsc::core::network::Socket;

using smsc::logger::Logger;
using scag::pers;
using namespace scag::config;

enum PersClientExceptionType{
    CANT_CONNECT,
    SERVER_BUSY,
    UNKNOWN_RESPONSE,
    SEND_FAILED,
    READ_FAILED,
    TIMEOUT,
    NOT_CONNECTED,
    BAD_RESPONSE,
    SERVER_ERROR,
    PROPERTY_NOT_FOUND,
    INVALID_KEY,
    INVALID_PROPERTY_TYPE
};

    static const char* strs[] = {
        "Cant connect to persserver",
        "Server busy",
        "Unknown server response",
        "Send failed",
        "Read failed",
        "Read/write timeout",
        "Not connected",
        "Bad response",
        "Server error",
        "Property not found",
        "Invalid key",
        "Invalid property type(should be int or date)"
    };

class PersClientException{
protected:
    PersClientExceptionType et;
public:
    PersClientException(PersClientExceptionType e) { et = e; };
    PersClientExceptionType getType() { return et; };
    const char* what() const { return strs[et]; };
};

class PersClient {
protected:
    virtual ~PersClient() {};

public:
    static PersClient& Instance();
    static void Init(const char *_host, int _port, int timeout, int pingTimeout);// throw(PersClientException);
    static void Init(const PersClientConfig& cfg);// throw(PersClientException);    

    virtual void SetProperty(ProfileType pt, const char* key, Property& prop) = 0;// throw(PersClientException);
    virtual void SetProperty(ProfileType pt, uint32_t key, Property& prop) = 0;// throw(PersClientException);

    virtual void GetProperty(ProfileType pt, const char* key, const char *property_name, Property& prop) = 0;// throw(PersClientException);
    virtual void GetProperty(ProfileType pt, uint32_t key, const char *property_name, Property& prop) = 0;// throw(PersClientException);

    virtual void DelProperty(ProfileType pt, const char* key, const char *property_name) = 0;// throw(PersClientException);
    virtual void DelProperty(ProfileType pt, uint32_t key, const char *property_name) = 0;// throw(PersClientException);

    virtual void IncProperty(ProfileType pt, const char* key, Property& prop) = 0;// throw(PersClientException);
    virtual void IncProperty(ProfileType pt, uint32_t key, Property& prop) = 0;// throw(PersClientException);

    virtual int IncModProperty(ProfileType pt, const char* key, Property& prop, uint32_t mod) = 0; //throw(PersClientException)
    virtual int IncModProperty(ProfileType pt, uint32_t key, Property& prop, uint32_t mod) = 0; //throw(PersClientException)

protected:
    static bool  inited;
    static Mutex initLock;
};

}}}

#endif

