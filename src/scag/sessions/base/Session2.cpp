#include <set>
#include <cassert>
#include <cstdlib>      // for abort
#include <sys/time.h>

#include "util/cstrings.h"
#include "util/Exception.hpp"
#include "scag/exc/SCAGExceptions.h"
#include "core/synchronization/Mutex.hpp"
#include "ExternalTransaction.h"
#include "Operation.h"
#include "Session2.h"
#include "SessionStore2.h"
#include "scag/transport/CommandOperation.h"

namespace {

smsc::logger::Logger*              log_ = 0;
smsc::core::synchronization::Mutex sessionloggermutex;
smsc::core::synchronization::Mutex sessionopidmutex;

inline void getlog() {
    if ( !log_ ) {
        MutexGuard mg(sessionloggermutex);
        if ( !log_ ) log_ = smsc::logger::Logger::getInstance("session");
    }
}


using namespace scag2::sessions;
using namespace scag2::exceptions;

Hash<int> InitReadOnlyPropertiesHash()
{
    Hash<int> hs;
    // hs["USR"]               = PROPERTY_USR;

    // operation scope
    hs["%ICC_STATUS"]        = 1; // PROPERTY_ICC_STATUS;
    hs["%operation_id"]      = 1;

    // global scope
    hs["$abonent"]           = 1; // PROPERTY_ABONENT;
    return hs;
}
Hash<int> ReadOnlyPropertiesHash = InitReadOnlyPropertiesHash();


std::set< std::string > initGlobalScopeSet()
{
    std::set< std::string > res;
    res.insert( "abonent" );
    return res;
}

std::set< std::string > initOperationScopeSet()
{
    std::set< std::string > res;
    res.insert( "ICC_STATUS" );
    res.insert( "operation_id" );
    return res;
}

std::set< std::string > readonlyGlobalScopeSet = initGlobalScopeSet();
std::set< std::string > readonlyOperationScopeSet = initOperationScopeSet();


class SessionPropertyScopeWrapper : public SessionPropertyScope
{
public:
    SessionPropertyScopeWrapper( Session* patron, std::set< std::string >& rd ) :
    SessionPropertyScope(patron), readonly_(&rd) {}

    virtual Property* getProperty( const std::string& name )
    {
        Property* p;
        if ( isReadonly(name) ) {
            p = processReadonly( name );
        } else {
            p = SessionPropertyScope::getProperty( name );
        }
        return p;
    }

protected:
    virtual bool isReadonly( const std::string& name ) const {
        return ( readonly_->find( name ) == readonly_->end() ? false : true );
    }

    // only a few readonly items are expected
    Property* processReadonly( const std::string& name )
    {
        AdapterProperty** p = properties_.GetPtr( name.c_str() );
        AdapterProperty* np = 0;
        if ( name == "ICC_STATUS" ) {

            Operation* op = session_->getCurrentOperation();
            const int opstat = op ? op->getStatus() : 0;
            if ( !p ) {
                np = new AdapterProperty( name, session_, opstat );
            } else {
                (*p)->setInt( opstat );
            }

        } else if ( name == "abonent" ) {

            if ( !p )
                np = new AdapterProperty( name, session_, 
                                          session_->sessionKey().toString() );

        } else if ( name == "operation_id" ) {
            
            if ( !p ) {
                np = new AdapterProperty( name, session_, session_->getCurrentOperationId() );
            } else {
                (*p)->setInt( session_->getCurrentOperationId() );
            }

        } else {

            throw SCAGException( "logic error in SessionPropertyScope:processReadonly, unregistered name='%s' is requested", 
                                 name.c_str() );
        }
        if ( np ) {
            properties_.Insert( name.c_str(), np );
        } else {
            assert(p);
            np = *p;
        }
        smsc_log_debug( log_, "readonly property '%s' has value '%s'",
                        name.c_str(), np->getStr().c_str() );
        return np;
    }

private:
    // we don't need hash here as only a few (<5) readonly values are expected.
    std::set< std::string >* readonly_;
};

} // namespace


