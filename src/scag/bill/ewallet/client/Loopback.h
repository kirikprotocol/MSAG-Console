#ifndef SCAG_BILL_EWALLET_CLIENT_LOOPBACK_H
#define SCAG_BILL_EWALLET_CLIENT_LOOPBACK_H

#include "scag/util/WatchedThreadedTask.h"
#include "core/synchronization/EventMonitor.hpp"
#include "scag/bill/ewallet/proto/SocketBase.h"

namespace scag2 {
namespace bill {
namespace ewallet {
namespace client {

class ClientContext;
class ClientCore;

/// an implementation
class Loopback : public proto::SocketBase
{
private:
    class Processor : public util::WatchedThreadedTask
    {
    public:
        Processor( Loopback& loopback ) : loopback_(&loopback) {}
        virtual const char* taskName() { return "ewall.lbp"; }
    protected:
        virtual int doExecute();
        void processBuffer( char* buffer, size_t buflen );
    private:
        Loopback* loopback_;
    };

public:
    Loopback( ClientCore& core );

    virtual ~Loopback();

    virtual bool isConnected() const;

    // start a processing thread
    Processor* getProcessor() { return processor_.get(); }

    // close socket and attached writer
    void close();

    // virtual const char* taskName() { return "ewall.loop"; }

    // virtual void shutdown();

    virtual void send( std::auto_ptr< proto::Context >& context, bool request ); // throw

protected:
    // virtual int doExecute();

protected:
    /// return context, may return 0
    WriteContext* getContext();

    // ClientCore&                                        core_;
    // mutable smsc::core::synchronization::EventMonitor  mon_;
    // smsc::logger::Logger*                              log_;

    // smsc::core::synchronization::EventMonitor          queueMon_;
    // std::auto_ptr< smsc::core::buffers::CyclicQueue<ClientContext*> >  queue_;
private:
    std::auto_ptr< Processor > processor_;
};

} // namespace client
} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_CLIENT_LOOPBACK_H */
