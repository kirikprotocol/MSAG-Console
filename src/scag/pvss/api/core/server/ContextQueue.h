#ifndef _SCAG_PVSS_CORE_SERVER_CONTEXTQUEUE_H
#define _SCAG_PVSS_CORE_SERVER_CONTEXTQUEUE_H

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

class ContextQueue
{
public:
    /// notify queue that a new request is received.
    void requestReceived(std::auto_ptr<ServerContext>& context) throw (PvssException);

    /// receive a new context (blocking), until a new non-expired context is arrived.
    ServerContext* getContext();

    /// fast check w/o locking
    int getSize();

    /// stop the queue
    void stop();

    /// check if queue is started.
    bool isStarted() const;

    /// startup the queue.
    void startup();

    /// notify the queue that it is shutdown.
    void shutdown();
};

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_SERVER_CONTEXTQUEUE_H */
