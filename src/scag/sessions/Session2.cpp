#include <cassert>
#include <cstdlib>      // for abort
#include <sys/time.h>

#include "util/Exception.hpp"
#include "scag/exc/SCAGExceptions.h"
#include "core/synchronization/Mutex.hpp"
#include "ExternalTransaction.h"
#include "Operation.h"
#include "Session2.h"
#include "SessionStore2.h"
#include "scag/re/CommandOperations.h"

namespace {

    smsc::core::synchronization::Mutex sessionloggermutex;

} // namespace


namespace scag2 {
namespace sessions {

using namespace scag::exceptions;

SessionPropertyScope::~SessionPropertyScope()
{
    clear();
}


Serializer& SessionPropertyScope::serialize( Serializer& o ) const
{
    uint32_t sz = size();
    o << sz;
    if ( sz > 0 ) {
        char* key;
        AdapterProperty* value;
        for ( Hash< AdapterProperty* >::Iterator i(&properties_); i.Next(key,value); --sz ) {
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
                AdapterProperty* p = new AdapterProperty( key, patron_, value );
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
            delete key;
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


smsc::logger::Logger* Session::log_ = 0;


Session::Session( const SessionKey& key ) :
key_(key),
expirationPolicy_(COMMON),
pkey_(key),
command_(0),
transactions_(0),
globalScope_(0),
serviceScopes_(0),
contextScopes_(0),
operationScopes_(0)
{
    if ( ! log_ ) {
        MutexGuard mg(::sessionloggermutex);
        if ( !log_ ) log_ = smsc::logger::Logger::getInstance("session");
    }
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
                delete key;
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


Operation* Session::getCurrentOperation() const
{
    return currentOperation_;
}


Operation* Session::setCurrentOperation( opid_type opid )
{
    do {
        if ( opid == 0 ) break;

        Operation** optr = operations_.GetPtr(opid);
        if ( ! optr ) throw SCAGException( "Cannot find operation id=%u, key=%s",
                                           unsigned(opid), sessionKey().toString().c_str() );
        currentOperationId_ = opid;
        currentOperation_ = *optr;
        smsc_log_debug( log_, "Session: set current operation=%p, id=%u, type=%d",
                        currentOperation_, unsigned(opid), currentOperation_->type() );
        // changed_ = true;

    } while ( false );
    return currentOperation_;
}


opid_type Session::getUSSDOperationId() const
{
    return ussdOperationId_;
}


Operation* Session::createOperation( SCAGCommand& cmd, int operationType )
{
    std::auto_ptr< Operation > auop( new Operation(this, operationType) );
    Operation* op = auop.get();

    opid_type opid = getNewOperationId();
    cmd.setOperationId( opid );
    operations_.Insert( opid, auop.release() );
    if ( operationType == re::CO_USSD_DIALOG ) {
        // if the operation is already there, replace it
        if ( ussdOperationId_ != 0 ) {
            Operation* oldop = operations_.Get( ussdOperationId_ );
            smsc_log_debug( log_, "** Session: old USSD operation exists, delete: session=%p, key=%s, op=%p, opid=%u",
                            this, sessionKey().toString().c_str(), oldop, unsigned(ussdOperationId_) );
            operations_.Delete( ussdOperationId_ );
            delete oldop;
        }
        ussdOperationId_ = opid;
    }
    currentOperationId_ = opid;
    currentOperation_ = op;
    // changed_ = true;
    smsc_log_debug( log_, "** Session: create new operation: session=%p, key=%s, opid=%u, type=%d",
                    this, sessionKey().toString().c_str(), unsigned(currentOperationId_), operationType );
    return currentOperation_;
}


void Session::closeCurrentOperation()
{
    if ( ! currentOperation_ ) return;
    smsc_log_debug( log_, "Session: close current operation, session=%p key=%s op=%p opid=%u type=%d",
                    this, sessionKey().toString().c_str(), currentOperation_,
                    unsigned(currentOperationId_), currentOperation_->type() );

    operations_.Delete( currentOperationId_ );
    delete currentOperation_;
    currentOperation_ = 0;
    if ( ussdOperationId_ == currentOperationId_ ) {
        ussdOperationId_ = 0;
    }
    currentOperationId_ = 0;
    // changed_ = true;
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

            if (value) delete value;

        }
        s->Empty();
    }


    void Session::abort()
    {
        // FIXME
        ::abort();
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
