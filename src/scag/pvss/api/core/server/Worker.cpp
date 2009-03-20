#include <memory>
#include "Worker.h"
#include "ServerContext.h"
#include "ServerConfig.h"
#include "ServerCore.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

Worker::Worker( Server::SyncLogic& logic, ServerCore& core ) :
WorkerThread(core,taskName()),
logic_(logic)
{}


Worker::~Worker()
{
    shutdown();
}


void Worker::shutdown()
{
    queue_.shutdown();
    waitUntilReleased();
}


int Worker::doExecute()
{
    std::auto_ptr<ServerContext> context;
    while ( true ) {
        context.reset( queue_.getContext() );
        if ( ! context.get() ) {
            if ( ! queue_.couldHaveRequests() ) stop();
            if ( ! queue_.isStarted() ) break;
            continue;
        }
        switch (context->getState()) {
        case (ServerContext::SENT) :
            logic_.responseSent( context );
            break;
        case (ServerContext::FAILED) :
            logic_.responseFail( context );
            break;
        case (ServerContext::NEW) :
            try {
                // FIXME: check request expiration
                Response* resp = logic_.process(*context->getRequest().get());
                context->setResponse(resp);
            } catch (PvssException& e){
                context->setError(e.getMessage());
            }
            core_.contextProcessed(context); // it will come back to the queue after sending response
            if ( !queue_.couldHaveRequests() ) {
                // send notification that this worker should stop
                stop();
            }
            break;
        case (ServerContext::PROCESSED) :
        default :
            smsc_log_warn(log_,"unexpected context state: %d", context->getState());
        } // switch
    }
    return 0;
}

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2
