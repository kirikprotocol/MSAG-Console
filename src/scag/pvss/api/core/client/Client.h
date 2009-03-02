#ifndef _SCAG_PVSS_CORE_CLIENT_CLIENT_H
#define _SCAG_PVSS_CORE_CLIENT_CLIENT_H

#include <memory>
#include "scag/pvss/common/PvssException.h"
#include "scag/pvss/api/packets/Request.h"
#include "scag/pvss/api/packets/Response.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace client {

/// interface
class Client
{
public:
    virtual ~Client() {}

    /**
     * Method starts PVSS Client up. Checks configuration.
     * Creates & starts: channel connector, readers & writers.
     *
     * @throws PvssException    Thrown if PVSS Client failed to start
     */
    virtual void startup() throw(PvssException) = 0;

    /**
     * Method shuts PVSS Client down.
     * Stops connector, shuts down readers & writers, closes opened channels.<p/>
     * Waits while all requests in outgoing writers queues will be sent or cancelled &
     * notifies client logic.
     */
    virtual void shutdown() = 0;

    /**
     * Interface contains callback methods used by PVSS Client to notify client logic.
     * Methods called when PVSS Client receives response or produces error for accepted PVAP request.
     * Used as external event handler for async mode PVAP requests processing.
     */
    class ResponseHandler
    {
    public:
        virtual ~ResponseHandler() {}

        /**
         * External handler called when PVSS Client has received response on sent PVAP request.
         *
         * @param request   Original sent request
         * @param response  Response received from PVSS server
         */
        virtual void handleResponse(std::auto_ptr<Request> request, std::auto_ptr<Response> response) = 0;

        /**
         * External handler called when PVSS Client failed to process accepted PVAP request.
         * Request is accepted after PVSS Client enqueued it. It is possible that request wasn't sent yet.
         *
         * @param exc       Exception occurred on processing accepted PVAP request
         * @param request   Original accepted request
         */
        virtual void handleError(const PvssException& exc, std::auto_ptr<Request> request) = 0;

    };

    /**
     * Sends PVAP request and waits PVAP response (sync mode).
     * Method blocks calling thread until response came or error will be occured.
     * However another thread may process diffrent call in same time on this Client.
     *
     * @param request           PVAP request command to send to PVSS server
     * @return                  PVAP response command on success
     * @throws PvssException    Thrown when client failed to accept or process request
     */
    virtual std::auto_ptr<Response> processRequestSync(std::auto_ptr<Request>& request) throw(PvssException) = 0;

    /**
     * Send PVAP request and register callback handler to wait PVAP response (async mode).
     * Outgoing request will be enqueued in client (accepted) and provided handler will be registered for it.
     * This handler will be notified when response came or error be occurred for accepted request.
     *
     * @param request           PVAP request command to send to PVSS server
     * @param handler           Callback handler to notify on response or error for this request
     * @throws PvssException    Thrown when client rejects request processing (e.g. not inited/connected, queue is full)
     */
    virtual void processRequestAsync( std::auto_ptr<Request>& request,
                                      ResponseHandler& handler ) throw(PvssException) = 0;
};

} // namespace client
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_CLIENT_CLIENT_H */
