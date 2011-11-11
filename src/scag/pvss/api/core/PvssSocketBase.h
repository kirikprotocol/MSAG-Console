#ifndef _SCAG_PVSS_CORE_PVSSSOCKETBASE_H
#define _SCAG_PVSS_CORE_PVSSSOCKETBASE_H

#include "informer/io/EmbedRefPtr.h"
#include "logger/Logger.h"
#include "core/synchronization/Mutex.hpp"
#include "core/network/Socket.hpp"

namespace scag2 {
namespace pvss {
namespace core {

class PvssSocket;

class PvssSocketBase
{
    friend class eyeline::informer::EmbedRefPtr< PvssSocketBase >;

protected:
    static smsc::logger::Logger*                      log_;

protected:
    PvssSocketBase( smsc::core::network::Socket* sock,
                    const char* host = 0,
                    unsigned port = 0 ) :
    sock_(sock), ref_(0), host_(host?host:""), port_(port) 
    {
        if (sock_) sock_->setData(0,this);
        initHostName();
        smsc_log_debug(log_,"ctor %p sock=%p %s:%u",this,sock_,host_.c_str(),unsigned(port_));
    }

    void initHostName( const char* hn = 0 );

public:

    virtual ~PvssSocketBase() {
        smsc_log_debug(log_,"dtor %p sock=%p %s:%u",this,sock_,host_.c_str(),unsigned(port_));
        if (sock_) { delete sock_; }
    }
    inline smsc::core::network::Socket* getSocket() const { return sock_; }

protected:
    void ref()
    {
        unsigned r;
        {
            smsc::core::synchronization::MutexGuard mg(reflock_);
            r = ++ref_;
        }
        smsc_log_debug(log_,"ref+1=%u %p sock=%p%c %s:%u",r,this,sock_,
                       (sock_?sock_->isConnected():false)?'+':'-',
                       host_.c_str(),unsigned(port_));
    }

    void unref()
    {
        unsigned r;
        {
            smsc::core::synchronization::MutexGuard mg(reflock_);
            r = --ref_;
            smsc_log_debug(log_,"ref-1=%u %p sock=%p%c %s:%u",r,this,sock_,
                           (sock_?sock_->isConnected():false)?'+':'-',
                           host_.c_str(),unsigned(port_));
            if (r) return;
        }
        delete this;
    }

private:
    PvssSocketBase( const PvssSocketBase& );
    PvssSocketBase& operator = ( const PvssSocketBase& );

private:
    smsc::core::network::Socket*         sock_;
    smsc::core::synchronization::Mutex   reflock_;
    unsigned                             ref_;

protected:
    std::string                   host_;
    unsigned                      port_;
};

typedef eyeline::informer::EmbedRefPtr< PvssSocketBase > PvssSockPtr;

}
}
}

#endif