namespace scag2 {
namespace sessions {

using namespace scag::exceptions;


// statics
opid_type              Session::newopid_ = SCAGCommand::invalidOpId();


SessionPropertyScope::SessionPropertyScope( Session* patron ) :
session_(patron) {
    ::getlog();
}

SessionPropertyScope::~SessionPropertyScope()
{
    clear();
}


Property* SessionPropertyScope::getProperty( const std::string& name )
{
    AdapterProperty* res;
    AdapterProperty** ptr = properties_.GetPtr( name.c_str() );
    if ( ptr ) {
        res = *ptr;
    } else {
        res = new AdapterProperty( name, session_, "" );
        properties_.Insert( name.c_str(), res );
    }
    return res;
}


Serializer& SessionPropertyScope::serialize( Serializer& o ) const
{
    uint32_t sz = size();
    o << sz;
    if ( sz > 0 ) {
        char* key;
        AdapterProperty* value;
        for ( Hash< AdapterProperty* >::Iterator i(&properties_); i.Next(key,value); --sz ) {
            if ( isReadonly(key) ) continue;
            o << key << value->getStr();
        }
        assert( sz == 0 );
    }
    return o;
}


Deserializer& SessionPropertyScope::deserialize( Deserializer& o ) throw (DeserializerException)
{
    uint32_t sz;
    o >> sz;
    clear();
    std::string key;
    std::string value;
    for ( ; sz > 0; --sz ) {
        o >> key >> value;
        if ( key.size() > 0 ) {
            AdapterProperty* p = new AdapterProperty( key, session_, value );
            properties_.Insert( smsc::util::cStringCopy(key.c_str()), p );
        }
    }
    return o;
}


void SessionPropertyScope::clear()
{
    char* key;
    AdapterProperty* value;
    for ( Hash< AdapterProperty* >::Iterator i( &properties_ ); i.Next(key,value); ) {
        delete value;
    }
    properties_.Empty();
}


// ======================================


struct Session::TransportNewFlag
{
    TransportNewFlag() {
        memset( old, 0, sizeof(old) );
    }
    bool isNew( int trans ) const {
        if ( unsigned(--trans) >= maxtrans ) {
            throw smsc::util::Exception( "too big transport=%d specified in Session::isNew", ++trans );
        }
        return ! old[trans];
    }
    void setNew( int trans, bool an ) {
        if ( unsigned(--trans) >= maxtrans ) {
            throw smsc::util::Exception( "too big transport=%d specified in Session::setNew", ++trans );
        }
        old[trans] = !an;
    }

private:
    static const unsigned maxtrans = 3;
    bool old[maxtrans];

};


bool Session::isReadOnlyProperty( const char* name )
{
    return ::ReadOnlyPropertiesHash.GetPtr(name);
}


Session::Session( const SessionKey& key ) :
key_(key),
expirationPolicy_(COMMON),
pkey_(key),
command_(0),
transactions_(0),
nextContextId_(0),
globalScope_(0),
serviceScopes_(0),
contextScopes_(0),
operationScopes_(0)
{
    getlog();
    clear();
}


    Session::~Session()
    {
        clear();
        // FIXME: delete things
        if ( cmdQueue_.size() > 0 ) {
            smsc_log_error( log_, "!!!Session command queue is not empty: %d", cmdQueue_.size() );
            this->abort();
        }
        /*
        for ( std::list< scag::transport::SCAGCommand* >::iterator i = cmdQueue_.begin();
              i != cmdQueue_.end();
              ++i ) {
            delete *i;
        }
        cmdQueue_.erase( cmdQueue_.begin(), cmdQueue_.end() );
         */
        delete transactions_;
        delete globalScope_;
        delete serviceScopes_;
        delete contextScopes_;
        delete operationScopes_;
    }


    Serializer& Session::serialize( Serializer& s ) const
    {
        key_.serialize( s );
        // s << bornTime_.tv_sec << bornTime_.tv_usec;
        // FIXME: cmdQueue?
        s << currentOperationId_;
        serializeScope( s, globalScope_ );
        serializeScopeHash( s, serviceScopes_ );
        serializeScopeHash( s, contextScopes_ );
        serializeScopeHash( s, operationScopes_ );
        return s;
    }


    Deserializer& Session::deserialize( Deserializer& s ) throw (DeserializerException)
    {
        key_.deserialize( s );
        // s >> bornTime_.tv_sec >> bornTime_.tv_usec;
        // FIXME: cmdQueue?
        opid_type opid;
        s >> opid;
        // FIXME: operations hash
        deserializeScope( s, globalScope_ );
        deserializeScopeHash( s, serviceScopes_ );
        deserializeScopeHash( s, contextScopes_ );
        deserializeScopeHash( s, operationScopes_ );
        // post-processing
        setCurrentOperation( opid );
        return s;
    }


