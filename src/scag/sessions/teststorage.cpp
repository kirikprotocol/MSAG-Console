#include <cassert>
#include <time.h>
#include "SessionStore2.h"
#include "core/buffers/CyclicQueue.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/Thread.hpp"
#include "logger/Logger.h"
#include "scag/config/Config.h"
#include "scag/config/ConfigView.h"
#include "scag/config/TestConfig.h"
#include "scag/util/storage/StorageNumbering.h"
#include "scag/transport/SCAGCommand2.h"

using namespace scag::sessions2;
using namespace scag::transport2;
using namespace scag::config;

SessionKey genKey( unsigned node )
{
    uint64_t k;
    const StorageNumbering& n = StorageNumbering::instance();
    do {
        k = 89130000000ULL + random() % 1000000;
    } while ( n.nodeByNumber( k ) != node );
    return SessionKey( k );
}


smsc::logger::Logger* slog = 0;
smsc::logger::Logger* glog = 0;
smsc::logger::Logger* mlog = 0;
smsc::logger::Logger* qlog = 0;


/*
class StoppingThread : public smsc::core::threads::Thread
{
public:
    StoppingThread() {}
    virtual ~StoppingThread() {}

    static void stop() 
    {
        MutexGuard mg(stopMutex_);
        stopping_ = true;
    }

protected:
    static Mutex stopMutex_;
    static bool  stopping_;
};
 */


class StateMach : public smsc::core::threads::Thread
{
public:
    StateMach( unsigned          mynode,
               SessionStore*     store,
               SCAGCommandQueue* queue ) :
    node_(mynode), store_(store), queue_(queue) {}

    virtual ~StateMach() {}
    virtual int Execute();

private:
    unsigned          node_;
    SessionStore*     store_;
    SCAGCommandQueue* queue_;
};



class SKCommand : public SCAGCommand
{
public:
    SKCommand( const SessionKey& sk ) : sk_(sk), s_(0) {}
    virtual ~SKCommand() {}
    
    virtual scag::transport::TransportType getType() const {
        return scag::transport::SMPP;
    }

    virtual int getServiceId() const { return 0; }
    virtual void setServiceId( int ) {}
    
    virtual int64_t getOperationId() const { return 0; }
    virtual void setOperationId(int64_t ) {}

    virtual uint8_t getCommandId() const { return 1; }

    const SessionKey& sessionKey() const { return sk_; }

    virtual Session* getSession() {
        return s_;
    }
protected:
    virtual void setSession( Session* s ) {
        s_ = s;
    }

protected:
    SessionKey   sk_;
    Session*     s_;     // not owned
};


class SCAGCommandGenerator : public smsc::core::threads::Thread
{
public:
    SCAGCommandGenerator( unsigned mynode,
                          SCAGCommandQueue* queue ) :
    node_(mynode), queue_(queue) {
        assert(queue_);
    }

    virtual ~SCAGCommandGenerator() {}
    virtual int Execute();

private:
    unsigned          node_;
    SCAGCommandQueue* queue_;
};



class DummySessionFinalizer : public SessionFinalizer
{
public:
    DummySessionFinalizer() {}
    virtual ~DummySessionFinalizer() {}
    virtual void finalize( Session& s ) {
        smsc_log_debug( slog, "session %p is finalized", &s );
    }
};


class SCAGCommandQueueImpl : public SCAGCommandQueue
{
public:
    SCAGCommandQueueImpl( unsigned waitsize = 1000000 ) :
    stopping_(false),
    statprocess_(0),
    statmaxsize_(0),
    statstarttm_(time(0)),
    count_(0),
    waitsize_(waitsize) {}

    virtual ~SCAGCommandQueueImpl() 
    {
        stop(); 
        MutexGuard mg(lock_);
        while ( count_ > 0 ) 
        {
            smsc_log_debug( qlog, "queue size is: %u, waiting", count_ );
            lock_.wait(1000);
            lock_.notifyAll();
        }
        lock_.wait(10);
    }

    virtual void stop() 
    {
        smsc_log_debug(qlog, "scag command queue is stopping" );
        MutexGuard mg(lock_);
        stopping_ = true;
        lock_.notifyAll();
        double freq = ( time(0) - statstarttm_ );
        if ( freq > 0 ) freq = statprocess_ / freq;
        printf( "queue statistics: processed=%lu, maxsize=%u, freq=%6.0fHz\n",
                statprocess_, statmaxsize_, freq );
        smsc_log_info( qlog, "statistics: processed=%lu, maxsize=%u, freq=%6.0fHz",
                       statprocess_, statmaxsize_, freq );
    }

    virtual bool isStopping() const {
        MutexGuard mg(lock_);
        return stopping_;
    }

    // NOTE: cmd ownership is taken.
    virtual unsigned pushCommand( SCAGCommand* cmd,
                                  int action = PUSH )
    {
        if ( ! cmd ) return unsigned(-1);

        do { // fake loop

            MutexGuard mg(lock_);

            if ( action == RESERVE ) {
                ++count_;
                smsc_log_debug( qlog, "reserve place for a cmd=%p: sz=%u/%u", cmd, queue_.Count(), count_ );
                return count_;
            }

            if ( action == PUSH ) {
                // only for fresh commands
                while ( count_ >= waitsize_ && ! stopping_ ) {
                    lock_.Unlock();
                    {
                        MutexGuard wmg(waitlock_);
                        waitlock_.wait(1000);
                    }
                    lock_.Lock();
                }
            }

            if ( stopping_ ) {
                if ( action == MOVE && count_ > 0 ) --count_;
                smsc_log_debug( qlog, "stopping, sz=%u/%u", queue_.Count(), count_ );
                lock_.notifyAll();
                break;
            }

            queue_.Push( cmd );
            if ( action == PUSH ) ++count_;
            lock_.notify();
            smsc_log_debug( qlog, "pushing command %p to a queue, sz=%u/%u", cmd, queue_.Count(), count_ );
            if ( count_ > statmaxsize_ ) statmaxsize_ = count_;
            return count_;

        } while ( false );

        if ( ! cmd->getSession() ) delete cmd; // else cmd is owned by session
        return unsigned(-1);

    }


