#ifndef SCAG_TRANSPORT_HTTP_IMPL_IOTASKS
#define SCAG_TRANSPORT_HTTP_IMPL_IOTASKS

#include "core/synchronization/EventMonitor.hpp"
#include "core/network/Socket.hpp"
#include "core/network/Multiplexer.hpp"
#include "logger/Logger.h"
#include "TaskList.h"

namespace scag2 {
namespace transport {
namespace http {

using smsc::core::synchronization::EventMonitor;
using smsc::core::network::Socket;
using smsc::core::network::Multiplexer;
using smsc::logger::Logger;

class HttpManagerImpl;
class HttpContext;

class IOTask : public IOTaskParent 
{

public:
    IOTask(HttpManagerImpl& m, IOTaskManager& iom, const int timeout);

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
        const pollfd* getFds(unsigned int i) {
        	return &fds[i];
        }
    };

    
    void checkConnectionTimeout(Multiplexer::SockArray& error);
    inline bool isTimedOut(Socket* s, time_t now);
    void killSocket(Socket *s);
    void removeSocket(Multiplexer::SockArray &error);
    inline void removeSocket(Socket *s);
//    inline static void deleteSocket(Socket *s, int how);
    inline static void clearSocket(Socket *s);
     
    HttpMultiplexer multiplexer;
    EventMonitor sockMon;
    HttpManagerImpl &manager;
    IOTaskManager &iomanager;
    Logger *logger;    
    int connectionTimeout;
    smsc::core::buffers::Array<Socket*> waitingAdd;
    inline void addSocket(Socket* s, bool connected);
};

class HttpReaderTask : public IOTask {
public:
    HttpReaderTask(HttpManagerImpl& m, IOTaskManager& iom, const int timeout);

    virtual int Execute();
    virtual const char* taskName();
    virtual void registerContext(HttpContext* cx);
};

class HttpWriterTask : public IOTask {
public:
    HttpWriterTask(HttpManagerImpl& m, IOTaskManager& iom, const int timeout);
        
    virtual int Execute();
    virtual const char* taskName();
    virtual void registerContext(HttpContext* cx);

protected:
    smsc::core::buffers::Array<Socket*> waitingConnect;
};

}}}

#endif // SCAG_TRANSPORT_IOTASKS