    void Session::clear()
    {
        expirationPolicy_ = COMMON;
        // gettimeofday( &bornTime_, 0 );
        pkey_ = SessionPrimaryKey( key_ ); // to reset born time
        lastAccessTime_ = time(0);
        expirationTime_ = lastAccessTime_ + 5; // FIXME: customize

        delete command_; command_ = 0;
        
        isnew_.Empty();
        if ( ! initrulekeys_.empty() ) {
            smsc_log_warn( log_, "rule keys stack is not empty, finalization failed?" );
        }
        initrulekeys_ = std::stack< std::pair<int,int> >();

        { // clear operations

            currentOperationId_ = 0;
            ussdOperationId_ = 0;
            currentOperation_ = 0;
            umr_ = -1;

            assert( operations_.Count() == 0 );
            int opkey;
            Operation* op;
            for ( IntHash< Operation* >::Iterator i(operations_); i.Next(opkey,op); ) {
                delete op;
            }
            operations_.Empty();

        }

        if ( transactions_ ) {
            char*          key;
            ExternalTransaction* value;
            for ( Hash< ExternalTransaction* >::Iterator i(transactions_);
                  i.Next(key,value); ) {
                if ( !value ) continue;
                value->rollback();
                delete value;
            }
            transactions_->Empty();
        }

        if ( globalScope_ ) globalScope_->clear();
        clearScopeHash( operationScopes_ );
        clearScopeHash( serviceScopes_ );
        clearScopeHash( contextScopes_ );
    }


void Session::changed( AdapterProperty& )
{
    // FIXME
}


/// transaction methods
ExternalTransaction* Session::getTransaction( const char* id )
{
    if ( !id || !transactions_ ) return 0;
    ExternalTransaction** ptr = transactions_->GetPtr( id );
    return ( ptr ? *ptr : 0 );
}


bool Session::addTransaction( const char* id, std::auto_ptr<ExternalTransaction> tr )
{
    if ( ! tr.get() ) return false;
    do {
        if ( !id ) break;
        if ( ! transactions_ ) transactions_ = new Hash< ExternalTransaction* >;
        ExternalTransaction** ptr = transactions_->GetPtr( id );
        if ( ptr ) {
            if ( *ptr ) break;
            *ptr = tr.release();
        } else {
            transactions_->Insert( id, tr.release() );
        }
        return true;
    } while ( false );
    tr->rollback();
    return false;
}


std::auto_ptr< ExternalTransaction > Session::releaseTransaction( const char* id )
{
    std::auto_ptr< ExternalTransaction > ret;
    if ( id && transactions_ ) {
        ExternalTransaction** ptr = transactions_->GetPtr(id);
        if ( ptr ) {
            ret.reset( *ptr );
            *ptr = 0;
        }
    }
    return ret;
}


/// operation methods
Operation* Session::setCurrentOperation( opid_type opid )
{
    do {

        if ( opid == SCAGCommand::invalidOpId() ) {
            currentOperation_ = 0;
            break;
        }

        Operation** optr = operations_.GetPtr(opid);
        if ( ! optr ) {
            currentOperation_ = 0;
            break;
        }

        currentOperation_ = *optr;

    } while ( false );

    if ( ! currentOperation_ ) {
        currentOperationId_ = SCAGCommand::invalidOpId();
        smsc_log_warn( log_, "Session=%p cannot find operation id=%u, key=%s",
                       this, unsigned(opid), sessionKey().toString().c_str() );
        // FIXME: should we throw?
        // throw SCAGException( "Session=%p cannot find operation id=%u, key=%s",
        // this, unsigned(opid), sessionKey().toString().c_str() );
        return 0;
    }

    const uint8_t optype = currentOperation_->type();
    currentOperationId_ = opid;
    smsc_log_debug( log_, "Session=%p set current op=%p, opid=%u, type=%d(%s)",
                    this, currentOperation_, unsigned(opid), int(optype),
                    commandOpName(optype) );
    // changed_ = true;
    return currentOperation_;
}


Operation* Session::createOperation( SCAGCommand& cmd, int operationType )
{
    std::auto_ptr< Operation > auop( new Operation(this, operationType) );
    Operation* op = auop.get();
    opid_type opid = getNewOperationId();
    if ( operationType == transport::CO_USSD_DIALOG ) {
        // if the operation is already there, replace it
        if ( ussdOperationId_ != SCAGCommand::invalidOpId() ) {
            smsc_log_debug( log_, "Session: old USSD operation exists, going to delete" );
            setCurrentOperation( ussdOperationId_ );
            closeCurrentOperation();
        }
        ussdOperationId_ = opid;
    }
    cmd.setOperationId( opid );
    operations_.Insert( opid, auop.release() );
    currentOperationId_ = opid;
    currentOperation_ = op;
    // changed_ = true;
    smsc_log_debug( log_, "Session=%p key=%s create op=%p opid=%u type=%d(%s)",
                    this, sessionKey().toString().c_str(),
                    currentOperation_,
                    unsigned(currentOperationId_),
                    operationType,
                    commandOpName(operationType) );
    return currentOperation_;
}


void Session::closeCurrentOperation()
{
    if ( ! currentOperation_ ) return;
    smsc_log_debug( log_, "Session=%p key=%s close op=%p opid=%u type=%d(%s)",
                    this, sessionKey().toString().c_str(),
                    currentOperation_,
                    unsigned(currentOperationId_), currentOperation_->type(),
                    commandOpName(currentOperation_->type()) );

    operations_.Delete( currentOperationId_ );
    delete currentOperation_;
    currentOperation_ = 0;
    if ( ussdOperationId_ == currentOperationId_ ) {
        ussdOperationId_ = SCAGCommand::invalidOpId();
        umr_ = -1;
    }
    currentOperationId_ = SCAGCommand::invalidOpId();
    // changed_ = true;
}


void Session::setUSSDref( int32_t ref ) throw (SCAGException)
{
    if ( ref <= 0 ) throw SCAGException( "setUSSDref(ref=%d), ref should be >0", ref );
    if ( umr_ > 0 ) throw SCAGException( "setUSSDref(ref=%d), UMR=%d is already set", ref, umr_ );
    if ( umr_ == -1 ) throw SCAGException( "setUSSDref(ref=%d), no USSD operation found", ref );
    umr_ = ref;
}


bool Session::isNew( int serv, int trans ) const {
    TransportNewFlag* f = isnew_.GetPtr( serv );
    if ( ! f ) return true;
    return f->isNew( trans );
}


void Session::setNew( int serv, int trans, bool an ) {
    TransportNewFlag* f = isnew_.GetPtr( serv );
    if ( !f ) {
        TransportNewFlag ff;
        ff.setNew( trans, an );
        isnew_.Insert(serv,ff);
    } else {
        f->setNew( trans, an );
    }
}


void Session::pushInitRuleKey( int serv, int trans )
{
    std::pair<int,int> st(serv,trans);
    if ( ! initrulekeys_.empty() && st == initrulekeys_.top() ) return;
    initrulekeys_.push( st );
}

bool Session::getRuleKey( int& serv, int& trans ) const
{
    if ( initrulekeys_.empty() ) return false;
    const std::pair<int,int>& st = initrulekeys_.top();
    serv = st.first;
    trans = st.second;
    return true;
}


void Session::popInitRuleKey()
{
    if ( ! initrulekeys_.empty() ) initrulekeys_.pop();
}


/// create context Scope.
int Session::createContextScope()
{
    if ( ! contextScopes_ ) contextScopes_ = new IntHash< SessionPropertyScope* >;
    contextScopes_->Insert( ++nextContextId_, new SessionPropertyScope(this) );
    return nextContextId_;
}


/// delete context Scope.
bool Session::deleteContextScope( int ctxid )
{
    if ( ! contextScopes_ ) return false;
    SessionPropertyScope** sptr = contextScopes_->GetPtr( ctxid );
    if ( sptr ) {
        delete *sptr;
        contextScopes_->Delete( ctxid );
        return true;
    }
    return false;
}


/// @return session global Scope
SessionPropertyScope* Session::getGlobalScope()
{
    if ( ! globalScope_ ) {
        SessionPropertyScopeWrapper* w = 
            new SessionPropertyScopeWrapper( this, readonlyGlobalScopeSet );
        globalScope_ = w;
    }
    return globalScope_;
}


/// @return service Scope by id (created on-demand)
SessionPropertyScope* Session::getServiceScope( int servid )
{
    SessionPropertyScope* res;
    if ( ! serviceScopes_ ) serviceScopes_ = new IntHash< SessionPropertyScope* >;
    SessionPropertyScope** sptr = serviceScopes_->GetPtr( servid );
    if ( ! sptr ) {
        res = new SessionPropertyScope( this );
        serviceScopes_->Insert( servid, res );
    } else {
        res = *sptr;
    }
    return res;
}


/// @return context Scope by id (may return NULL)
SessionPropertyScope* Session::getContextScope( int ctxid )
{
    SessionPropertyScope* res = 0;
    if ( contextScopes_ ) {
        SessionPropertyScope** sptr = contextScopes_->GetPtr( ctxid );
        if ( sptr ) res = *sptr;
    }
    return res;
}

    
/// @return current operation Scope (created on-demand)
SessionPropertyScope* Session::getOperationScope()
{
    SessionPropertyScope* res;
    if ( getCurrentOperation() == 0 )
        throw SCAGException( "session=%p cannot get operation scope, op=0", this );

    if ( ! operationScopes_ )
        operationScopes_ = new IntHash< SessionPropertyScope* >;
    SessionPropertyScope** sptr = operationScopes_->GetPtr( getCurrentOperationId() );
    if ( ! sptr ) {
        SessionPropertyScopeWrapper* w = 
            new SessionPropertyScopeWrapper( this, readonlyOperationScopeSet );
        res = w;
        operationScopes_->Insert( getCurrentOperationId(), res );
    } else {
        res = *sptr;
    }
    return res;
}


time_t Session::expirationTime() const 
{
    return expirationTime_;
}


