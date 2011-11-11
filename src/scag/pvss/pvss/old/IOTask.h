#ifndef _SCAG_PVSS_SERVER_IOTASK_H_
#define _SCAG_PVSS_SERVER_IOTASK_H_

#include "logger/Logger.h"
#include "core/threads/ThreadedTask.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/buffers/Array.hpp"
#include "core/network/Multiplexer.hpp"
#include "core/network/Pipe.hpp"
#include "ConnectionContext.h"

namespace scag2 {
namespace pvss  { 

class IOTask : public smsc::core::threads::ThreadedTask
{

protected:
    typedef smsc::core::buffers::Array< ConnPtr > ConnArray;

public:
    IOTask( uint32_t connectionTimeout,
            uint16_t ioTimeout,
            const char* logName);
    virtual ~IOTask() {}
    virtual int Execute();
    virtual void stop();

    void registerContext(ConnPtr& cx);

    /// try to unregister
    /// @return true if unregistered
    bool unregisterContext(ConnPtr& cx);
    
    uint32_t getSocketsCount();

    bool operator < ( IOTask& t ) {
        return getSocketsCount() < t.getSocketsCount();
    }

protected:
    // NOTE: under lock
    virtual void processSockets( smsc::core::network::Multiplexer::SockArray &ready,
                                 smsc::core::network::Multiplexer::SockArray &error,
                                 time_t now) = 0;

    virtual void preDisconnect( ConnectionContext* cx );

    void showmul();

private:

    // NOTE: under lock
    time_t checkConnectionTimeout();

protected:
    smsc::logger::Logger* log_;
    smsc::core::synchronization::EventMonitor socketMonitor_;
    smsc::core::network::Pipe        wakepipe_;
    smsc::core::network::Multiplexer multiplexer_;
    ConnArray working_; // is in work
    const uint16_t ioTimeout_;

private:
    const uint32_t connectionTimeout_;
    uint16_t checkTimeoutPeriod_;
    time_t lastCheckTime_;

    ConnArray waiting_; // to be added to multiplexer
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
    void preDisconnect( ConnectionContext* cx );

private:
    Performance performance_;
};


class MTPersWriter: public IOTask 
{
public:
    MTPersWriter(uint32_t connectionTimeout,
                 uint16_t ioTimeout):IOTask(connectionTimeout, ioTimeout, "oldwrtr") {};
    virtual const char * taskName() { return "MTPersWriter"; }
protected:
    void processSockets(smsc::core::network::Multiplexer::SockArray &ready,
                        smsc::core::network::Multiplexer::SockArray &error,
                        time_t now);
};

}//pvss
}//scag2

#endif

