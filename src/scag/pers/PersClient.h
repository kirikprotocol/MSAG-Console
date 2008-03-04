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
    CANT_CONNECT = 1,
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
    INVALID_PROPERTY_TYPE,
  BAD_REQUEST,
  TYPE_INCONSISTENCE,
  BATCH_ERROR,
  PROFILE_LOCKED
};

    static const char* strs[] = {
      "Unknown exception",
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
        "Invalid property type(should be int or date)",
      "Bad request",
      "Types inconsistence",
      "Batch prepare error",
      "Profile locked"
    };

class PersClientException{
protected:
    PersClientExceptionType et;
public:
    PersClientException(PersClientExceptionType e) { et = e; };
    PersClientExceptionType getType() const { return et; };
    const char* what() const { return strs[et]; };
};

union PersKey{
	const char* skey;
	uint32_t ikey;
	PersKey() {};
	PersKey(const char* s) {skey = s;};
	PersKey(uint32_t i) {ikey = i;};
};

class PersCallParams : public LongCallParams{
public:
    PersCallParams() : error(0), result(0) {};
    ProfileType pt;
	SerialBuffer sb;
	uint32_t ikey;
    std::string skey;
    std::string propName;
	Property prop;
    uint32_t mod;
    
    int32_t error;
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

    virtual void SetProperty(ProfileType pt, const PersKey& key, Property& prop) = 0;
    virtual void GetProperty(ProfileType pt, const PersKey& key, const char *property_name, Property& prop) = 0;
    virtual bool DelProperty(ProfileType pt, const PersKey& key, const char *property_name) = 0;
    virtual int IncProperty(ProfileType pt, const PersKey& key, Property& prop) = 0;
    virtual int IncModProperty(ProfileType pt, const PersKey& key, Property& prop, uint32_t mod) = 0;

    virtual void SetPropertyPrepare(ProfileType pt, const PersKey& key, Property& prop, SerialBuffer& bsb) = 0;
    virtual void GetPropertyPrepare(ProfileType pt, const PersKey& key, const char *property_name, SerialBuffer& bsb) = 0;
    virtual void DelPropertyPrepare(ProfileType pt, const PersKey& key, const char *property_name, SerialBuffer& bsb) = 0;
    virtual void IncPropertyPrepare(ProfileType pt, const PersKey& key, Property& prop, SerialBuffer& bsb) = 0;
    virtual void IncModPropertyPrepare(ProfileType pt, const PersKey& key, Property& prop, uint32_t mod, SerialBuffer& bsb) = 0;
    
    virtual void SetPropertyResult(SerialBuffer& bsb) = 0;
    virtual void GetPropertyResult(Property& prop, SerialBuffer& bsb) = 0;
    virtual bool DelPropertyResult(SerialBuffer& bsb) = 0;
    virtual int IncPropertyResult(SerialBuffer& bsb) = 0;
    virtual int IncModPropertyResult(SerialBuffer& bsb) = 0;

	virtual void PrepareBatch(SerialBuffer& bsb, bool transactMode = false) = 0;
	virtual void RunBatch(SerialBuffer& sb) = 0;
	virtual void FinishPrepareBatch(uint32_t cnt, SerialBuffer& bsb) = 0;
	
    virtual bool call(LongCallContext* context) = 0;
    
    virtual void Stop() = 0;
protected:
    static bool  inited;
    static Mutex initLock;
};

}}}

#endif
