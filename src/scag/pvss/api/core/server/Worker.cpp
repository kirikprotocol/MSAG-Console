#include <memory>
#include "Worker.h"
#include "ServerContext.h"
#include "ServerConfig.h"
#include "ServerCore.h"
#include "core/buffers/File.hpp"

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
        context.reset( queue_.getContext(100) );
        if ( ! context.get() ) {
            if ( ! queue_.couldHaveRequests() ) stop();
            if ( ! queue_.isStarted() ) break;
            logic_.keepAlive();
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
            do {
                util::msectime_type currentTime = util::currentTimeMillis();
                try {
                    if ( currentTime  > context->getCreationTime() + core_.getConfig().getProcessTimeout() ) {
                        context->getRequest()->timingMark("expired");
                        throw PvssException(PvssException::REQUEST_TIMEOUT,"processing timeout");
                    }
                } catch (PvssException& e){
                    smsc_log_debug(log_,"proc tmo in context: %s", context->getRequest()->toString().c_str() );
                    core_.countExceptions(e.getType(),"workProcTmo");
                    try {
                        context->setError(e.getMessage(),e.getType());
                    } catch (...) {}
                    break;
                }
                
                try {
                    context->getRequest()->timingMark("inQueue");
                    Response* resp = logic_.process(*context->getRequest().get());
                    if ( context->getRequest()->hasTiming() ) {
                        context->getRequest()->timingMark("respgot");
                        if (resp) {
                            resp->startTiming(*context->getRequest().get());
                        }
                    }
                    context->setResponse(resp);
                } catch ( smsc::core::buffers::FileException& e ) {
                    smsc_log_fatal(log_,"FileExc in process: %s, SIGTERM will follow", e.what());
                    core_.countExceptions(PvssException::UNKNOWN,"procFileExc");
                    try {
                        context->setError(e.what(),Response::UNKNOWN);
                    } catch (...) {}
                    kill(getpid(),SIGTERM);
                    break;
                } catch (PvssException& e) {
                    smsc_log_debug(log_,"exc in process: %s", context->getRequest()->toString().c_str());
                    context->getRequest()->timingMark("procFail");
                    core_.countExceptions(e.getType(),"procFailed");
                    try {
                        context->setError(e.getMessage(),e.getType());
                    } catch (...) {}
                    break;
                }
            } while ( false );

            core_.contextProcessed(context); // it will come back to the queue after sending response
            if ( !queue_.couldHaveRequests() ) {
                // send notification that this worker should stop
                stop();
            }
            break;
        case (ServerContext::PROCESSED) :
        default :
            smsc_log_debug(log_,"unexpected context state (processed)", context->getState());
            core_.countExceptions(PvssException::UNKNOWN,"workBadState");
        } // switch
    }
    return 0;
}

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2
