#ifndef _SCAG_TRANSPORT_SCAGCOMMAND2_H
#define _SCAG_TRANSPORT_SCAGCOMMAND2_H

#include "TransportType.h"

namespace scag {
namespace sessions {

    class Session2;
    class SessionStore2;
    
} // namespace sessions

namespace transport {

class SCAGCommand2
{
    friend class scag::sessions::SessionStore2;

private:
    SCAGCommand2( const SCAGCommand2& c );
    SCAGCommand2& operator = ( const SCAGCommand2& c );
public:
    // static const uint8_t SESSIONDESTROY = 255;

    virtual TransportType getType() const = 0;

    virtual int getServiceId() const = 0;
    virtual void setServiceId(int serviceId) = 0;

    virtual int64_t getOperationId() const = 0;
    virtual void setOperationId(int64_t op) = 0;

    virtual uint8_t getCommandId() const = 0;
      
    /*
    virtual SessionPtr getSession() = 0;
    virtual void setSession(const SessionPtr&) = 0;
    virtual bool hasSession() = 0;
     */

    virtual ~SCAGCommand2() {};

protected:
    /// session stuff is accessed from SessionStore to determine
    /// if the command has locked the session.
    virtual scag::sessions::Session2* getSession() = 0;
    virtual void setSession( scag::sessions::Session2* as ) = 0;

    SCAGCommand2() {}
};


/// The queue interface.
/// It is supposed that some commands may be kept into external sources,
/// but they still should be counted.  To achieve this goal method
/// pushCommand() accepts the second argument 'action', which
/// tells what to do with a command.
class SCAGCommandQueue
{
public:
    enum {
            RESERVE,    // reserve a place for a command (don't push)
            MOVE,       // move command from external source (push, but don't count)
            PUSH        // push command (push and count)
    };

    virtual ~SCAGCommandQueue() {}

    /// tell the queue to stop.
    /// The queue then stop accepting command.
    virtual void stop() = 0;

    /// check if the queue is stopped
    virtual bool isStopping() const = 0;

    /// push a command to a queue and return the resulting queue size.
    /// \param cmd -- command, \param action -- what to do (see above).
    /// return queue size after command is added to queue or
    ///   unsigned(-1) if cmd cannot be added.
    virtual unsigned pushCommand( scag::transport::SCAGCommand2* cmd,
                                  int action = PUSH ) = 0;

    /// return 0 if the queue is stopped
    virtual scag::transport::SCAGCommand2* popCommand() = 0;

};

} // namespace transport
} // namespace scag

#endif /* !_SCAG_TRANSPORT_SCAGCOMMAND2_H */