    unsigned Session::appendCommand( SCAGCommand* cmd )
    {
        if (cmd) cmdQueue_.push_back( cmd );
        return cmdQueue_.size();
    }


    SCAGCommand* Session::popCommand()
    {
        SCAGCommand* cmd = 0; 
        if ( ! cmdQueue_.empty() ) {
            cmd = cmdQueue_.front();
            cmdQueue_.pop_front();
        }
        return cmd;
    }


void Session::abort()
{
    // FIXME
    ::abort();
}


    void Session::serializeScope( Serializer& o, const SessionPropertyScope* s ) const
    {
        const uint32_t sz = s ? s->size() : 0;
        if (!sz) {
            o << sz;
            return;
        }
        s->serialize( o );
    }


    void Session::deserializeScope( Deserializer& o, SessionPropertyScope*& s ) throw (DeserializerException)
    {
        const size_t rpos = o.rpos();
        uint32_t sz;
        o >> sz;
        if ( s ) s->clear();
        if ( !sz ) return;
        o.setrpos( rpos );
        if ( !s ) s = new SessionPropertyScope( this );
        s->deserialize( o );
    }


    void Session::serializeScopeHash( Serializer& o, const IntHash< SessionPropertyScope* >* s ) const
    {
        uint32_t sz = s ? s->Count() : 0;
        o << sz;
        if ( !sz ) return;
        int key;
        SessionPropertyScope* value;
        for ( IntHash< SessionPropertyScope* >::Iterator i(*s); i.Next(key,value); --sz ) {
            o << uint32_t(key);
            serializeScope(o,value);
        }
        assert( sz == 0 );
    }


