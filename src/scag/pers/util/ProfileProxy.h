#ifndef _SCAG_PERS_UTIL_PROFILEPROXY_H
#define _SCAG_PERS_UTIL_PROFILEPROXY_H

#if 0

#include <vector>
#include "Types.h"
#include "scag/util/storage/SerialBuffer.h"
#include "scag/re/base/LongCallContextBase.h"
#include "PersClientException.h"

namespace scag2 {

namespace re {
namespace actions {
class ActionContext;
}
}

namespace pers {
namespace util {

using scag2::util::storage::SerialBuffer;

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
    int readServerStatus( re::actions::ActionContext& ctx, SerialBuffer& sb );
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
                    const std::string& msgName );

    void setKey( const std::string& key );
    void setKey( int32_t key );

    const char* getStringKey() const { return skey_.c_str(); }
    int32_t getIntKey() const { return ikey_; };

    // inline ProfileProxy& proxy() { return proxy_; }

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

    inline bool hasBeenSent() const { return hasBeenSent_; }

    inline SerialBuffer& buffer() { return buf_; }

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
    bool                            hasBeenSent_; // true if the call has been sent to server
};

}
}
}

#endif

#endif /* ! _SCAG_PERS_UTIL_PROFILEPROXY_H */
