#ifndef _SCAG_SESSIONS_SESSION2_H
#define _SCAG_SESSIONS_SESSION2_H

#include <list>
#include <stack>

// #include "util/stringhash.hpp"
#include "SessionKey.h"
#include "core/buffers/Hash.hpp"
#include "core/buffers/IntHash.hpp"
#include "logger/Logger.h"
#include "scag/util/properties/Properties2.h"
#include "scag/util/storage/Serializer.h"
#include "scag/transport/SCAGCommand2.h"
#include "scag/re/base/LongCallContext.h"
#include "scag/exc/SCAGExceptions.h"


namespace scag2 {
namespace sessions {

using lcm::LongCallContext;
using namespace smsc::logger;
using namespace scag::util::properties;
using namespace smsc::core::buffers;
using namespace scag::util::storage;
using namespace transport;

typedef  transport::opid_type  opid_type;

enum ProtocolForEvent
{
    PROTOCOL_SMPP_SMS =  1,
    PROTOCOL_SMPP_USSD = 2,
    PROTOCOL_HTTP = 3,
    PROTOCOL_MMS = 4
};


class Session;


/// Scope of properties (e.g. global, service, context, operation)
class SessionPropertyScope
{
public:
    SessionPropertyScope( Session* patron ) : session_(patron) {}
    virtual ~SessionPropertyScope();
    virtual Property* getProperty( const std::string& name );
    Serializer& serialize( Serializer& s ) const;
    Deserializer& deserialize( Deserializer& s ) throw (DeserializerException);

    // clear the scope
    void clear();
    unsigned size() const { return properties_.GetCount(); }

protected:
    virtual bool isReadonly( const char* ) const { return false; }

protected:
    Session*                         session_;
    mutable Hash< AdapterProperty* > properties_; // owned
};


class ExternalTransaction;
class Operation;


///
/// a new realization of msag Session
///
class Session : public Changeable
{
private:
    // why we need this?
    /*
    enum ReadOnlyProperties
    {
        // PROPERTY_USR = 1,
        PROPERTY_ICC_STATUS = 2,
        PROPERTY_ABONENT = 3
    };
     */

public:
    /// NOTE: the full field name (with prefix) is required
    static bool isReadOnlyProperty( const char* fullname );

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

    /// === transaction methods
    /// NOTE: all non-released transactions will be rollbacked at destructor!
    /// So, if you want to commit a transaction, the proper way to do it is:
    /// releaseTransaction( id )->commit();
    
    /// get transaction by id
    ExternalTransaction* getTransaction( const char* id );

    /// add transaction with id (taking ownership).
    /// return false, if there is already transaction with given id.
    bool setTransaction( const char* id, std::auto_ptr<ExternalTransaction> tr );

    /// remove transaction from session.
    /// NOTE: it is your responsibility to rollback/commit + delete it.
    std::auto_ptr<ExternalTransaction> releaseTransaction( const char* id );


    /// === operations methods
    /// NOTE: opid_type(0) denotes invalid operation
    opid_type getCurrentOperationId() const {
        return currentOperationId_;
    }
    Operation* getCurrentOperation() const { 
        return currentOperation_;
    }

    /// set an existing operation by id and return it.
    Operation* setCurrentOperation( opid_type opid );

    /// return ussd operation id or -1
    opid_type getUSSDOperationId() const {
        return ussdOperationId_;
    }

    /// create a new operation and set it as current
    Operation* createOperation( SCAGCommand& cmd, int operationType );

    /// destroy current operation
    void closeCurrentOperation();
    
    /// number of operations
    unsigned operationsCount() const {
        return operations_.Count();
    }

    /// this field is not in ussd operation because operation is created later (in RE)
    int32_t getUSSDref() const {
        return umr_;
    }

    /// NOTE: this method works only if getUSSDref() == 0.
    void setUSSDref( int32_t ref ) throw (exceptions::SCAGException);


    /// FIXME: should it be here?
    LongCallContext& getLongCallContext() {
        return lcmCtx_;
    }

    /// check if the session is new for given service/transport
    bool isNew( int serviceId, int transport ) const;
    void setNew( int serviceId, int transport, bool anew );

    /// push a rule key onto the rulekey stack
    void pushInitRuleKey( int serviceId, int transport );
    bool getRuleKey( int& serviceId, int& transport ) const;
    void popInitRuleKey();


    /// --- property Scopes, for use from ActionContext

    /// create a new context scope and return its id.
    int createContextScope();

    /// delete context scope by id.
    bool deleteContextScope( int ctxid );

    /// @return session global Scope
    SessionPropertyScope* getGlobalScope();

    /// @return service Scope by id (created on-demand)
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


    /// abort the session
    void abort();

private: // methods
    Session();
    Session( const Session& s );
    Session& operator = ( const Session& s );

    void serializeScope( Serializer& o, const SessionPropertyScope* s ) const;
    void deserializeScope( Deserializer& o, SessionPropertyScope*& s ) throw (DeserializerException);
    void serializeScopeHash( Serializer& o, const IntHash< SessionPropertyScope* >* s ) const;
    void deserializeScopeHash( Deserializer& o, IntHash< SessionPropertyScope* >*& s ) throw (DeserializerException);
    void clearScopeHash( IntHash< SessionPropertyScope* >* s );
    opid_type getNewOperationId() const;

private: // statics

    static Logger*   log_;
    static opid_type newopid_;

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

    /// === fields for init/destroy handlers (not pers?)
    struct TransportNewFlag;
    IntHash< TransportNewFlag >      isnew_;
    std::stack< std::pair<int,int> > initrulekeys_;

    /// the list of pending commands (owned, not pers).
    std::list< SCAGCommand* > cmdQueue_;

    /// === operations
    opid_type   currentOperationId_;
    opid_type   ussdOperationId_;
    Operation*  currentOperation_;

    // TODO: think about movind operations creation/deletion into smppstatemachine,
    // then move umr_ field into operation.
    uint32_t    umr_;   // ussd reference number (-1 -- invalid, 0 -- pending)
    
    /// the hash of operations (int -> Operation)
    IntHash< Operation* > operations_;

    /// the hash of external transactions (string -> transaction)
    Hash< ExternalTransaction* >* transactions_;


    /// --- property Scopes

    int nextContextId_;

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
