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
const uint32_t RECEIVED_ALL_PARTS            = 2;
const uint32_t RECEIVED_ALL_RESPS            = 4;
const uint32_t RECEIVED_ALL                  = RECEIVED_ALL_PARTS | RECEIVED_ALL_RESPS;
const uint32_t WAIT_RECEIPT                  = 0x10;
}


class Operation
{
public:
    Operation( Session* owner, uint8_t type );
    ~Operation() {}

    uint8_t type() const { return type_; }

    int parts() const { return receivedParts_; }
    int resps() const { return receivedResps_; }

    void receiveNewPart( int currentIndex, int lastIndex );
    void receiveNewResp( int currentIndex, int lastIndex );

    ICCOperationStatus getStatus() const { return status_; }
    void setStatus( ICCOperationStatus st ) { status_ = st; }
    const char* getNamedStatus() const;

    inline void setFlag( uint32_t f ) { flags_ |= f; }
    inline void clearFlag( uint32_t f ) { flags_ &= ~f; }
    inline bool flagSet( uint32_t f ) const { return (flags_ & f); }
    inline uint32_t flags() const { return flags_; }

private:
    Operation& operator = ( const Operation& );
    Operation( const Operation& );

private:
    static smsc::logger::Logger* log_;
    static Mutex                 loggerMutex_;

private:
    Session*            owner_;
    int                 receivedParts_;
    int                 receivedResps_;
    ICCOperationStatus  status_;
    uint8_t             type_;
    uint32_t            flags_;

};

} // namespace sessions
} // namespace scag

#endif /* !_SCAG_SESSIONS_OPERATION_H */
