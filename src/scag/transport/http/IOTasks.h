#ifndef SCAG_TRANSPORT_IOTASKS
#define SCAG_TRANSPORT_IOTASKS

#include "core/synchronization/EventMonitor.hpp"
#include "core/network/Socket.hpp"
#include "core/network/Multiplexer.hpp"
#include "TaskList.h"

namespace scag { namespace transport { namespace http
{
using smsc::core::synchronization::EventMonitor;
using smsc::core::network::Socket;
using smsc::core::network::Multiplexer;

class HttpManager;
class HttpContext;

class IOTask : public IOTaskParent {
public:
    IOTask(HttpManager& m, IOTaskManager& iom, const int timeout);

    unsigned int getSocketCount() {
        return socketCount;
    }
    virtual void registerContext(HttpContext* cx) = 0;    
    virtual void stop();

protected:
    class HttpMultiplexer : public Multiplexer {
    public:
        unsigned int Count() {
            return sockets.Count();
        }
        Socket *getSocket(unsigned int i) {
            return sockets[i];
        }
    };
    
    inline bool isTimedOut(Socket* s, time_t now);
    void killSocket(Socket *s);
    void removeSocket(Multiplexer::SockArray &error);
    inline void removeSocket(Socket *s);
    inline static void deleteSocket(Socket *s, char *buf, int how);
     
    HttpMultiplexer multiplexer;
    EventMonitor sockMon;
    HttpManager &manager;
    IOTaskManager &iomanager;
    int connectionTimeout;
};

class HttpReaderTask : public IOTask {
public:
    HttpReaderTask(HttpManager& m, IOTaskManager& iom, const int timeout) :
        IOTask(m, iom, timeout) {}

    virtual int Execute();
    virtual const char* taskName();
    virtual void registerContext(HttpContext* cx);

protected:
    inline void addSocket(Socket* s);
};

class HttpWriterTask : public IOTask {
public:
    HttpWriterTask(HttpManager& m, IOTaskManager& iom, const int timeout) :
        IOTask(m, iom, timeout) {}
        
    virtual int Execute();
    virtual const char* taskName();
    virtual void registerContext(HttpContext* cx);

protected:
    inline void addSocket(Socket* s, bool connected);

    Array<Socket*> waitingConnect;
};

}}}

#endif // SCAG_TRANSPORT_IOTASKS
