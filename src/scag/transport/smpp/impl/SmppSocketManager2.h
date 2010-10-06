#ifndef __SCAG_TRANSPORT_SMPP_SMPPSOCKETMANAGER2_H__
#define __SCAG_TRANSPORT_SMPP_SMPPSOCKETMANAGER2_H__

#include "SmeAcceptor2.h"
#include "scag/transport/smpp/base/SmppChannelRegistrator2.h"
#include "scag/transport/smpp/base/SmppCommandQueue2.h"
#include "SmppReader2.h"
#include "scag/transport/smpp/base/SmppSMInterface2.h"
#include "SmppWriter2.h"
#include "SmscConnector2.h"
#include "core/threads/ThreadPool.hpp"
#include "logger/Logger.h"
#include "scag/config/base/ConfigManager2.h"

namespace scag2 {
namespace transport {
namespace smpp{

    using namespace scag2::config;

class SmppSocketManager : public SmppSMInterface
{
private:

    class IpLimit {
    public:
        IpLimit() : connected_(0), lastFailure_(0) {}
        inline unsigned connectionCount() const { return connected_; }
        inline time_t lastFailure() const { return lastFailure_; }
        inline unsigned addConnection() { return ++connected_; }
        inline unsigned removeConnection() { return --connected_; }
        void setLastFailure( time_t t ) { lastFailure_ = t; }
    private:
        unsigned connected_;
        time_t   lastFailure_;
    };
    
public:
    SmppSocketManager(SmppChannelRegistrator* argReg,SmppCommandQueue* argQueue):
    acc(0), conn(0),
    reg(argReg),
    queue(argQueue),
    log(0),
    // readerCount_(0),
    registeredConnections_(0),
    socketsPerThread_(16),
    bindTimeout_(10),
    connectionsPerIp_(100),
    failTimeout_(60),
    maxReaderCount_(10)
  {
    acc=new SmeAcceptor(this);
    //acc->Init("0.0.0.0",8001);
    acc->Init(
              ConfigManager::Instance().getConfig()->getString("smpp.host"),
              ConfigManager::Instance().getConfig()->getInt("smpp.port")
    );
    conn=new SmscConnector(this);
    conn->Init(
               ConfigManager::Instance().getConfig()->getString("smpp.host")
    );
      tp.startTask( acc );
      tp.setMaxThreads( maxReaderCount_*2 + 1 );
    log=smsc::logger::Logger::getInstance("smpp.sock");
  }

    void init( unsigned socketsPerThread,
               unsigned bindTimeout,
               unsigned connectionsPerIp,
               unsigned failTimeout,
               unsigned maxReaderCount )
    {
        MutexGuard mg(mtx);
        socketsPerThread_ = socketsPerThread;
        bindTimeout_ = bindTimeout;
        connectionsPerIp_ = connectionsPerIp;
        failTimeout_ = failTimeout;
        if ( maxReaderCount < maxReaderCount_ ) {
            smsc_log_error( log, "cannot lower a max number of readers: %u", maxReaderCount_ );
        } else {
            maxReaderCount_ = maxReaderCount;
            tp.setMaxThreads( 2*maxReaderCount_ + 1 );
        }
        smsc_log_info(log,"SocketManager::init sockPerThread=%u bindTimeout=%u connPerIp=%u failTimeout=%u maxRWCount=%u",
                      socketsPerThread_, bindTimeout_, connectionsPerIp_, failTimeout_, maxReaderCount_ );
    }

    /// add whitelisted ip address
    void addWhiteIp( const char* dottedaddr );

  SmscConnectorAdmin* getSmscConnectorAdmin()
  {
    return conn;
  }
    virtual bool registerSocket(SmppSocket* sock);
    virtual void unregisterSocket(SmppSocket* sock);
    void shutdown();

    unsigned bindTimeout() const { return bindTimeout_; }

private:
    typedef IntHash< IpLimit > IphashType;

protected:
    // enum {MaxSocketsPerThread=16};
  Array<SmppReader*> readers;
  Array<SmppWriter*> writers;
  sync::Mutex mtx;

  SmeAcceptor* acc;
  SmscConnector* conn;

  SmppChannelRegistrator* reg;
  SmppCommandQueue* queue;

  smsc::logger::Logger* log;

  thr::ThreadPool tp;

    sync::Mutex        statMutex_;
    unsigned           registeredConnections_;
    IphashType         iphash_;
    IntHash< uint8_t > whiteList_;

    // limits
    unsigned socketsPerThread_;     // max number of sockets per multiplexer thread
    unsigned bindTimeout_;          // max time a socket is allowed to be in unbound state
    unsigned connectionsPerIp_;     // max number of connections per ip
    unsigned failTimeout_;          // min time an IP is blocked after failure
    unsigned maxReaderCount_;  // max number of multiplexer threads
};


}//smpp
}//transport
}//scag


#endif
