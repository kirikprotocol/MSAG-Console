#ifndef _SCAG_SESSIONS_SESSION2_H
#define _SCAG_SESSIONS_SESSION2_H

#include <list>

// #include "util/stringhash.hpp"
#include "SessionKey.h"
#include "core/buffers/Hash.hpp"
#include "core/buffers/IntHash.hpp"
#include "logger/Logger.h"
#include "scag/util/properties/Properties.h"
#include "scag/util/storage/Serializer.h"
#include "scag/transport/SCAGCommand2.h"


namespace scag {
namespace sessions2 {

    using namespace smsc::logger;
    using namespace scag::util::properties;
    using namespace smsc::core::buffers;
    using namespace scag::util::storage;
    using namespace scag::transport2;


/// Scope of properties (e.g. global, service, context, operation)
class SessionPropertyScope
{
public:
    SessionPropertyScope( Changeable* patron ) : patron_(patron) {}
    ~SessionPropertyScope();
    Property* getProperty( const std::string& name );

    Serializer& serialize( Serializer& s ) const;
    Deserializer& deserialize( Deserializer& s ) throw (DeserializerException);

    // clear the scope
    void clear();

    unsigned size() const { return properties_.GetCount(); }

private:
    Changeable*              patron_;
    Hash< AdapterProperty* > properties_; // owned
};


class Operation;
class ExternalTransaction;

///
/// a new realization of msag Session
///
class Session : public Changeable
{
private:
    Session();
    Session( const Session& s );
    Session& operator = ( const Session& s );

public:
    Session( const SessionKey& key );

    virtual ~Session();

    Serializer& serialize( Serializer& s ) const;
    Deserializer& deserialize( Deserializer& s ) throw (DeserializerException);

    /// clear internal state except session key, and the command queue.
    /// reset expiration policy and creation time.
    void clear();

    /// notification when property is changed
    virtual void changed( AdapterProperty& prop );

    inline const SessionKey& sessionKey() const {
        return key_;
    }

    /// --- property Scopes, for use from ActionContext

    /// create context Scope.
    /// @return false if the Scope with userid is already exist
    bool createContextScope( int userid );

    /// @return session global Scope
    SessionPropertyScope* getGlobalScope();

    /// @return service Scope by id
    SessionPropertyScope* getServiceScope( int servid );

    /// @return context Scope by id (may return NULL)
    SessionPropertyScope* getContextScope( int contextid );
    
    /// @return current operation Scope (created on-demand)
    SessionPropertyScope* getOperationScope();

    /// @return session expiration time in GMT (as returned by time(2) )
    time_t expirationTime() const;



    // === the following methods should be invoked from session store only

    /// current processing command (owned)
    /// the presence of a command tells that session is locked.
    inline void setCurrentCommand( SCAGCommand* cmd ) {
        if ( command_ && command_ != cmd ) delete command_;
        command_ = cmd;
    }

    inline SCAGCommand* currentCommand() {
        return command_;
    }

    /// append command to the list of session commands.
    /// @return the size of command queue.
    /// NOTE: this method should be invoked from SessionStore only (as it requires locking).
    /// NOTE: cmd gets owned.
    unsigned appendCommand( SCAGCommand* cmd );

    /// pop one command from the list of session commands.
    /// NOTE: this method should be invoked from SessionStore only (as it requires locking).
    /// NOTE: cmd disowned, may return NULL.
    SCAGCommand* popCommand();

private: // methods

    void serializeScope( Serializer& o, const SessionPropertyScope* s ) const;
    void deserializeScope( Deserializer& o, SessionPropertyScope*& s ) throw (DeserializerException);
    void serializeScopeHash( Serializer& o, const IntHash< SessionPropertyScope* >* s ) const;
    void deserializeScopeHash( Deserializer& o, IntHash< SessionPropertyScope* >*& s ) throw (DeserializerException);
    void clearScopeHash( IntHash< SessionPropertyScope* >* s );
    void abort();

private: // statics

    static Logger* log_;

private:
    /// session key (msisdn)
    SessionKey     key_;

    /// expiration policy (COMMON,FIXED,ACCESS) (pers)
    ///   COMMON: if has operations, die after last operations die, otherwise
    ///         die after createTime+TMO;
    ///   FIXED: die after createTime+TMO;
    ///   ACCESS: die after lastAccessTime+TMO;
    enum { COMMON = 0, FIXED = 1, ACCESS = 2 }
    expirationPolicy_;

    /// session create time
    timeval bornTime_;

    /// last access time (pers)
    time_t lastAccessTime_;

    /// expiration Time
    time_t expirationTime_;

    /// current command being processed, it locks the session (owned, not pers).
    SCAGCommand* command_;

    /// the list of pending commands (owned, not pers).
    std::list< SCAGCommand* > cmdQueue_;

    /// last operation serial number (int)
    uint64_t lastOperationId_;

    /// ussd operation serial number (or 0 if n/a)
    uint64_t ussdOperationId_;
    
    /// the hash of operations (int -> Operation)
    IntHash< Operation* > operations_;

    /// the hash of external transactions (string -> transaction)
    Hash< ExternalTransaction* >* transactions_;

    /// --- property Scopes

    /// global var Scope
    SessionPropertyScope* globalScope_;

    /// the hash of service var Scopes (int(service_id) -> SessionServiceScope)
    IntHash< SessionPropertyScope* >* serviceScopes_;
    
    /// the hash of context var Scopes (int(context_id) -> SessionContextScope)
    IntHash< SessionPropertyScope* >* contextScopes_;

    /// the hash of operation var Scopes (int(op_id) -> SessionOperationScope)
    IntHash< SessionPropertyScope* >* operationScopes_;

};


class SessionStore;


/// this class is to guarantee that the session is returned to store
/// NOTE: not a thread-safe
class ActiveSession
{
public:
    ActiveSession() : s_(0) {}
    ActiveSession( SessionStore& st, Session& s );
    // store_(&st), s_(&s), cmd_(&cmd), flush_(false) {}

    ~ActiveSession() { if (s_) release(); }
    ActiveSession( const ActiveSession& as ) : s_(0) { copy(as); }
    ActiveSession& operator = ( const ActiveSession& as ) {
        if ( &as != this ) copy(as);
        return *this;
    }

    inline Session* get() const {
        return s_;
    }

    inline Session& operator* () const {
        return *s_;
    }

    inline Session* operator-> () const {
        return s_;
    }

    /// get/set flush state of session
    void setFlush( bool f ) { flush_ = f; }
    bool getFlush() const { return flush_; }

    /// explcitly release current session
    void release();

private:
    inline void copy( const ActiveSession& as ) {
        if (s_) release();
        store_ = as.store_;
        s_ = as.s_;
        flush_ = as.flush_;
        as.s_ = 0;
    }
private:
    SessionStore*    store_;
    mutable Session* s_;
    bool             flush_;
};

} // namespace sessions
} // namespace scag

inline scag::util::storage::Serializer& operator << ( scag::util::storage::Serializer& o, const scag::sessions2::Session& s )
{
    return s.serialize(o);
}
inline scag::util::storage::Deserializer& operator >> ( scag::util::storage::Deserializer& o, scag::sessions2::Session& s )
{
    return s.deserialize(o);
}

#endif /* !_SCAG_SESSIONS_SESSION2_H */
