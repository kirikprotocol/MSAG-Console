#ifndef _SCAG_SESSIONS_OPERATION_H
#define _SCAG_SESSIONS_OPERATION_H

#include "core/synchronization/Mutex.hpp"
#include "logger/Logger.h"
#include "Session2.h"
#include "scag/util/io/Serializer.h"

namespace scag2 {
namespace sessions {

using namespace smsc::core::synchronization;

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
// const uint32_t WAIT_RECEIPT                  = 0x10;
const uint32_t PERSISTENT                    = 0x20;
const uint32_t NEXTUSSDISSUBMIT              = 0x40; // if set next ussd should be submit
}


class Operation
{
public:
    inline static int32_t invalidUSSDref() { return -1; }
    inline static int32_t notsetUSSDref() { return -2; }

    Operation( Session* owner = 0,
               uint8_t type = transport::CO_NA,
               time_t ussdLastTime = 0 );
    ~Operation();

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

    /// set/get current context scope (0--invalid)
    void setContextScope( int ctx ) { ctxid_ = ctx; }
    int getContextScope() const { return ctxid_; }

    /// ussd reference number (-1 -- invalid, -2 -- pending).
    /// this field is only valid if optype is CO_USSD_DIALOG
    void setUSSDref( int32_t umr ) /* throw (exceptions::SCAGException) */;
    inline int32_t getUSSDref() const { return umr_; }

    time_t getUSSDLastTime() const { return ussdLastTime_; }
    void setUSSDLastTime( time_t ct ) { ussdLastTime_ = ct; }

    /// used for segmented messages
    int32_t getSARref() const;
    void setSARref( int32_t sar );
    const re::RuleStatus& getSARstatus() const;
    void setSARstatus( const re::RuleStatus& );

    const std::string* getKeywords() const {
        return keywords_;
    }
    void setKeywords( const std::string& kw ) {
        if ( keywords_ ) delete keywords_;
        if ( kw.empty() ) keywords_ = 0; 
        else keywords_ = new std::string( kw );
    }

    void print( util::Print& p, opid_type opid = invalidOpId() ) const;

    util::io::Serializer& serialize( util::io::Serializer& s ) const;
    util::io::Deserializer& deserialize( util::io::Deserializer& s ) /* throw (DeserializerException) */;

private:
    Operation& operator = ( const Operation& );
    Operation( const Operation& );

private:
    static smsc::logger::Logger* log_;
    static Mutex                 loggerMutex_;

private:
    struct Segmentation;

private:
    Session*            owner_;
    int                 receivedParts_;
    int                 receivedResps_;
    ICCOperationStatus  status_;
    uint8_t             type_;
    uint32_t            flags_;
    int32_t             ctxid_;
    int32_t             umr_;     // used for USSD
    time_t              ussdLastTime_; // not persistent
    std::string*        keywords_;
    Segmentation*       segmentation_; // owned
};

} // namespace sessions
} // namespace scag

#endif /* !_SCAG_SESSIONS_OPERATION_H */
