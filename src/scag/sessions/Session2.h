#ifndef _SCAG_SESSIONS_SESSION2_H
#define _SCAG_SESSIONS_SESSION2_H

#include <list>

// #include "util/stringhash.hpp"
#include "SessionKey.h"
#include "core/buffers/Hash.hpp"
#include "core/buffers/IntHash.hpp"
#include "logger/Logger.h"
#include "scag/util/properties/Properties2.h"
#include "scag/util/storage/Serializer.h"
#include "scag/transport/SCAGCommand2.h"
#include "scag/lcm/LongCallManager2.h"


namespace scag2 {
namespace sessions {

using lcm::LongCallContext;
using namespace smsc::logger;
using namespace scag::util::properties;
using namespace smsc::core::buffers;
using namespace scag::util::storage;
using namespace transport;


enum ProtocolForEvent
{
    PROTOCOL_SMPP_SMS =  1,
    PROTOCOL_SMPP_USSD = 2,
    PROTOCOL_HTTP = 3,
    PROTOCOL_MMS = 4
};


enum ICCOperationStatus
{
    OPERATION_INITED = 1,
    OPERATION_CONTINUED,
    OPERATION_COMPLETED
};


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

    inline const SessionPrimaryKey& sessionPrimaryKey() const {
        return pkey_;
    }

    /// get current operation
    Operation* getCurrentOperation();

    /// destroy current operation
    void closeCurrentOperation();

    /// FIXME: should it be here?
    LongCallContext& getLongCallContext() {
        return lcmCtx_;
    }

    bool isNew() const;
    void setNew( bool anew );

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

    /// current processing command (owned).
    /// the presence of a command tells that session is locked.
    /// \return the previous owned command.
    /// It gives the possibility to destroy it after unlocking.
    inline SCAGCommand* setCurrentCommand( SCAGCommand* cmd ) {
        SCAGCommand* prev = command_;
        if ( command_ ) {
            if ( command_ == cmd ) {
                prev = 0;
                /*
            } else if ( cmd ) {
                smsc_log_warn(log_,"session %p has changed command: session->cmd=%p cmd=%p", this, command_, cmd );
                 */
            }
        }
        command_ = cmd;
        return prev;
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

    // session create time is incorporated into primarykey
    // timeval bornTime_;
    SessionPrimaryKey  pkey_;

    /// last access time (pers)
    time_t lastAccessTime_;

    /// expiration Time
    time_t expirationTime_;

    /// current command being processed, it locks the session (owned, not pers).
    SCAGCommand* command_;

    /// FIXME: should it be here?
    LongCallContext  lcmCtx_;

    /// if the session is just created
    bool isnew_;

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


    /// explcitly release current session.
    void release();

    /// move lock of the session to the specified command.
    /// An example: when processSubmit failed we would like to
    /// pass lock on the session immediately to processSubmitResp.
    void moveLock( SCAGCommand* cmd );

    /// leave session locked.
    /// Typically invoked after successfull call to longcallmanager
    /// to make sure the session is locked for processing the command.
    void leaveLocked() { s_ = 0; }

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

inline scag::util::storage::Serializer& operator << ( scag::util::storage::Serializer& o, const scag2::sessions::Session& s )
{
    return s.serialize(o);
}
inline scag::util::storage::Deserializer& operator >> ( scag::util::storage::Deserializer& o, scag2::sessions::Session& s )
{
    return s.deserialize(o);
}

#endif /* !_SCAG_SESSIONS_SESSION2_H */
