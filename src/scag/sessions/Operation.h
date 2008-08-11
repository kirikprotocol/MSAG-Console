#ifndef _SCAG_SESSIONS_OPERATION_H
#define _SCAG_SESSIONS_OPERATION_H

#include "core/synchronization/Mutex.hpp"
#include "logger/Logger.h"

namespace scag2 {
namespace sessions {

using namespace smsc::core::synchronization;

class Session;


typedef enum ICCOperationStatus
{
    OPERATION_INITED = 1,
    OPERATION_CONTINUED,
    OPERATION_COMPLETED
} ICCOperationStatus;


namespace OperationFlags
{
const uint32_t SERVICE_INITIATED_USSD_DIALOG = 1;
}


class Operation
{
public:
    Operation( Session* owner, uint8_t type );
    ~Operation() {}

    uint8_t type() const { return type_; }

    void receiveNewPart( int currentIndex, int lastIndex );
    void receiveNewResp( int currentIndex, int lastIndex );

    ICCOperationStatus getStatus() const { return status_; }
    void setStatus( ICCOperationStatus st ) { status_ = st; }
    const char* getNamedStatus() const;

    void setFlag( uint32_t f ) { flags_ |= f; }
    void clearFlag( uint32_t f ) { flags_ &= ~f; }
    bool flagSet( uint32_t f ) const { return flags_ & f; }
    uint32_t flags() const { return flags_; }

private:
    Operation& operator = ( const Operation& );
    Operation( const Operation& );

private:
    static smsc::logger::Logger* log_;
    static Mutex                 loggerMutex_;

private:
    Session*            owner_;
    int                 receivedParts_;
    bool                receivedAllParts_;
    int                 receivedResps_;
    bool                receivedAllResps_;
    ICCOperationStatus  status_;
    uint8_t             type_;
    uint32_t            flags_;

};

} // namespace sessions
} // namespace scag

#endif /* !_SCAG_SESSIONS_OPERATION_H */
