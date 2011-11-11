#include "Core.h"

#include <algorithm>

namespace {
using namespace scag2::pvss::core;

class ChannelCountComparator {
public:
    bool operator () ( SockTask* t1, SockTask* t2 ) const {
        return t1->sockets() < t2->sockets();
    }
};
}

namespace scag2 {
namespace pvss {
namespace core {

smsc::logger::Logger* Core::logger = 0;

Core::~Core()
{
    // shutdownIO();
    if ( !released() && isStarted() ) {
        waitUntilReleased();
    }
    delete protocol;
    delete config;
    smsc_log_info(logger,"dtor: core");
}


void Core::closeChannel( PvssSocketBase& channel )
{
    inactivityTracker->removeChannel(channel);
    // channel.disconnect();
    channel.getSocket()->Close();
    smsc_log_info(logger,"Channel closed: %p sock=%p",&channel,channel.getSocket());
}


bool Core::registerChannel( PvssSocketPtr& channel, util::msectime_type utime )
{
    try {
        registerForRead(channel);
        registerForWrite(channel);
        inactivityTracker->registerChannel(*channel, utime);
        smsc_log_info(logger,"channel %p sock=%p connected and registered",
                      channel.get(), channel->getSocket() );
    }
    catch (PvssException& register_exc) {
        smsc_log_error( logger, "Failed to register new channel. Details: %s", register_exc.what() );
        closeChannel( *channel );
        return false;
    }
    return true;
}


void Core::registerForWrite( PvssSocketPtr& channel ) /* throw (PvssException) */ 
{
    PacketWriter* writer = 0;
    {
        smsc::core::synchronization::MutexGuard mg(writersMutex);
        if ( !writers.empty() ) {
            // select busyless Writer
            writer = *std::min_element( writers.begin(),
                                        writers.end(),
                                        ChannelCountComparator());
        }
    }
    if (writer == 0)
        throw PvssException(PvssException::CONFIG_INVALID, "No one writer is created!" );
    if (writer->sockets() > unsigned(config->getMaxWriterChannelsCount()))
        throw PvssException(PvssException::CLIENT_BUSY, "Maximum count of writer channels exceeded");

    writer->registerChannel(channel);
}


void Core::registerForRead( PvssSocketPtr& channel ) /* throw(PvssException) */ 
{
    PacketReader* reader = 0;
    {
        smsc::core::synchronization::MutexGuard mg(readersMutex);
        if ( !readers.empty() ) {
            reader = *std::min_element(readers.begin(),
                                       readers.end(),
                                       ChannelCountComparator());
        }
    }
    if (reader == 0)
        throw PvssException(PvssException::CONFIG_INVALID, "No one reader is created!");
    if (reader->sockets() > unsigned(config->getMaxReaderChannelsCount()))
        throw PvssException(PvssException::CLIENT_BUSY, "Maximum count of reader channels exceeded");

    reader->registerChannel(channel);
}


void Core::startupIO() /* throw(PvssException) */ 
{
    smsc_log_info( logger, "Starting up IO tasks...");
    {
        smsc::core::synchronization::MutexGuard mg(readersMutex);
        stopReaders();
        for ( int i=0; i < config->getReadersCount(); i++) {
            try {
                PacketReader* reader = new PacketReader(*config, *this);
                readers.push_back( reader );
                threadPool_.startTask(reader,false);
            } catch (exceptions::IOException& init_exc) {
                stopReaders();
                throw PvssException(PvssException::IO_ERROR, "Failed to start readers: %s", init_exc.what());
            }
        }
    }

    {
        smsc::core::synchronization::MutexGuard mg(writersMutex);
        stopWriters(false);
        for (int i=0; i<config->getWritersCount(); i++) {
            try {
                PacketWriter* writer = new PacketWriter(*config,*this);
                writers.push_back(writer);
                threadPool_.startTask(writer,false);
            } catch (exceptions::IOException& init_exc) {
                {
                    smsc::core::synchronization::MutexGuard mgr(readersMutex);
                    stopReaders();
                }
                stopWriters(false);
                throw PvssException(PvssException::IO_ERROR, "Failed to start writers: %s", init_exc.what());
            }
        }
    }
    threadPool_.startTask(inactivityTracker.get(),false);
    smsc_log_info(logger, "IO tasks started");
}

} // namespace core
} // namespace pvss
} // namespace scag2
