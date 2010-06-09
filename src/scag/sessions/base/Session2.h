#ifndef _SCAG_SESSIONS_SESSION2_H
#define _SCAG_SESSIONS_SESSION2_H

#include <list>

#include "SessionKey.h"
#include "core/buffers/Hash.hpp"
#include "core/buffers/IntHash.hpp"
#include "logger/Logger.h"
#include "scag/util/properties/Properties2.h"
#include "scag/util/io/Serializer.h"
#include "scag/util/io/Print.h"
#include "scag/transport/SCAGCommand2.h"
#include "scag/exc/SCAGExceptions.h"
#include "scag/re/base/LongCallContext.h"

namespace scag2 {
namespace sessions {

using namespace smsc::logger;
using namespace util::properties;
using namespace smsc::core::buffers;
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
    SessionPropertyScope( Session* patron );
    virtual ~SessionPropertyScope();
    virtual Property* getProperty( const std::string& name );
    virtual void delProperty( const std::string& name );

    util::io::Serializer& serialize( util::io::Serializer& s ) const;
    util::io::Deserializer& deserialize( util::io::Deserializer& s ) 
        /* throw (util::io::DeserializerException) */; 

    // clear the scope
    void clear();
    unsigned size() const { return properties_.GetCount(); }

protected:
    /// method is necessary for serialization
    virtual bool isReadonly( const std::string& ) const { return false; }

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
public:
    /// return the default live time of the session
    static unsigned defaultLiveTime();
    static unsigned ussdLiveTime();
    static void setDefaultLiveTime( unsigned tmo );
    static void setUssdLiveTime( unsigned tmo );

    /// NOTE: the full field name (with prefix) is required
    static bool isReadOnlyProperty( const char* fullname );

public:
    Session( const SessionKey& key, bool quiet = false );

    virtual ~Session();

    util::io::Serializer& serialize( util::io::Serializer& s ) const;
    util::io::Deserializer& deserialize( util::io::Deserializer& s )
        /* throw (util::io::DeserializerException) */;

    /// notification when property is changed
    virtual void changed( AdapterProperty& prop );

    inline const SessionKey& sessionKey() const {
        return key_;
    }

    inline const SessionPrimaryKey& sessionPrimaryKey() const {
        return pkey_;
    }

    /// debugging printout.
    /// NOTE: command should not be printed, as it may be already deleted!
    void print( util::Print& p ) const;

    // --- check if session needs flushing to disk.
    // NOTE: this flag may be set from closeCurrentOperation()
    // when the operation is persistent.
    bool needsFlush() const { return needsflush_; }
    // void setPersistent( bool p = true ) { persistent_ = p; }

    /// === transaction methods
    /// NOTE: all non-released transactions will be rollbacked at destructor!
    /// So, if you want to commit a transaction, the proper way to do it is:
    /// releaseTransaction( id )->commit();
    
    /// get transaction by id
    ExternalTransaction* getTransaction( const char* id );

    /// add transaction with id (taking ownership).
    /// return false, if there is already transaction with given id.
    bool addTransaction( const char* id, std::auto_ptr<ExternalTransaction> tr );

    /// remove transaction from session.
    /// NOTE: it is your responsibility to rollback/commit + delete it.
    std::auto_ptr<ExternalTransaction> releaseTransaction( const char* id );


    /// === operations methods
    /// NOTE: invalidOpId() denotes invalid operation
    inline opid_type getCurrentOperationId() const {
        return currentOperationId_;
    }
    inline Operation* getCurrentOperation() const { 
        return currentOperation_;
    }

    /// set an existing operation as current by id and return it.
    Operation* setCurrentOperation( opid_type opid, bool updateExpire = false );

    /// return ussd operation id or invalidOpId()
    opid_type getUSSDOperationId() const {
        return ussdOperationId_;
    }

    /// create a new operation and set it as current
    Operation* createOperation( SCAGCommand& cmd, int operationType );

    /*
    class operation_iterator
    {
    public:
        operation_iterator( Session& s ) :
        id_(int(SCAGCommand::invalidOpId())),
        op_(0),
        i_(s.operations_) {}

        bool next() {
            return i_.Next(id_,op_);
        }

        inline opid_type opid() const { return id_; }
        inline Operation* operation() const { return op_; }

    private:
        int        id_;
        Operation* op_;
        IntHash< Operation* >::Iterator i_;
    };
     */

    /// destroy current operation
    void closeCurrentOperation();
    
    /// check if the session has at least one persistent operation
    bool hasPersistentOperation() const;

    /// number of operations
    inline unsigned operationsCount() const {
        return operations_.Count();
    }

    // this field is not in ussd operation because operation is created later (in RE)
    // int32_t getUSSDref() const {
    // return umr_;
    // }
    // NOTE: this method works only if getUSSDref() == 0.
    // void setUSSDref( int32_t ref ) throw (exceptions::SCAGException);


    /// FIXME: should it be here?
    lcm::LongCallContext& getLongCallContext() {
        return lcmCtx_;
    }

    /// check if the session is new for given service/transport
    bool isNew( int serviceId, int transport ) const;
    void setNew( int serviceId, int transport, bool anew );

    /// push a rule key onto the rulekey stack
    void pushInitRuleKey( int serviceId, int transport );
    bool getRuleKey( int& serviceId, int& transport ) const;

    /// drop the given rule and reset expiration time to wait if possible.
    /// expiration time cannot be lowered if there are living services.
    void dropInitRuleKey( int serviceId, int transport, int wait = 0 );

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
    /// NOTE: this time gives the soft expiration limit.
    inline time_t expirationTime() const {
        return expirationTime_;
    }

