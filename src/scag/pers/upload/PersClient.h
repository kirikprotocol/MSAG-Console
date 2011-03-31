/* $Id$ */

#ifndef SCAG_PERS_UPLOAD_CLIENT_H
#define SCAG_PERS_UPLOAD_CLIENT_H
 
#include "core/network/Socket.hpp"
#include "scag/util/storage/SerialBuffer.h"
#include "scag/re/base/LongCallContextBase.h"
#include "scag/pvss/base/PersClientException.h"
#include "scag/pvss/base/Property.h"
#include "scag/pvss/base/Types.h"
#include "scag/pers/upload/PersKey.h"

namespace scag { namespace pers { namespace util {

using namespace pvss;
using smsc::core::network::Socket;
using scag2::lcm::LongCallParams;
using scag2::lcm::LongCallContextBase;
using scag::util::storage::SerialBuffer;
using scag::util::storage::SerialBufferOutOfBounds;

class PersCallParams : public LongCallParams {
public:
    PersCallParams() : LongCallParams(), error(0), result(0) {};
    ProfileType pt;
    SerialBuffer sb;
    uint32_t ikey;
    std::string skey;
    std::string propName;
    Property prop;
    uint32_t mod;
    
    int32_t error;
    uint32_t result;
};

class PersClient {
protected:
    virtual ~PersClient() {};

public:
    static PersClient& Instance();
    static void Init(const char *_host, int _port, int timeout, int pingTimeout, int _reconnectTimeout, int _maxCallsCount, int speed);// throw(PersClientException);

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

    virtual void SetPropertyPrepare(Property& prop, SerialBuffer& bsb) = 0;
    virtual void GetPropertyPrepare(const char *property_name, SerialBuffer& bsb) = 0;
    virtual void DelPropertyPrepare(const char *property_name, SerialBuffer& bsb) = 0;
    virtual void IncPropertyPrepare(Property& prop, SerialBuffer& bsb) = 0;
    virtual void IncModPropertyPrepare(Property& prop, uint32_t mod, SerialBuffer& bsb) = 0;
    
    virtual void SetPropertyResult(SerialBuffer& bsb) = 0;
    virtual void GetPropertyResult(Property& prop, SerialBuffer& bsb) = 0;
    virtual bool DelPropertyResult(SerialBuffer& bsb) = 0;
    virtual int IncPropertyResult(SerialBuffer& bsb) = 0;
    virtual int IncModPropertyResult(SerialBuffer& bsb) = 0;

    virtual void PrepareBatch(SerialBuffer& bsb, bool transactMode = false) = 0;
    virtual void PrepareMTBatch(SerialBuffer& bsb, ProfileType pt, const PersKey& key, uint16_t cnt, bool transactMode = false) = 0;
    virtual void RunBatch(SerialBuffer& sb) = 0;
    virtual void FinishPrepareBatch(uint32_t cnt, SerialBuffer& bsb) = 0;
	
    virtual bool call(LongCallContextBase* context) = 0;
    
    virtual void Stop() = 0;

    virtual int getClientStatus() = 0;
protected:
    static bool  inited;
    static Mutex initLock;
};

}//util
}//pers
}//scag

#endif
