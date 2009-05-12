#ifndef _SCAG_PVSS_COMMON_STATUSTYPE_H
#define _SCAG_PVSS_COMMON_STATUSTYPE_H

#include "util/int.h"

namespace scag2 {
namespace pvss {

///
/// Response status
///
class StatusType 
{
public:
    // NOTE: those with '//exc' were in PvssException, those with '//both' were in both
    enum Type {

            // business logics messages (infos?): range [0x0..0x0f]
            OK                  = 0x00,
            PROPERTY_NOT_FOUND  = 0x01,
            TYPE_INCONSISTENCE  = 0x02,
            INVALID_KEY         = 0x03, // exc
            INVALID_SCOPE       = 0x04, // exc

            // protocol messages (warnings): range [0x10..0x1f]
            BAD_REQUEST         = 0x10, // both
            BAD_RESPONSE        = 0x11, // exc
            UNKNOWN_RESPONSE    = 0x12, // exc
            UNEXPECTED_RESPONSE = 0x13, // exc

            // generic severe failures (errors?): range [0x20..0x2f]
            IO_ERROR           = 0x20,  // exc
            NOT_SUPPORTED      = 0x21,
            SERVER_SHUTDOWN    = 0x22,
            SERVER_BUSY        = 0x23,  // both
            REQUEST_TIMEOUT    = 0x24,  // both
            CONNECT_FAILED     = 0x25,  // exc
            NOT_CONNECTED      = 0x26,  // exc
            CLIENT_BUSY        = 0x27,  // exc
            CONFIG_INVALID     = 0x28,  // exc

            // unknown
            UNKNOWN            = 0xff
    };

    static const char* statusToString( uint8_t stat );
    static const char* statusMessage( uint8_t stat );

    static inline bool statusIsInfo( uint8_t stat ) {
        return (stat & 0xf0) == 0x00;
    }
    static inline bool statusIsWarn( uint8_t stat ) {
        return (stat & 0xf0) == 0x10;
    }
    static inline bool statusIsError( uint8_t stat ) {
        return (stat & 0xf0) == 0x20;
    }

protected:
    /// NOTE: they are all protected to prevent nasty things
    StatusType() {}
    StatusType( const StatusType& ) {}
    StatusType& operator == ( const StatusType& ) { return *this; }
    ~StatusType() throw () {}
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_COMMON_STATUSTYPE_H */
