#ifndef _SCAG_PVSS_CORE_SERVER_SERVER_H
#define _SCAG_PVSS_CORE_SERVER_SERVER_H

#include <memory>
#include "scag/pvss/common/PvssException.h"
#include "scag/pvss/api/packets/Request.h"
#include "scag/pvss/api/packets/Response.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

class ServerContext;
class SyncDispatcher;
class AsyncDispatcher;

/// interface
class Server
{
public:
    virtual ~Server() {}

    /**
     * Method starts PVSS Server up. Checks configuration.
     * Creates & starts: channel acceptor, readers & writers.
     *
     * @throws PvssException    Thrown if PVSS Server failed to start
     */
    virtual void startup( SyncDispatcher& dispatcher ) /* throw(PvssException) */  = 0;
    virtual void startup( AsyncDispatcher& dispatcher ) /* throw (PvssException) */  = 0;

    /**
     * Method shuts PVSS Server down.
     * Stops acceptor, shuts down readers & writers, closes opened channels.<p/>
     * Cancels all pending requests (if any), generates fail-responses for them.
     * Waits while all responses in outgoing writers queues will be sent or cancelled &
     * server logic will process all response states.
     */
    virtual void shutdown() = 0;


    class Logic
    {
    public:
        /**
         * Notifies server logic that PVAP response was successfuly sent.
         * Server logic may safely commit transaction.
         *
         * @param context   Contains processed PVAP request & sent PVAP response.
         */
        virtual void responseSent(std::auto_ptr<ServerContext> context) = 0;

        /**
         * Notifies server logic that server failed to send PVAP response.
         * Server logic should rollback transaction.
         *
         * @param context   Contains processed PVAP request & failed PVAP response.
         */
        virtual void responseFail(std::auto_ptr<ServerContext> context) = 0;
    };


    class AsyncLogic : public Logic
    {
    public:
        /**
         * Notifies server logic that new PVAP request was received and needs to be processed.
         * Request remains unprocessed until setResponse() or setError() method will be called.
         * These methods causes server to start response sending.<p/>
         * When processing timeout expires and request remains unprocessed than server generates fail-response,
         * attempt to call setResponse() or setError() method will cause exception.</p>
         * Server logic may choose to send response in any time later (e.g. from another thread).
         *
         * @param context   Contains unprocessed (new) PVAP request.
         */
        virtual void requestReceived(std::auto_ptr<ServerContext>& context ) /* throw (PvssException) */  = 0;

        /**
         * Notifies server logic that processing should be stopped and waits until it finishes.
         */
        virtual void shutdown() = 0;
    };


    /**
     * Interface used to interact with server logic in sync mode.
     */
    class SyncLogic : public Logic
    {
    public:
        /**
         * Notifies server logic that new PVAP request was received and needs to be processed.
         * Server expects that response will be returned from this method.
         *
         * @param request       Unprocessed (new) PVAP request.
         * @return              PVAP response generated by server logic for this request (ownership passed).
         * @throws Exception    May be thrown by server logic when it failed to process request.
         */
        virtual Response* process(Request& request) /* throw(PvssException) */  = 0;

    };


    /*
     * Method use calling thread for sync server logic processing.
     * Provided interface methods will be called from it on server events.
     * Blocks calling thread untill server will be shutted down.
     *
     * @param logic     Sync server logic mode.
     */
    // virtual void waitEvents(Dispatcher& logic) = 0;

    /*
     * Method use calling thread for async server logic processing.
     * Provided interface methods will be called from it on server events.
     * Blocks calling thread untill server will be shutted down.
     *
     * @param logic     Async server logic mode.
     */
    // virtual void waitEvents(AsyncLogic& logic) = 0;

};

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_SERVER_SERVER_H */
