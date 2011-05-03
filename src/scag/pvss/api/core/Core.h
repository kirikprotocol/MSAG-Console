#ifndef _SCAG_PVSS_CORE_CORE_H
#define _SCAG_PVSS_CORE_CORE_H

#include <memory>
#include <vector>
#include "ChannelInactivityListener.h"
#include "ChannelInactivityTracker.h"
#include "Config.h"
#include "core/buffers/XHash.hpp"
#include "core/threads/ThreadPool.hpp"
#include "scag/util/WatchedThreadedTask.h"
#include "logger/Logger.h"
#include "PacketReader.h"
#include "PacketWriter.h"

namespace scag2 {
namespace pvss {

class Packet;
class Protocol;

namespace core {

class PacketWriter;
class PacketReader;

/// abstract class
class Core : public util::WatchedThreadedTask, public ChannelInactivityListener
{
public:
    /**
     * Enum defines packet states:<br/>
     * RECEIVED - New packet was receiver by PacketReader<br/>
     * EXPIRED  - Sending packet was expired<br/>
     * FAILED   - Packet wasn't sent (error occured)<br/>
     * SENT     - Packet was succesfuly sent
     */
    enum PacketState {
            RECEIVED = 1,
            EXPIRED,
            FAILED,
            SENT
    };

protected:
    /// config and proto gets owned
    Core( Config* theConfig, Protocol* theProtocol ) :
    config(theConfig),
    protocol(theProtocol),
    inactivityTracker(new ChannelInactivityTracker(*this,config->getInactivityTime()))
    {
        if ( !logger ) {
            static smsc::core::synchronization::Mutex mtx;
            smsc::core::synchronization::MutexGuard mg(mtx);
            if ( !logger ) logger = smsc::logger::Logger::getInstance("pvss.core");
        }
    }

public:
    virtual ~Core();


    Protocol* getProtocol() const {
        return protocol;
    }
    

    /**
     * Method pass a new packet (received from network) to a processing into core.
     * Obviously, packet state is RECEIVED.
     */
    virtual void receivePacket( std::auto_ptr<Packet> packet, PvssSocket& channel ) = 0;

    /**
     * Method notifies core logic that context (owning the packet) has been sent/expired/failed.
     *
     * @param context   Context owning the packet.
     * @param state     Packet state in IO processing { EXPIRED, SENT, FAIL }
     * 
     */
    virtual void reportPacket( uint32_t seqNum, smsc::core::network::Socket& channel, PacketState state ) = 0;

    /**
     * Method reports that error was occured when operating with channel.
     * Logs error and closes channel.
     *
     * @param exc       Exception contains proplem description
     * @param channel   Channel on wich problem was occurred
     */
    void handleError( const PvssException& exc, PvssSocket& channel ) {
        smsc_log_error( logger, "exc %s on socket %p of channel %p: %s",
                        PvssException::statusToString(exc.getType()),
                        channel.socket(), &channel, exc.what() );
        closeChannel( * channel.socket() );
    }
        
    /**
     * Method closes registered channel.
     * Used from Reader or Writer tasks to report that channel is needed to be closed.
     *
     * @param channel   Channel to close
     */
    virtual void closeChannel( smsc::core::network::Socket& channel );

    /**
     * Method registers a channel for regular IO operations.
     * A channel should be already connected.
     */
    virtual bool registerChannel( PvssSocket& channel, util::msectime_type utime)
    {
        try {
            registerForRead(channel);
            registerForWrite(channel);
            inactivityTracker->registerChannel(channel, utime);
            smsc_log_info(logger,"Socket %p connected and registered", channel.socket() );
        }
        catch (PvssException& register_exc) {
            smsc_log_error( logger, "Failed to register new channel. Details: %s", register_exc.what() );
            closeChannel( *channel.socket() );
            return false;
        }
        return true;
    }

    virtual Config& getConfig() const {
        return *config;
    }

protected:

    void updateChannelActivity( PvssSocket& channel ) {
        updateChannelActivity(channel, util::currentTimeMillis());
    }

    void startupIO() /* throw(PvssException) */ ;

    void shutdownIO( bool writePending )
    {
        {
            smsc::core::synchronization::MutexGuard mg(readersMutex);
            stopReaders();
        }
        stopCoreLogic();
        {
            smsc::core::synchronization::MutexGuard mg(writersMutex);
            stopWriters( writePending );
        }
        inactivityTracker->shutdown();
        // waiting until all tasks are stopped
        destroyDeadTasks();
    }

    virtual void stopCoreLogic() {}

private:
    void updateChannelActivity( PvssSocket& channel, util::msectime_type utime ) {
        inactivityTracker->updateChannel(channel, utime);
    }

    void registerForWrite( PvssSocket& channel ) /* throw(PvssException) */ ;

    void registerForRead( PvssSocket& channel ) /* throw(PvssException) */ ;

    void stopReaders()
    {
        for ( std::vector<PacketReader*>::iterator i = readers.begin();
              i != readers.end();
              ++i ) {
            (*i)->shutdown();
        }
        smsc::core::synchronization::MutexGuard mg(deadMutex);
        std::copy(readers.begin(),readers.end(),std::back_inserter(deadTasks));
        readers.clear();
    }

    void stopWriters( bool writePending )
    {
        for ( std::vector<PacketWriter*>::iterator i = writers.begin();
              i != writers.end();
              ++i ) {
            if (writePending) (*i)->writePending();
            (*i)->shutdown();
        }
        smsc::core::synchronization::MutexGuard mg(deadMutex);
        std::copy(writers.begin(),writers.end(),std::back_inserter(deadTasks));
        writers.clear();
    }

    void destroyDeadTasks()
    {
        std::vector<SockTask*> aCopy;
        {
            smsc::core::synchronization::MutexGuard mg(deadMutex);
            aCopy = deadTasks;
            deadTasks.clear();
        }
        for ( std::vector<SockTask*>::iterator i = aCopy.begin();
              i != aCopy.end();
              ++i ) {
            (*i)->shutdown();
            (*i)->waitUntilReleased();
            delete *i;
        }
    }

protected:
    static smsc::logger::Logger*     logger;

protected:
    Config*                          config;
    Protocol*                        protocol;
    std::auto_ptr<ChannelInactivityTracker> inactivityTracker;

    smsc::core::synchronization::Mutex readersMutex;
    smsc::core::synchronization::Mutex writersMutex;
    std::vector< PacketReader* >       readers;
    std::vector< PacketWriter* >       writers;

    smsc::core::synchronization::Mutex deadMutex;
    std::vector< SockTask* >           deadTasks;
    smsc::core::threads::ThreadPool    threadPool_;
};

} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_CORE_H */
