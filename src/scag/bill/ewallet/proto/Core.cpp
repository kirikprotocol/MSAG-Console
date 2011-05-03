#include "Core.h"
#include "SocketReader.h"
#include "SocketWriter.h"

using smsc::core::synchronization::MutexGuard;

namespace {
using namespace scag2::bill::ewallet::proto;

struct SocketCountComparator
{
    bool operator () ( const SocketTask* r1, const SocketTask* r2 ) const {
        return ( r1->sockets() < r2->sockets() );
    }
};

}


namespace scag2 {
namespace bill {
namespace ewallet {
namespace proto {

void Core::closeSocket( Socket& socket )
{
    socket.close();
}


bool Core::registerSocket( Socket& socket )
{
    try {
        // registerForWrite(...);
        SocketWriter* writer = 0;
        {
            MutexGuard mg(writersMutex_);
            if ( ! writers_.empty() ) {
                std::sort( writers_.begin(), writers_.end(), SocketCountComparator() );
                writer = writers_.front();
            }
        }
        if ( writer == 0 ) {
            throw Exception(Status::CONFIG_INVALID, "no writer is created!");
        }
        if ( writer->sockets() >= config_->getMaxWriterSocketCount() ) {
            throw Exception(Status::CLIENT_BUSY, "maximum count of writer sockets exceeded");
        }

        // registerForRead(...);
        SocketReader* reader = 0;
        {
            MutexGuard mg(readersMutex_);
            if ( ! readers_.empty() ) {
                std::sort( readers_.begin(), readers_.end(), SocketCountComparator() );
                reader = readers_.front();
            }
        }
        if ( reader == 0 ) {
            throw Exception(Status::CONFIG_INVALID, "no reader is created!");
        }
        if ( reader->sockets() >= config_->getMaxReaderSocketCount() ) {
            throw Exception(Status::CLIENT_BUSY, "maximum count of reader sockets exceeded");
        }
        writer->registerSocket(socket);
        reader->registerSocket(socket);

        if (tracker_.get()) tracker_->registerSocket(socket);

    } catch ( Exception& exc ) {
        smsc_log_error( log_, "failed to register new socket: %s", exc.what() );
        closeSocket(socket);
        return false;
    }
    return true;
}


void Core::startupIO()
{
    smsc_log_info( log_, "Starting up IO tasks...");
    {
        MutexGuard mg(readersMutex_);
        stopReaders();
        for ( size_t i = 0; i < config_->getReadersCount(); i++) {
            try {
                SocketReader* reader = new SocketReader(*this);
                readers_.push_back( reader );
                threadPool_.startTask(reader,false);
            } catch ( Exception& exc) {
                stopReaders();
                throw Exception( Status::IO_ERROR, "Failed to start readers: %s", exc.what());
            }
        }
    }

    {
        MutexGuard mg(writersMutex_);
        stopWriters(false);
        for ( size_t i=0; i < config_->getWritersCount(); i++) {
            try {
                SocketWriter* writer = new SocketWriter(*this);
                writers_.push_back(writer);
                threadPool_.startTask(writer,false);
            } catch (Exception& exc) {
                {
                    MutexGuard mgr(readersMutex_);
                    stopReaders();
                }
                stopWriters(false);
                throw Exception( Status::IO_ERROR, "Failed to start writers: %s", exc.what());
            }
        }
    }
    // we don't need a tracker
    // tracker_.reset( new SocketInactivityTracker(*this) );
    // threadPool_.startTask( tracker_.get(),false);
    smsc_log_info(log_, "IO tasks started");
}


void Core::shutdownIO( bool serverLike )
{
    if ( serverLike ) {
        {
            MutexGuard mg(readersMutex_);
            stopReaders();
        }
    }
    stopCoreLogic();
    {
        MutexGuard mg(writersMutex_);
        stopWriters(serverLike);
    }
    if ( ! serverLike ) {
        {
            MutexGuard mg(readersMutex_);
            stopReaders();
        }
    }

    if (tracker_.get()) tracker_->shutdown();

    // destroyDeadTasks();
    std::vector< SocketTask* > deadtasks;
    {
        MutexGuard mg(deadMutex_);
        deadtasks = deadTasks_;
        deadTasks_.clear();
    }
    for ( std::vector< SocketTask* >::iterator i = deadtasks.begin();
          i != deadtasks.end();
          ++i ) {
        (*i)->shutdown();
        (*i)->waitUntilReleased();
        delete *i;
    }
}


void Core::stopReaders()
{
    for ( std::vector< SocketReader* >::iterator i = readers_.begin();
          i != readers_.end();
          ++i ) {
        (*i)->shutdown();
    }
    MutexGuard mg(deadMutex_);
    deadTasks_.insert( deadTasks_.end(), readers_.begin(), readers_.end() );
    readers_.clear();
}


void Core::stopWriters( bool serverLike )
{
    for ( std::vector< SocketWriter* >::iterator i = writers_.begin();
          i != writers_.end();
          ++i ) {
        if ( serverLike ) (*i)->writePending();
        (*i)->shutdown();
    }
    MutexGuard mg(deadMutex_);
    deadTasks_.insert( deadTasks_.end(), writers_.begin(), writers_.end() );
    writers_.clear();
}

} // namespace proto
} // namespace ewallet
} // namespace bill
} // namespace scag2
