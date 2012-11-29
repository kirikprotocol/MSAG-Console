#include "Types.h"

namespace eyeline {
namespace smpp {

const char* pduTypeToString( PduType pt )


{
#define PTTOSTRING(x) case(x) : return #x
    switch (pt) {
        PTTOSTRING(GENERIC_NACK);
        PTTOSTRING(BIND_RECEIVER);
        PTTOSTRING(BIND_RECEIVER_RESP);
        PTTOSTRING(BIND_TRANSMITTER);
        PTTOSTRING(BIND_TRANSMITTER_RESP);
        PTTOSTRING(QUERY_SM);
        PTTOSTRING(QUERY_SM_RESP);
        PTTOSTRING(SUBMIT_SM);
        PTTOSTRING(SUBMIT_SM_RESP);
        PTTOSTRING(DELIVER_SM);
        PTTOSTRING(DELIVER_SM_RESP);
        PTTOSTRING(UNBIND);
        PTTOSTRING(UNBIND_RESP);
        PTTOSTRING(REPLACE_SM);
        PTTOSTRING(REPLACE_SM_RESP);
        PTTOSTRING(CANCEL_SM);
        PTTOSTRING(CANCEL_SM_RESP);
        PTTOSTRING(BIND_TRANSCEIVER);
        PTTOSTRING(BIND_TRANSCEIVER_RESP);
        PTTOSTRING(OUTBIND);
        PTTOSTRING(ENQUIRE_LINK);
        PTTOSTRING(ENQUIRE_LINK_RESP);
        PTTOSTRING(SUBMIT_MULTI);
        PTTOSTRING(SUBMIT_MULTI_RESP);
        PTTOSTRING(ALERT_NOTIFICATION);
        PTTOSTRING(DATA_SM);
        PTTOSTRING(DATA_SM_RESP);
#undef PTTOSTRING
    default: return "???";
    }
}


PduType bindModeToPduType( BindMode bm )
{
    switch(bm) {
    case(BINDMODE_RECEIVER) : return BIND_RECEIVER;
    case (BINDMODE_TRANSMITTER) : return BIND_TRANSMITTER;
    case(BINDMODE_TRANSCEIVER) : return BIND_TRANSCEIVER;
    default : return GENERIC_NACK;
    }
}


BindMode bindPduTypeToMode( PduType pt )
{
    switch (pt) {
    case(BIND_RECEIVER) : return BINDMODE_RECEIVER;
    case (BIND_TRANSMITTER) : return BINDMODE_TRANSMITTER;
    case(BIND_TRANSCEIVER) : return BINDMODE_TRANSCEIVER;
    default : return BindMode(0);
    }
}

PduType pduTypeToResp( PduType pt )
{
    if ((pt & 0x80000000)) return PduType(0);
    switch (pt) {
    case OUTBIND : return PduType(0);
    case ALERT_NOTIFICATION: return PduType(0);
    default:
        return PduType(pt + 0x80000000);
    }
}

}
}
