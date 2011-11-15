#ifndef _SCAG_PVSS_SERVER_IOTASK_H_
#define _SCAG_PVSS_SERVER_IOTASK_H_

#include <vector>
#include "logger/Logger.h"
#include "core/threads/ThreadedTask.hpp"
#include "core/synchronization/EventMonitor.hpp"
// #include "core/buffers/Array.hpp"
#include "core/network/Multiplexer.hpp"
#include "core/network/Pipe.hpp"
#include "ConnectionContext.h"

namespace scag2 {
namespace pvss  { 

class IOTask : public smsc::core::threads::ThreadedTask
{

protected:
    typedef std::vector< ConnPtr > ConnArray;

public:
    IOTask( uint32_t connectionTimeout,
            uint16_t ioTimeout,
            const char* logName);
    virtual ~IOTask() {}
    virtual int Execute();
    virtual void stop();

    virtual void registerContext(ConnPtr& cx);

    /// add to unregister queue
    void unregisterContext(ConnPtr& cx);
    
    uint32_t getSocketsCount();

    bool operator < ( IOTask& t ) {
        return getSocketsCount() < t.getSocketsCount();
    }

protected:
    // invoked with socketMontitor_ locked
    virtual bool hasExtraEvents() { return true; }

    // NOTE: under lock
    virtual void processSockets( smsc::core::network::Multiplexer::SockArray &ready,
                                 smsc::core::network::Multiplexer::SockArray &error,
                                 time_t now) = 0;

    virtual void preDisconnect( ConnectionContext* cx );

    // void showmul();

private:

    // NOTE: under lock
    time_t checkConnectionTimeout();
    void removeWorking( ConnectionContext* cx );

protected:
    smsc::logger::Logger* log_;

    smsc::core::network::Pipe        wakepipe_;
    smsc::core::network::Multiplexer multiplexer_;
    ConnArray working_; // access only from working thread
    const uint16_t ioTimeout_;

    smsc::core::synchronization::EventMonitor socketMonitor_;
private:
    ConnArray waiting_; // to be added to multiplexer
    ConnArray unwaiting_; // to be removed from multiplexer

    const uint32_t connectionTimeout_;
    uint16_t checkTimeoutPeriod_;
    time_t lastCheckTime_;

};


class MTPersReader: public IOTask 
{
public:
    MTPersReader(uint32_t connectionTimeout,
                 uint16_t ioTimeout) : IOTask(// iomanager,
                                              connectionTimeout, ioTimeout, "oldredr") {}
    virtual const char * taskName() { return "MTPersReader"; }
    Performance getPerformance();

protected:
    void processSockets(smsc::core::network::Multiplexer::SockArray &ready,
                        smsc::core::network::Multiplexer::SockArray &error,
                        time_t now);

    virtual void registerContext( ConnPtr& ptr );
    virtual void preDisconnect( ConnectionContext* cx );

private:
    smsc::core::synchronization::Mutex perfLock_;
    Performance                        performance_;
    ConnArray                          perfSockets_;
};


class MTPersWriter: public IOTask 
{
public:
    MTPersWriter(uint32_t connectionTimeout,
                 uint16_t ioTimeout) : 
        IOTask(connectionTimeout, ioTimeout, "oldwrtr"), packetIsReady_(false) {}
    virtual const char * taskName() { return "MTPersWriter"; }

    void packetIsReady();

protected:
    virtual bool hasExtraEvents();
    virtual void registerContext(ConnPtr& cx);
    void preDisconnect( ConnectionContext* cx );
    void processSockets(smsc::core::network::Multiplexer::SockArray &ready,
                        smsc::core::network::Multiplexer::SockArray &error,
                        time_t now);

private:
    bool packetIsReady_;
};

}//pvss
}//scag2

#endif