    virtual SCAGCommand* popCommand()
    {
        SCAGCommand* cmd = 0;
        bool waitnotify = false;
        {
            MutexGuard mg(lock_);
            while ( true ) {

                if ( queue_.Pop( cmd ) ) {

                    if ( count_ == 0 )
                        smsc_log_warn( qlog, "popping command when count=%u", count_ );
                    else {
                        if ( count_ == waitsize_ ) waitnotify = true;
                        --count_;
                    }
                    ++statprocess_;
                    smsc_log_debug( qlog, "popping command %p from a queue, sz=%u/%u", cmd, queue_.Count(), count_ );
                }

                if ( stopping_ ) {
                    lock_.notifyAll();
                    waitnotify = true;
                    break;
                }

                if ( cmd ) break;

                lock_.wait(500);
            }
        }
        if ( waitnotify ) {
            MutexGuard mg(waitlock_);
            waitlock_.notify();
        }
        return cmd;
    }


private:
    bool                                                               stopping_;
    mutable smsc::core::synchronization::EventMonitor                  lock_;
    mutable smsc::core::synchronization::EventMonitor                  waitlock_;
    smsc::core::buffers::CyclicQueue< SCAGCommand* > queue_;

    // statistics
    unsigned long statprocess_;
    unsigned      statmaxsize_;
    time_t        statstarttm_;
    unsigned      count_;
    unsigned      waitsize_;
};



int main( int argc, char** argv )
{
    const unsigned mynode = 0;

    smsc::logger::Logger::Init();
    slog = smsc::logger::Logger::getInstance("main");
    glog = smsc::logger::Logger::getInstance("gen");
    mlog = smsc::logger::Logger::getInstance("stmach");
    qlog = smsc::logger::Logger::getInstance("queue");

    smsc_log_info(slog, "===================================");
    smsc_log_info(slog, "===  TESTSTORAGE IS STARTED  ======");
    smsc_log_info(slog, "===================================");

    TestConfig tc;
    tc.init();

    const Config* cfg = tc.getConfig();
    if ( !cfg ) {
        smsc_log_error( slog, "cannot get config" );
    }

    // set the number of nodes
    scag::util::storage::StorageNumbering::setInstance( 5 );

    std::auto_ptr< DummySessionFinalizer > df( new DummySessionFinalizer );
    std::auto_ptr< SCAGCommandQueueImpl > cq( new SCAGCommandQueueImpl( 2000 ) );
    std::auto_ptr< SessionStore > ss( SessionStore::create
                                       ( mynode, * df.get(), * cq.get() ) );
    ss->init( ConfigView( *cfg, "sessions" ) );

    const unsigned machines = 50;
    std::auto_ptr<StateMach> machs[machines];

    for ( unsigned i = 0; i < machines; ++i ) {
        machs[i].reset( new StateMach(mynode, ss.get(), cq.get()) );
        machs[i]->Start();
    }
    
    std::auto_ptr<SCAGCommandGenerator> gen(new SCAGCommandGenerator(mynode, cq.get() ));
    gen->Start();

    timespec t;
    t.tv_sec = 60;
    t.tv_nsec = 0;
    nanosleep( &t, 0 );

    printf("sending stop to statemachines\n");
    cq->stop();
    printf("exit main\n");
    return 0;
}


int SCAGCommandGenerator::Execute()
{
    smsc_log_info( glog, "cmd generator started" );
    while ( true ) 
    {
        SCAGCommand* cmd = new SKCommand(genKey(node_));
        unsigned sz = queue_->pushCommand( cmd );
        if ( sz == unsigned(-1) ) break;
    }
    smsc_log_info( glog, "cmd generator finished" );
    return 0;
}


int StateMach::Execute()
{
    smsc_log_info( mlog,"state machine started");
    while ( true ) {

        SKCommand* cmd( static_cast< SKCommand* >( queue_->popCommand() ) );
        if ( ! cmd ) break;

        const SessionKey& key = cmd->sessionKey();
        smsc_log_debug( mlog, "got command %p, key=%s", cmd, key.toString().c_str() );

        /// cmd gets owned
        smsc_log_debug( mlog, "trying to get session for key=%s cmd=%p", key.toString().c_str(), cmd );
        ActiveSession as = store_->fetchSession( key, cmd );
        if ( ! as.get() ) {
            // cmd is pushed to session
            smsc_log_debug( mlog, "session for key=%s is LOCKED", key.toString().c_str() );
            continue;
        }
        smsc_log_debug( mlog, "got session %p, key=%s", as.get(), key.toString().c_str() );

        if ( random() % 100 < 5 ) as.setFlush( true );

        timespec t;
        t.tv_sec = 0;
        // const unsigned hz = 1000000;
        t.tv_nsec = 1000;
        nanosleep( &t, 0 );
        smsc_log_debug( mlog, "session %p, key=%s processed", as.get(), key.toString().c_str() );

    } // while
    smsc_log_info( mlog,"state machine finished");
    return 0;
}