    void Session::deserializeScopeHash( Deserializer& o, IntHash< SessionPropertyScope* >*& s ) throw (DeserializerException)
    {
        uint32_t sz;
        o >> sz;
        clearScopeHash( s );
        if ( !sz ) return;
        if ( !s ) s = new IntHash< SessionPropertyScope* >;
        for ( ; sz > 0; --sz ) {
            uint32_t key;
            SessionPropertyScope* value;
            o >> key;
            deserializeScope(o,value);
            if ( value ) s->Insert( int(key), value );
        }
    }



void Session::clearScopeHash( IntHash< SessionPropertyScope* >* s )
{
    if ( !s ) return;
    int key;
    SessionPropertyScope* value;
    for ( IntHash< SessionPropertyScope* >::Iterator i(*s);
          i.Next(key,value); ) {
        delete value;
    }
    s->Empty();
}


opid_type Session::getNewOperationId() const
{
    MutexGuard mg(::sessionopidmutex);
    if ( ++newopid_ == SCAGCommand::invalidOpId() ) ++newopid_;
    return newopid_;
}


    // ======================================


    ActiveSession::ActiveSession( SessionStore& st, Session& s ) :
    store_(&st), s_(&s), flush_(false) {}


    void ActiveSession::release() 
    {
        if ( s_ ) store_->releaseSession( *s_, flush_ );
        s_ = 0;
    }

    void ActiveSession::moveLock( SCAGCommand* cmd )
    {
        if ( s_ ) store_->moveLock( *s_, cmd );
    }

} // namespace sessions
} // namespace scag
