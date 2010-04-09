#ifndef _SCAG_SESSIONS_IMPL_MANAGER2_H
#define _SCAG_SESSIONS_IMPL_MANAGER2_H

#include <map>
#include <list>
#include <time.h>
#include <unistd.h>
#include <string>
#include "SessionManager2.h"
#include "CompositeSessionStore.h"
#include "core/buffers/XHash.hpp"
#include "core/synchronization/Event.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/Thread.hpp"
#include "logger/Logger.h"
#include "scag/config/base/ConfigListener2.h"
#include "scag/config/base/ConfigManager2.h"
#include "scag/exc/SCAGExceptions.h"
#include "scag/re/base/LongCallContextBase.h"
#include "scag/sessions/base/SessionFinalizer.h"
#include "scag/sessions/base/Session2.h"
#include "scag/sessions/base/SessionManager2.h"
#include "scag/sessions/base/SessionExpirationQueue.h"
#include "util/int.h"

namespace scag2 {
namespace sessions {

// using transport::SCAGCommand;
// using transport::SCAGCommandQueue;

class SessionManagerImpl :
public smsc::core::threads::Thread,
public config::ConfigListener,
public SessionManager,
public lcm::LongCallInitiator,
public SessionFinalizer,
public SessionExpirationQueue
{
private:

    static const time_t DEFAULT_EXPIRE_INTERVAL = 60;

    /*
    struct ExpireData 
    {
        ExpireData( time_t expTime, const SessionKey& k ) :
        expiration(expTime), key(k) {}
        bool operator < ( const ExpireData& e ) const {
            return expiration < e.expiration;
        }
                
        bool operator == ( const ExpireData& e ) const {
            return expiration == e.expiration;
        }

    public:
        time_t      expiration;
        SessionKey  key;
    };
     */

    struct Expire {
        Expire( time_t e, time_t a, const SessionKey& sk ) : expire(e), access(a), key(sk) {}
        time_t      expire;
        time_t      access;
        SessionKey  key;
    };

    struct KeyPtr {
        explicit KeyPtr( const SessionKey* k ) : key(k) {}
        explicit KeyPtr( Expire& ek ) : key(&ek.key) {}
        bool operator == ( const KeyPtr& o ) const {
            return *o.key == *key;
        }
        static inline unsigned int CalcHash( const KeyPtr& k ) {
            return SessionKey::CalcHash( *k.key );
        }
    public:
        const SessionKey* key;
    };

    typedef std::list< Expire >                               ExpireList;
    typedef std::multimap< time_t, ExpireList::iterator >     ExpireMap;
    typedef XHash< KeyPtr, ExpireList::iterator, KeyPtr > ExpireHash;

    struct lessAccessTime;

public:

    // only one instance may be created!
    SessionManagerImpl();
    virtual ~SessionManagerImpl();

    // void AddRestoredSession(Session * session);

    void init( const scag2::config::SessionManagerConfig& config,
               unsigned nodeNumber,
               SCAGCommandQueue& cmdqueue );

    // virtual void releaseSession(SessionPtr session);

    virtual void getSessionsCount( uint32_t& sessionsCount,
                                   uint32_t& sessionsLoadedCount,
                                   uint32_t& sessionsLockedCount );

    void Start();

    // virtual SessionPtr newSession(CSessionKey& sessionKey);

    virtual void continueExecution( lcm::LongCallContextBase* context, bool dropped );

    /// --- interface of ConfigListener
    virtual void configChanged();

    /// --- interface of SessionExpirationQueue
    virtual void scheduleExpire( time_t            expirationTime,
                                 time_t            lastaccessTime,
                                 const SessionKey& key );

    /// --- interface of SessionFinalizer
    virtual bool finalize( Session& s );

protected:

    // SessionManager interface
    virtual ActiveSession fetchSession( const SessionKey&           key,
                                        std::auto_ptr<SCAGCommand>& cmd,
                                        bool                        create = true );

private:

    /// --- interface of Thread
    virtual int Execute();

    void Start( int ); // to make compiler happy
    virtual void Stop();
    bool isStarted();
    int  processExpire();
    // void deleteSession(SessionPtr& session);
    // bool processDeleteSession(SessionPtr& session);
    // bool deleteQueuePop(SessionPtr& s);
    // void deleteQueuePush(SessionPtr& s, bool expired);

    // uint16_t getNewUSR(Address& address);
    // uint16_t getLastUSR(Address& address);

    // void reorderExpireQueue(Session* session);
    ExpireMap::iterator findExpire( time_t expire, const SessionKey& key );

private:
    unsigned          nodeNumber_;
    unsigned          flushLimitSize_;
    unsigned          flushLimitTime_;
    unsigned          activeSessions_;
    SCAGCommandQueue* cmdqueue_;

    EventMonitor      expireMonitor_;

    // unsigned          inputCount_;    // count of items in the inputList_
    ExpireList*       inputList_;     // this one is used to pick input from scheduleExpire()
    ExpireList        expireList_;    // this list actually keeps expiration data
    ExpireMap         expireMap_;     // ordered by expiration time
    ExpireHash        expireHash_;    // fast access via sessionKey

    Logger*           log_;

    Mutex             stopLock_;
    bool              started_;
    time_t            oldestsession_;

    std::auto_ptr<SessionAllocator>       allocator_;
    std::auto_ptr<CompositeSessionStore>  store_;
    scag2::config::SessionManagerConfig   config_;
};

}}

#endif // SCAG_SESSIONS_MANAGER
