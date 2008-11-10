#ifndef _SCAG_PERS_UTIL_PROFILEPROXY_H
#define _SCAG_PERS_UTIL_PROFILEPROXY_H

#include <vector>
#include "Types.h"
#include "scag/util/storage/SerialBuffer.h"
#include "scag/re/base/LongCallContextBase.h"

namespace scag2 {

namespace re {
namespace actions {
class ActionContext;
}
}

namespace pers {
namespace util {

using scag2::util::storage::SerialBuffer;

enum PersClientExceptionType {
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
  PROFILE_LOCKED,
  COMMAND_NOTSUPPORT,
  CLIENT_BUSY
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
  "Profile locked",
  "Command Not Supports",
  "Client busy"
};

class PersClientException
{
public:
    PersClientException(PersClientExceptionType e) { et = e; };
    PersClientExceptionType getType() const { return et; };
    const char* what() const { return strs[et]; };
protected:
    PersClientExceptionType et;
};

union PersKey
{
    const char* skey;
    uint32_t ikey;
    PersKey() {};
    PersKey(const char* s) {skey = s;};
    PersKey(uint32_t i) {ikey = i;};
};


class ProfileProxy
{
public:
    ProfileProxy( ProfileType pt, SerialBuffer& sb ) :
    type_(pt), buf_(sb) {}

    inline ProfileType getType() const { return type_; }

    inline void setKey( const PersKey& key ) { key_ = key; }

    inline const PersKey& getKey() const { return key_; }

    inline SerialBuffer& buffer() { return buf_; }

private:
    ProfileProxy();
    ProfileProxy( const ProfileProxy& );
    ProfileProxy& operator = ( const ProfileProxy& );

private:
    ProfileType                       type_;
    PersKey                           key_;
    SerialBuffer&                     buf_;      // NOTE the reference
};


// FIXME: move to re::actions


class PropertyProxyBase
{
public:
    virtual const std::string& statusName() const = 0;
    virtual const std::string& msgName() const = 0;
    bool canProcessRequest( re::actions::ActionContext& ctx );
    void setStatus( re::actions::ActionContext& ctx,
                    int status,
                    int actionIdx = 0 );
};


class PropertyProxy : public PropertyProxyBase
{
public:
    virtual PersCmd cmdType() const = 0;
    virtual int fillSB( re::actions::ActionContext& ctx, SerialBuffer& sb ) = 0;
    virtual int readSB( re::actions::ActionContext& ctx, SerialBuffer& sb ) = 0;
};


class PersCallParams : public lcm::LongCallParams, public PropertyProxy
{
public:
    PersCallParams( ProfileType pt,
                    const std::string& statusName,
                    const std::string& msgName ) :
    error(0),
    ikey_(0),
    proxy_(pt, buf_),
    single_(0), batch_(0), statusName_(statusName), msgName_(msgName) {}

    void setKey( const std::string& key );
    void setKey( int32_t key );

    const char* getStringKey() const { return skey_.c_str(); }
    int32_t getIntKey() const { return ikey_; };

    inline ProfileProxy& proxy() { return proxy_; }

    /// set batch operation
    void batch( std::vector< PropertyProxy* >& proxies, bool transact );

    /// set non-batch operation
    void single( PropertyProxy& proxy );

    /// NOTE: I intentionally splitted different actions into separate methods
    /// to allow moving them around.

    /// fill buffer and set status if something goes wrong
    /// @return PersClientExceptionType or 0 if ok
    bool fillSB( re::actions::ActionContext& ctx );

    /// get results (simply parse buffer).
    void readSB( re::actions::ActionContext& ctx );

private:
    virtual pers::util::PersCmd cmdType() const;
    virtual int fillSB( re::actions::ActionContext& ctx, SerialBuffer& sb );
    virtual int readSB( re::actions::ActionContext& ctx, SerialBuffer& sb );
    virtual const std::string& statusName() const { return statusName_; }
    virtual const std::string& msgName() const { return msgName_; }
    
public:
    int32_t                         error;     // PersClientExceptionType

private:
    SerialBuffer                    buf_;
    std::string                     skey_;
    int32_t                         ikey_;
    ProfileProxy                    proxy_;
    PropertyProxy*                  single_;   // for single commands (not owned)
    std::vector< PropertyProxy* >*  batch_;    // for batch commands  (not owned)
    bool                            transact_;
    const std::string&              statusName_;
    const std::string&              msgName_;
};

}
}
}

#endif /* ! _SCAG_PERS_UTIL_PROFILEPROXY_H */
