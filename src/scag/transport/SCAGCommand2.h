#ifndef _SCAG_TRANSPORT_SCAGCOMMAND2_H
#define _SCAG_TRANSPORT_SCAGCOMMAND2_H

#include "scag/util/io/Print.h"
#include "OpId.h"
#include "TransportType.h"

namespace scag2 {
namespace sessions {

    class Session;
    class SessionStore;
    
} // namespace sessions

namespace transport {

using namespace scag::transport;

class SCAGCommand
{
    friend class scag2::sessions::SessionStore;

public:
    // static const uint8_t SESSIONDESTROY = 255;

    virtual TransportType getType() const = 0;

    virtual int getServiceId() const = 0;
    virtual void setServiceId(int serviceId) = 0;

    // initially set to invalidOpId()
    virtual opid_type getOperationId() const = 0;
    virtual void setOperationId( opid_type op) = 0;
    /*
    static inline opid_type invalidOpId() {
        return 0;
    }
     */

    virtual uint8_t getCommandId() const = 0;
      
    virtual uint32_t getSerial() const = 0;

    /*
    virtual SessionPtr getSession() = 0;
    virtual void setSession(const SessionPtr&) = 0;
    virtual bool hasSession() = 0;
     */

    virtual ~SCAGCommand() {};

    virtual scag2::sessions::Session* getSession() = 0;

    /// print-out the command (for debugging)
    virtual void print( util::Print& p ) const = 0;

protected:
    /// session is set from session store when the command is locking the session.
    virtual void setSession( scag2::sessions::Session* as ) = 0;

protected:
    SCAGCommand() {}
    SCAGCommand( const SCAGCommand& ) {}

    /// make serial, please use this method in subclasses to obtain the serial.
    /// 0   -- no serial
    /// <10 -- reserved
    static uint32_t makeSerial();

private:
    SCAGCommand& operator = ( const SCAGCommand& c );
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

    // tell the queue to stop.
    // The queue then stop accepting command.
    // virtual void stop() = 0;

    // check if the queue is stopped
    // virtual bool isStopping() const = 0;

    /// push a command to a queue and return the resulting queue size.
    /// \param cmd -- command, \param action -- what to do (see above).
    /// return queue size after command is added to queue or
    ///   unsigned(-1) if cmd cannot be added.
    virtual unsigned pushCommand( SCAGCommand* cmd,
                                  int action = PUSH ) = 0;

    // return 0 if the queue is stopped
    // virtual SCAGCommand* popCommand() = 0;

};

} // namespace transport2
} // namespace scag

#endif /* !_SCAG_TRANSPORT_SCAGCOMMAND2_H */
