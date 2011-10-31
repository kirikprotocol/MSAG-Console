#ifndef _SCAG_PVSS_SERVER_IOTASK_H_
#define _SCAG_PVSS_SERVER_IOTASK_H_

#include "logger/Logger.h"
#include "core/threads/ThreadedTask.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/buffers/Array.hpp"
#include "core/network/Multiplexer.hpp"
// #include "scag/util/multiplexer/RWMultiplexer.hpp"
#include "ConnectionContext.h"

namespace scag2 {
namespace pvss  { 

class IOTaskManager;
class TasksSorter;

/*
class SortedTask : public smsc::core::threads::ThreadedTask 
{
    friend class IOTaskManager;
    friend class TasksSorter;
public:
    // SortedTask(uint32_t itemsCount = 0) : index_(0) {}
    // SortedTask() : index_(0) {}
    virtual ~SortedTask() {};
    virtual int Execute() { return 0; };
    virtual const char* taskName() { return ""; };
    // void addItem() { ++itemsCount_; };

protected:
    // uint32_t itemsCount_;
    uint16_t index_;
};
*/


class IOTask : public smsc::core::threads::ThreadedTask
{

protected:
    typedef smsc::core::buffers::Array< smsc::core::network::Socket* > SockArray;

public:
    IOTask( // IOTaskManager& iomanager,
           uint32_t connectionTimeout,
           uint16_t ioTimeout,
           const char* logName);
    virtual ~IOTask() {}
    virtual int Execute();
    virtual void stop();

    void registerContext(ConnectionContext* cx);
    uint32_t getSocketsCount();
    // void addSocket(smsc::core::network::Socket* s);

    bool operator < ( IOTask& t ) {
        return getSocketsCount() < t.getSocketsCount();
    }

protected:
    // NOTE: under lock
    virtual void processSockets(smsc::core::network::Multiplexer::SockArray &ready,
                                smsc::core::network::Multiplexer::SockArray &error,
                                time_t now) = 0;

    virtual void preDisconnect( ConnectionContext* cx ) {}

private:

    // NOTE: under lock
    time_t checkConnectionTimeout(smsc::core::network::Multiplexer::SockArray& error);

    // inline bool isTimedOut(smsc::core::network::Socket* s, time_t now);
    void removeSockets(smsc::core::network::Multiplexer::SockArray &error);

    // NOTE: must be locked
    void removeSocket( smsc::core::network::Socket *s );

    // bool idle() const;

protected:
    // IOTaskManager& iomanager_;
    smsc::logger::Logger* log_;
    smsc::core::synchronization::EventMonitor socketMonitor_;
    smsc::core::network::Multiplexer multiplexer_;
    SockArray working_; // is in work
    const uint16_t ioTimeout_;

private:
    const uint32_t connectionTimeout_;
    uint16_t checkTimeoutPeriod_;
    time_t lastCheckTime_;

    SockArray waiting_; // to be added to multiplexer
};


class MTPersReader: public IOTask 
{
public:
    MTPersReader( // IOTaskManager& iomanager,
                 uint32_t connectionTimeout,
                 uint16_t ioTimeout) : IOTask(// iomanager,
                                              connectionTimeout, ioTimeout, "reader") {}
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
    MTPersWriter(// IOTaskManager& iomanager,
                 uint32_t connectionTimeout,
                 uint16_t ioTimeout):IOTask(// iomanager,
                                            connectionTimeout, ioTimeout, "writer") {};
    virtual const char * taskName() { return "MTPersWriter"; }
protected:
    void processSockets(smsc::core::network::Multiplexer::SockArray &ready,
                        smsc::core::network::Multiplexer::SockArray &error,
                        time_t now);
    // void disconnectSocket(smsc::core::network::Socket *s);
    // void addSocket(smsc::core::network::Socket* s);
};

}//pvss
}//scag2

#endif

