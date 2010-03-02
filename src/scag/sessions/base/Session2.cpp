#include <set>
#include <cassert>
// #include <cstdlib>      // for abort
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

using namespace scag2::util::io;

namespace {

// uint32_t serializationVersion = 1;
const uint32_t serializationVersion = 2; // support for sar fields in operations

smsc::logger::Logger*              log_ = 0;
smsc::logger::Logger*              logc_ = 0; // for ctor/dtor
smsc::core::synchronization::Mutex sessionloggermutex;
smsc::core::synchronization::Mutex sessionopidmutex;

inline void getlog() {
    if ( !log_ ) {
        MutexGuard mg(sessionloggermutex);
        if ( !log_ ) {
            log_ = smsc::logger::Logger::getInstance("session");
            logc_ = smsc::logger::Logger::getInstance("sess.alloc");
        }
    }
}

/// session default live time (in seconds)
unsigned sessionLiveTime = 60;

using namespace scag2::sessions;
using namespace scag2::exceptions;

Hash<int> InitReadOnlyPropertiesHash()
{
    Hash<int> hs;
    // hs["USR"]               = PROPERTY_USR;

    // operation scope
    hs["%ICC_STATUS"]             = 1; // PROPERTY_ICC_STATUS;
    hs["%operation_id"]           = 1;
    hs["%slicing_parts_received"] = 1;
    hs["%slicing_resps_received"] = 1;

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
    res.insert( "slicing_parts_received" );
    res.insert( "slicing_resps_received" );
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
        if ( name == "ICC_STATUS" ||
             name == "slicing_parts_received" ||
             name == "slicing_resps_received" ) {

            Operation* op = session_->getCurrentOperation();
            int opval = 0;
            if ( op ) {
                if (name == "ICC_STATUS") opval = op->getStatus();
                else if ( name == "slicing_parts_received" ) opval = op->parts();
                else if ( name == "slicing_resps_received" ) opval = op->resps();
                else {
                    throw SCAGException( "logic error in SessionPropertyScope:processReadonly, unregistered name='%s' is requested",
                                         name.c_str() );
                }
            }

            if ( !p ) {
                np = new AdapterProperty( name.c_str(), session_, opval );
            } else {
                (*p)->setInt( opval );
            }

        } else if ( name == "abonent" ) {

            if ( !p )
                np = new AdapterProperty( name.c_str(), session_, 
                                          session_->sessionKey().toString() );

        } else if ( name == "operation_id" ) {
            
            if ( !p ) {
                np = new AdapterProperty( name.c_str(), session_, session_->getCurrentOperationId() );
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




SessionPropertyScope* makeGlobalScope( Session* s )
{
    return new SessionPropertyScopeWrapper( s, readonlyGlobalScopeSet );
}


SessionPropertyScope* makeServiceScope( Session* s )
{
    return new SessionPropertyScope( s );
}


SessionPropertyScope* makeContextScope( Session* s )
{
    return new SessionPropertyScope( s );
}


SessionPropertyScope* makeOperationScope( Session* s )
{
    return new SessionPropertyScopeWrapper( s, readonlyOperationScopeSet );
}

} // namespace


namespace scag2 {
namespace sessions {

using namespace scag::exceptions;


// statics
opid_type Session::newopid_ = invalidOpId();


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
        res = new AdapterProperty( name.c_str(), session_, "" );
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
            if ( isReadonly(key) || !strlen(key) ) {
                o << "";
            } else {
                o << key << value->getStr().c_str();
            }
        }
        assert( sz == 0 );
    }
    return o;
}


Deserializer& SessionPropertyScope::deserialize( Deserializer& o ) /* throw (DeserializerException) */
{
    uint32_t sz;
    o >> sz;
    clear();
    std::string key;
    std::string value;
    for ( ; sz > 0; --sz ) {
        o >> key;
        if ( key.size() > 0 ) {
            o >> value;
            AdapterProperty* p = new AdapterProperty( key.c_str(), session_, value );
            properties_.Insert( key.c_str(), p );
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


// --- statics

unsigned Session::defaultLiveTime()
{
    return sessionLiveTime;
}

void Session::setDefaultLiveTime( unsigned tmo )
{
    sessionLiveTime = tmo;
    ::getlog();
    smsc_log_info(log_,"setting default session live time %u", tmo);
}

bool Session::isReadOnlyProperty( const char* name )
{
    return ::ReadOnlyPropertiesHash.GetPtr(name);
}


// --- non-statics

Session::Session( const SessionKey& key, bool quiet ) :
key_(key),
pkey_(key),
lastAccessTime_(time(0)),
expirationTime_(lastAccessTime_+defaultLiveTime()),
expirationTimeAtLeast_(lastAccessTime_),
needsflush_(false),
command_(0),        // unlocked
currentOperationId_(invalidOpId()),
ussdOperationId_(invalidOpId()),
currentOperation_(0),
transactions_(0),
nextContextId_(0),
globalScope_(0),
serviceScopes_(0),
contextScopes_(0),
operationScopes_(0),
quiet_(quiet)
{
    ::getlog();
    if (!quiet_) smsc_log_debug( logc_, "session=%p/%s +1", this, key.toString().c_str() );
    // clear();
}


Session::~Session()
{
    try {
        clear();
        if ( cmdQueue_.size() > 0 ) {
            smsc_log_error( log_, "LOGIC ERROR!!! session=%p/%s command queue is not empty: %d, MEMLEAK!",
                            this, sessionKey().toString().c_str(), cmdQueue_.size() );
            // this->abort();
        }
        delete transactions_;
        delete globalScope_;
        delete serviceScopes_;
        delete contextScopes_;
        delete operationScopes_;
        if (!quiet_) smsc_log_debug( logc_, "session=%p/%s -1", this, sessionKey().toString().c_str() );
    } catch ( std::exception& e ) {
        smsc_log_error( log_, "Exception in session=%p/%s dtor: %s", this, sessionKey().toString().c_str(), e.what() );
    } catch (...) {
        smsc_log_error( log_, "Exception in session=%p/%s dtor: unknown", this, sessionKey().toString().c_str() );
    }
}


Serializer& Session::serialize( Serializer& s ) const
{
    const int32_t oldVersion = s.version();
    s.setVersion(::serializationVersion);
    s << ::serializationVersion;
    key_.serialize( s );
    const timeval& tv( pkey_.bornTime() );
    s << uint64_t(tv.tv_sec) << 
        uint32_t(tv.tv_usec) <<
        uint64_t(lastAccessTime_) <<
        uint64_t(expirationTime_) <<
        uint64_t(expirationTimeAtLeast_);

    assert( ! lcmCtx_.continueExec );

    // services init/destroy fields
    uint32_t count = uint32_t(initrulekeys_.size());
    s << count;
    for ( std::list<std::pair<int,int> >::const_iterator i = initrulekeys_.begin();
          i != initrulekeys_.end();
          ++i ) {
        --count;
        const uint32_t service( i->first );
        const uint32_t transport( i->second );
        s << service << transport << uint8_t(isNew(service,transport));
    }
    assert( count == 0 );

    s << currentOperationId_ << ussdOperationId_;
    
    count = operationsCount();
    s << count;
    if ( count ) {
        int key;
        Operation* value;
        for ( IntHash< Operation* >::Iterator i(operations_); i.Next(key,value); --count ) {
            if ( ! value ) key = int(invalidOpId());
            s << opid_type(key);
            if ( opid_type(key) != invalidOpId() )
                value->serialize( s );
        }
        assert( count == 0 );
    }

    count = transactions_ ? transactions_->GetCount() : 0;
    s << count;
    if ( count ) {
        char* key;
        ExternalTransaction* value;
        for ( Hash< ExternalTransaction* >::Iterator i(transactions_);
              i.Next(key,value);
              ) {
            if ( !value || !strlen(key) ) {
                s << "";
            } else {
                s << key;
                value->serialize( s );
            }
        }
    }

    s << uint32_t(nextContextId_);
    serializeScope( s, globalScope_ );
    serializeScopeHash( s, serviceScopes_ );
    serializeScopeHash( s, contextScopes_ );
    serializeScopeHash( s, operationScopes_ );

    s.setVersion(oldVersion);
    return s;
}


Deserializer& Session::deserialize( Deserializer& s ) /* throw (DeserializerException) */
{
    clear();
    const int32_t oldVersion = s.version();
    try {
        uint32_t count;
        {
            s >> count;
            const char* fail = 0;
            if ( count > ::serializationVersion ) {
                fail = "Unknown serialization version: %u";
            } else if ( count != ::serializationVersion ) {
                fail = "Older serialization version is not supported: %u";
            }
            if ( fail ) {
                smsc_log_error( log_, fail, count );
                throw SCAGException( fail, count );
            }
        }
        s.setVersion(count);

        key_.deserialize( s );
        {
            uint64_t tvsec,atime,etimes,etimeh;
            uint32_t tvusec;
            s >> tvsec >> tvusec >> atime >> etimes >> etimeh;
            timeval tv = { tvsec, tvusec };
            pkey_ = SessionPrimaryKey(key_);
            pkey_.setBornTime(tv);
            expirationTime_ = time_t(etimes);
            expirationTimeAtLeast_ = time_t(etimeh);
        }

        s >> count;
        isnew_.Empty();
        initrulekeys_.clear();
        for ( ; count > 0; --count ) {
            uint32_t service, transport;
            uint8_t isnew;
            s >> service >> transport >> isnew;
            initrulekeys_.push_back( std::make_pair(int(service),int(transport)) );
            setNew( int(service), int(transport), isnew );
        }

        s >> currentOperationId_ >> ussdOperationId_;

        s >> count;
        for ( ; count > 0; --count ) {
            opid_type key;
            s >> key;
            if ( key != invalidOpId() ) {
                Operation* op = new Operation( this, CO_NA );
                op->deserialize( s );
                operations_.Insert( key, op );
            }
        }

        s >> count;
        if ( count > 0 ) {
            transactions_ = new Hash< ExternalTransaction* >;
            for ( ; count > 0; --count ) {
                std::string key;
                s >> key;
                if ( ! key.empty() ) {
                    ExternalTransaction* et = 
                        ExternalTransaction::createAndDeserialize( s );
                    if ( !et ) {
                        smsc_log_error(log_,"cannot deserialize external transaction session=%s key=%s",
                                       key_.toString().c_str(), key.c_str() );
                        throw SCAGException( "cannot deserialize external transation session=%s key=%s",
                                             key_.toString().c_str(), key.c_str());
                    }
                    transactions_->Insert( smsc::util::cStringCopy(key.c_str()), et );
                }
            }
        }

        s >> count;
        nextContextId_ = int32_t(count);

        deserializeScope( s, globalScope_, ::makeGlobalScope );
        deserializeScopeHash( s, serviceScopes_, ::makeServiceScope );
        deserializeScopeHash( s, contextScopes_, ::makeContextScope );
        deserializeScopeHash( s, operationScopes_, ::makeOperationScope );

        // post-processing
        setCurrentOperation( currentOperationId_ );
    } catch (...) {
        clear();
        s.setVersion(oldVersion);
        throw;
    }
    s.setVersion(oldVersion);
    return s;
}


void Session::clear()
{
    // if (!quiet_) smsc_log_debug( logc_, "session=%p/%s clear", this, sessionKey().toString().c_str() );

    pkey_ = SessionPrimaryKey(key_); // to reset born time

    const time_t now = time(0);
    lastAccessTime_ = now;
    expirationTimeAtLeast_ = now;
    expirationTime_ = expirationTimeAtLeast_ + defaultLiveTime();

    needsflush_ = false;

    lcmCtx_.clear();
    lcmCtx_.setActionContext( 0 );

    isnew_.Empty();
    initrulekeys_.clear();

    { // clear operations

        currentOperationId_ = invalidOpId();
        ussdOperationId_ = invalidOpId();
        currentOperation_ = 0;

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
            try {
                value->rollback();
            } catch ( std::exception& e ) {
                smsc_log_debug(log_, "exception while rolling back: %s", e.what() );
            } catch (...) {
                smsc_log_debug(log_, "unknown exception while rolling back" );
            }
            delete value;
        }
        transactions_->Empty();
    }

    // nextcontextid is not reset to prevent accidents

    if ( globalScope_ ) globalScope_->clear();
    clearScopeHash( operationScopes_ );
    clearScopeHash( serviceScopes_ );
    clearScopeHash( contextScopes_ );
}


void Session::changed( AdapterProperty& )
{
    // FIXME: do we need reaction on property change?
}


void Session::print( util::Print& p ) const
{
    if ( ! p.enabled() ) return;
    // if ( ! command_ ) return;  // session is not locked !

    const time_t now = time(0);
    // const int lastac = int(now - lastAccessTime_);
    p.print( "session=%p/%s tmASH=%d/%d/%d lock=%u svc/ops/trs=%u/%u/%u%s",
             this, sessionKey().toString().c_str(),
             int(lastAccessTime_ - now),
             int(expirationTime_ - now),
             int(expirationTimeAtLeast_ - now),
             command_,
             unsigned(initrulekeys_.size()),
             unsigned(operationsCount()),
             unsigned(transactions_ ? transactions_->GetCount() : 0),
             ussdOperationId_ == invalidOpId() ? "" : " hasUssd" );
    Operation* curop = getCurrentOperation();
    if ( curop ) curop->print( p, getCurrentOperationId() );
    int opid;
    Operation* op;
    for ( IntHash< Operation* >::Iterator i( operations_ ); i.Next(opid,op); ) {
        if ( ! op ) continue;
        if ( op == curop ) continue;
        op->print( p, opid_type(opid) );
    }
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
            transactions_->Delete(id);
        }
    }
    return ret;
}


/// operation methods
Operation* Session::setCurrentOperation( opid_type opid, bool updateExpire )
{
    do {

        if ( opid == invalidOpId() ) {
            currentOperationId_ = opid;
            return currentOperation_ = 0;
        }

        Operation** optr = operations_.GetPtr(opid);
        if ( ! optr ) {
            currentOperation_ = 0;
            break;
        }

        currentOperation_ = *optr;

    } while ( false );

    if ( ! currentOperation_ ) {
        currentOperationId_ = invalidOpId();
        smsc_log_warn( log_, "session=%p/%s cannot find operation id=%u",
                       this,
                       sessionKey().toString().c_str(),
                       unsigned(opid) );
        return 0;
    }

    const uint8_t optype = currentOperation_->type();
    currentOperationId_ = opid;
    if ( updateExpire ) {
        const time_t now = time(0);
        const time_t expire = now + defaultLiveTime();
        if ( expire > expirationTime_ ) expirationTime_ = expire;
    }
    smsc_log_debug( log_, "session=%p/%s setOp(opid=%u) => op=%p type=%d(%s) etime=%d",
                    this, sessionKey().toString().c_str(),
                    unsigned(opid), currentOperation_,
                    int(optype),
                    commandOpName(optype),
                    int(expirationTime_ - time(0)) );
    // changed_ = true;
    return currentOperation_;
}


Operation* Session::createOperation( SCAGCommand& cmd, int operationType )
{
    std::auto_ptr< Operation > auop( new Operation(this, operationType) );
    Operation* op = auop.get();
    opid_type opid;
    do {
        opid = getNewOperationId();
    } while ( operations_.Exist(opid) );
    if ( operationType == transport::CO_USSD_DIALOG ) {
        // if the operation is already there, replace it
        if ( ussdOperationId_ != invalidOpId() ) {
            smsc_log_info( log_, "session=%p/%s old USSD operation will be replaced",
                           this, sessionKey().toString().c_str() );
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

    // increase session live time
    time_t now = time(0);
    time_t expire = now + defaultLiveTime();
    if ( expire > expirationTime_ ) expirationTime_ = expire;
    smsc_log_debug( log_, "session=%p/%s createOp(cmd=%p) => op=%p opid=%u type=%d(%s), etime=%d",
                    this, sessionKey().toString().c_str(),
                    &cmd,
                    currentOperation_,
                    unsigned(currentOperationId_),
                    operationType,
                    commandOpName(operationType),
                    int(expirationTime_-now)
                    );
    return currentOperation_;
}


void Session::closeCurrentOperation()
{
    if ( ! currentOperation_ ) return;
    
    const Operation* prevop = currentOperation_;
    const opid_type  prevopid = currentOperationId_;
    const uint8_t    prevoptype = currentOperation_->type();
    
    if ( prevop->flagSet( OperationFlags::PERSISTENT ) )
        needsflush_ = true;

    // if ( log_->isDebugEnabled() ) {
    // scag_plog_debug(pl,log_);
    // this->print(pl);
    // }


    if ( operationScopes_ ) {
        SessionPropertyScope** sptr = operationScopes_->GetPtr(prevopid);
        if ( sptr ) {
            delete *sptr;
            operationScopes_->Delete(prevopid);
        }
    }

    operations_.Delete( currentOperationId_ );
    delete currentOperation_;
    currentOperation_ = 0;
    if ( ussdOperationId_ == currentOperationId_ ) {
        ussdOperationId_ = invalidOpId();
    }
    currentOperationId_ = invalidOpId();
    // changed_ = true;
    const unsigned opcount = operationsCount();
    if ( opcount == 0 ) {
        expirationTime_ = expirationTimeAtLeast_;
        // smsc_log_debug(log_, "session=%p key=%s has no ops, expiration=%d",
        //    this, sessionKey().toString().c_str(), int(expirationTime_ - now));
        // NOTE: we don't check for expired session here!
        // As closeOperation is performed from locked session,
        // session expiration time will be guaranteedly checked in releaseSession.
    }

    smsc_log_debug( log_, "session=%p/%s closeOp(op=%p opid=%u type=%d(%s)) => opcnt=%u etime=%d",
                    this,
                    sessionKey().toString().c_str(),
                    prevop,
                    unsigned(prevopid),
                    prevoptype,
                    commandOpName(prevoptype),
                    opcount,
                    int(expirationTime_ - time(0)) );
}



bool Session::hasPersistentOperation() const
{
    int opid;
    Operation* op;
    for ( IntHash< Operation* >::Iterator i(operations_);
          i.Next(opid,op);
          ) {
        if ( op->flagSet( OperationFlags::PERSISTENT ) )
            return true;
    }
    return false;
}


bool Session::isNew( int serv, int trans ) const {
    TransportNewFlag* f = isnew_.GetPtr( serv );
    if ( ! f ) return true;
    return f->isNew( trans );
}


void Session::setNew( int serv, int trans, bool an ) {
    TransportNewFlag* f = isnew_.GetPtr( serv );
    if ( f ) {
        f->setNew( trans, an );
    } else if ( !an ) {
        TransportNewFlag ff;
        ff.setNew( trans, an );
        isnew_.Insert(serv,ff);
    } // else already is new
}


void Session::pushInitRuleKey( int serv, int trans )
{
    std::pair<int,int> st(serv,trans);
    if ( ! initrulekeys_.empty() && st == initrulekeys_.front() ) return;
    initrulekeys_.push_front( st );
}

bool Session::getRuleKey( int& serv, int& trans ) const
{
    if ( initrulekeys_.empty() ) return false;
    const std::pair<int,int>& st = initrulekeys_.front();
    serv = st.first;
    trans = st.second;
    return true;
}


void Session::dropInitRuleKey( int serviceId, int transport, int wtime )
{
    if ( ! initrulekeys_.empty() ) {

        const std::pair<int,int> st(serviceId,transport);
        for ( std::list< std::pair<int,int> >::iterator i = initrulekeys_.begin();
              i != initrulekeys_.end();
              ++i ) {
            if ( *i == st ) {
                initrulekeys_.erase( i );
                setNew( serviceId, transport, true );
                break;
            }
        }

        if ( wtime >= 0 ) {
            if ( wtime > 100000 ) {
                smsc_log_warn( log_, "too great wait=%u", unsigned(wtime) );
                // ::abort();
            }
            if ( initrulekeys_.empty() ) {
                // if no services left, then reset session expiration time
                const time_t waitfor = time_t(time(0) + wtime);
                // smsc_log_debug( log_, "session=%p/%s no more initrulekeys",
                // this, sessionKey().toString().c_str() );
                if ( expirationTimeAtLeast_ > waitfor ) {
                    expirationTimeAtLeast_ = waitfor;
                    if ( operationsCount() == 0 )
                        expirationTime_ = expirationTimeAtLeast_;
                }
            } else {
                // there is at least one service
                waitAtLeast( unsigned(wtime) );
            }
        }
    }
}


/// create context Scope.
int Session::createContextScope()
{
    if ( ! contextScopes_ ) contextScopes_ = new IntHash< SessionPropertyScope* >;
    int32_t cid = ++nextContextId_;
    if ( ! cid ) cid = ++nextContextId_;
    contextScopes_->Insert( cid, ::makeContextScope(this) );
    return cid;
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
        globalScope_ = ::makeGlobalScope(this);
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
        res = ::makeServiceScope( this );
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
        throw SCAGException( "session=%p/%s cannot get operation scope, op=0",
                             this, sessionKey().toString().c_str() );

    if ( ! operationScopes_ )
        operationScopes_ = new IntHash< SessionPropertyScope* >;
    SessionPropertyScope** sptr = operationScopes_->GetPtr( getCurrentOperationId() );
    if ( ! sptr ) {
        res = ::makeOperationScope( this );
        operationScopes_->Insert( getCurrentOperationId(), res );
    } else {
        res = *sptr;
        if ( ! res ) res = *sptr = ::makeOperationScope( this );
    }
    return res;
}


void Session::waitAtLeast( unsigned sec )
{
    const time_t now = time(0);
    const time_t t = now + sec;
    if ( t > expirationTimeAtLeast_ ) {
        expirationTimeAtLeast_ = t;
        if ( t > expirationTime_ ) expirationTime_ = t;
        /*
        smsc_log_debug( log_, "session=%p/%s wait at least tmASH=%d/%d/%d",
                        this, sessionKey().toString().c_str(),
                        int(lastAccessTime_ - now),
                        int(expirationTime_ - now),
                        int(expirationTimeAtLeast_ - now) );
         */
    }
}


unsigned Session::appendCommand( SCAGCommand* cmd )
{
    if (cmd) cmdQueue_.push_back( cmd );
    return unsigned(cmdQueue_.size());
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


void Session::serializeScope( Serializer& o, const SessionPropertyScope* s ) const
{
    const uint32_t sz = s ? s->size() : 0;
    if (!sz) {
        o << sz;
        return;
    }
    s->serialize( o );
}


void Session::deserializeScope( Deserializer& o,
                                SessionPropertyScope*& s,
                                Opmaker opmaker ) /* throw (DeserializerException) */
{
    const size_t rpos = o.rpos();
    uint32_t sz;
    o >> sz;
    if ( s ) s->clear();
    if ( !sz ) return;
    o.setrpos( rpos );
    if ( !s ) s = (*opmaker)( this );
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


void Session::deserializeScopeHash( Deserializer& o,
                                    IntHash< SessionPropertyScope* >*& s,
                                    Opmaker opmaker ) /* throw (DeserializerException) */
{
    uint32_t sz;
    o >> sz;
    clearScopeHash( s );
    if ( !sz ) return;
    if ( !s ) s = new IntHash< SessionPropertyScope* >;
    for ( ; sz > 0; --sz ) {
        uint32_t key;
        SessionPropertyScope* value = 0;
        o >> key;
        deserializeScope(o,value,opmaker);
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
    if ( ++newopid_ == invalidOpId() ) ++newopid_;
    return newopid_;
}


// ======================================


ActiveSession::ActiveSession( SessionStore& st, Session& s ) :
store_(&st), s_(&s) {}

void ActiveSession::release() 
{
    if ( s_ ) store_->releaseSession( *s_ );
    s_ = 0;
}

void ActiveSession::moveLock( SCAGCommand* cmd )
{
    if ( s_ ) store_->moveLock( *s_, cmd );
}

} // namespace sessions
} // namespace scag
