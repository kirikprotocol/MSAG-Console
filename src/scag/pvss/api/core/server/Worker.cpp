#include <memory>
#include "Worker.h"
#include "ServerContext.h"
#include "ServerConfig.h"
#include "ServerCore.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

int Worker::Execute()
{
    while ( queue_.isStarted() ) {
        std::auto_ptr<ServerContext> context(queue_.getContext());
        if ( ! context.get() ) continue;
        context->setWorker(*this);
        try {
            context->setResponse(logic_.process(*context->getRequest().get()));
        } catch (PvssException& e){
            context->setError(e.getMessage());
        }
        core_.contextProcessed(context); // it will come back to the worker after sending response
    }
    return 0;
}

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2
