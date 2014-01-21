#ifndef _EYELINE_SMPP_PDU_TYPES_H
#define _EYELINE_SMPP_PDU_TYPES_H

#include "eyeline/smpp/Status.h"
#include "eyeline/smpp/BindMode.h"

namespace eyeline {
namespace smpp {

enum PduType {
        GENERIC_NACK                 = 0x80000000,
        BIND_RECEIVER                = 0x00000001,
        BIND_RECEIVER_RESP           = 0x80000001,
        BIND_TRANSMITTER             = 0x00000002,
        BIND_TRANSMITTER_RESP        = 0x80000002,
        QUERY_SM                     = 0x00000003,
        QUERY_SM_RESP                = 0x80000003,
        SUBMIT_SM                    = 0x00000004,
        SUBMIT_SM_RESP               = 0x80000004,
        DELIVER_SM                   = 0x00000005,
        DELIVER_SM_RESP              = 0x80000005,
        UNBIND                       = 0x00000006,
        UNBIND_RESP                  = 0x80000006,
        REPLACE_SM                   = 0x00000007,
        REPLACE_SM_RESP              = 0x80000007,
        CANCEL_SM                    = 0x00000008,
        CANCEL_SM_RESP               = 0x80000008,
        BIND_TRANSCEIVER             = 0x00000009,
        BIND_TRANSCEIVER_RESP        = 0x80000009,
        OUTBIND                      = 0x0000000b,
        ENQUIRE_LINK                 = 0x00000015,
        ENQUIRE_LINK_RESP            = 0x80000015,
        SUBMIT_MULTI                 = 0x00000021,
        SUBMIT_MULTI_RESP            = 0x80000021,
        ALERT_NOTIFICATION           = 0x00000102,
        DATA_SM                      = 0x00000103,
        DATA_SM_RESP                 = 0x80000103
};

const char* pduTypeToString( PduType pt );

/// return pdutype corresponding to @param bm or GENERIC_NACK
PduType bindModeToPduType( BindMode bm );

/// return bindmode corresponding to @param pt or BindMode(0)
BindMode bindPduTypeToMode( PduType pt );

/// return pdutype of resp corresponding to @param pt or PduType(0)
PduType pduTypeToResp( PduType pt );

}
}

#endif