    /// make sure the session is alive at least s seconds, i.e. set expirationTime
    /// to now()+s.  I.e. this method sets the hard expiration limit.
    /// NOTE: if expirationTime is already set to a later time, then do nothing.
    void waitAtLeast( unsigned s );


    // === ATTENTION! the following methods should be invoked from session store only

    /// clear internal state except session key, but not the command queue.
    /// reset expiration time.
    void clear();

    // get/set the last access time
    inline time_t lastAccessTime() const {
        return lastAccessTime_;
    }

    inline void setLastAccessTime( time_t t ) {
        lastAccessTime_ = t;
    }

    /// current processing command (owned).
    /// the presence of a command tells that session is locked.
    /// \return the previous owned command.
    /// It gives the possibility to destroy it after unlocking.
    inline uint32_t setCurrentCommand( uint32_t cmd ) {
        uint32_t prev = command_;
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

    /// the id of the command with currently locks the session
    /// NOTE: ids are (see also SCAGCommand):
    /// 0 -- unlocked
    /// 1 -- locked for finalization
    /// 2 -- locked for disk i/o
    /// 3-9 -- reserved
    inline uint32_t currentCommand() {
        return command_;
    }

    /// append command to the list of session commands.
    /// @return the size of command queue.
    /// NOTE: this method should be invoked from SessionStore only (as it requires locking).
    unsigned appendCommand( SCAGCommand* cmd );

    /// pop one command from the list of session commands.
    /// NOTE: this method should be invoked from SessionStore only (as it requires locking).
    SCAGCommand* popCommand();

    /// get the count of the command queue
    inline unsigned commandCount() {
        return unsigned(cmdQueue_.size());
    }

    // NOTE: aborting session is forbidden
    // abort the session
    // void abort();

private: // methods
    Session();
    Session( const Session& s );
    Session& operator = ( const Session& s );

    typedef SessionPropertyScope* (*Opmaker)( Session* );

    void serializeScope( util::io::Serializer& o, const SessionPropertyScope* s ) const;
    void deserializeScope( util::io::Deserializer& o, SessionPropertyScope*& s, Opmaker opmaker )
        /* throw (util::io::DeserializerException) */;
    void serializeScopeHash( util::io::Serializer& o, const IntHash< SessionPropertyScope* >* s ) const;
    void deserializeScopeHash( util::io::Deserializer& o,
                               IntHash< SessionPropertyScope* >*& s,
                               Opmaker opmaker )
        /* throw (util::io::DeserializerException) */;
    void clearScopeHash( IntHash< SessionPropertyScope* >* s );
    opid_type getNewOperationId() const;

private: // statics

    // static Logger*   log_;
    static opid_type newopid_;

private:
    /// session key (msisdn)
    SessionKey     key_;                          // (pers)

    // session create time is incorporated into primarykey
    // timeval bornTime_;
    SessionPrimaryKey  pkey_;                     // (pers: borntime only)

    // last access time (should not be changed by users)
    time_t lastAccessTime_;                       // (pers)

    /// expiration Time ( soft/hard limits)
    time_t expirationTime_;                       // (pers)
    time_t expirationTimeAtLeast_;                // (pers)

    /// the flag tells if the session should be flushed
    /// NOTE: this flag is reset after flush.
    bool needsflush_;                             // (not pers: false)

    /// the serial number of the current command being processed,
    /// the session is locked if not 0.
    uint32_t command_;                            // (not pers: don't touch)

    /// FIXME: should it be here?
    lcm::LongCallContext  lcmCtx_;                 // (not pers: continueExec=0)

    /// === fields for init/destroy services
    struct TransportNewFlag;
    IntHash< TransportNewFlag >      isnew_;         // pers
    std::list< std::pair<int,int> >  initrulekeys_;  // pers

    /// the list of pending commands (owned, not pers).
    std::list< SCAGCommand* > cmdQueue_;             // not pers: don't touch

    /// === operations
    opid_type   currentOperationId_;                 // pers
    opid_type   ussdOperationId_;                    // pers
    Operation*  currentOperation_;                   // not pers: reset on load via setCurOp

    /// the hash of operations (int -> Operation)
    IntHash< Operation* > operations_;               // pers

    /// the hash of external transactions (string -> transaction)
    Hash< ExternalTransaction* >* transactions_;     // pers

    /// --- property Scopes

    /// 0 -- not set
    int16_t nextContextId_;                          // pers

    /// global var Scope
    SessionPropertyScope* globalScope_;              // pers

    /// the hash of service var Scopes (int(service_id) -> SessionServiceScope)
    IntHash< SessionPropertyScope* >* serviceScopes_;
    
    /// the hash of context var Scopes (int(context_id) -> SessionContextScope)
    IntHash< SessionPropertyScope* >* contextScopes_;

    /// the hash of operation var Scopes (int(op_id) -> SessionOperationScope)
    IntHash< SessionPropertyScope* >* operationScopes_;

    bool quiet_;                                      // not pers
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

    // get/set flush state of session
    // void setFlush( bool f ) { flush_ = f; }
    // bool getFlush() const { return flush_; }

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
        // flush_ = as.flush_;
        as.s_ = 0;
    }
private:
    SessionStore*    store_;
    mutable Session* s_;
    // bool             flush_;
};

} // namespace sessions
} // namespace scag

inline scag2::util::io::Serializer& operator << ( scag2::util::io::Serializer& o, const scag2::sessions::Session& s )
{
    return s.serialize(o);
}
inline scag2::util::io::Deserializer& operator >> ( scag2::util::io::Deserializer& o, scag2::sessions::Session& s )
{
    return s.deserialize(o);
}

#endif /* !_SCAG_SESSIONS_SESSION2_H */
