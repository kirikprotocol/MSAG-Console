/* $Id$ */

#ifndef SCAG_PERS_CLIENT_H
#define SCAG_PERS_CLIENT_H

#include "SerialBuffer.h"
#include "Property.h"
#include "core/network/Socket.hpp"
#include "Types.h"
#include "scag/lcm/LongCallManager.h"
#include "scag/config/pers/PersClientConfig.h"
#include "scag/lcm/LongCallManager.h"

namespace scag { namespace pers { namespace client {

using smsc::core::network::Socket;
using scag::pers;
using scag::lcm::LongCallParams;
using scag::lcm::LongCallContext;

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

class PersCallParams : public LongCallParams{
public:
    PersCallParams() : error(0), result(0) {};
    ProfileType pt;
    uint32_t ikey;
    std::string skey;
    Property prop;
    std::string propName;
    uint32_t mod;
    
    uint32_t error;
    uint32_t result;
    std::string exception;
};

class PersClient {
protected:
    virtual ~PersClient() {};

public:
    static PersClient& Instance();
    static void Init(const char *_host, int _port, int timeout, int pingTimeout);// throw(PersClientException);
    static void Init(const scag::config::PersClientConfig& cfg);// throw(PersClientException);    

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

    virtual bool call(LongCallContext* context) = 0;
protected:
    static bool  inited;
    static Mutex initLock;
};

}}}

#endif
