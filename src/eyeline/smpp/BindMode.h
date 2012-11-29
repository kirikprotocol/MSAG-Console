#ifndef _EYELINE_SMPP_BINDMODE_H
#define _EYELINE_SMPP_BINDMODE_H

namespace eyeline {
namespace smpp {

enum BindMode {
        BINDMODE_RECEIVER    = 1, // RX
        BINDMODE_TRANSMITTER = 2, // TX
        BINDMODE_TRANSRECV   = 3, // TXRX
        BINDMODE_TRANSCEIVER = 4  // TRX
};

inline const char* bindModeToString( BindMode bm )
{
    switch(bm) {
    case (BINDMODE_RECEIVER)    : return "RX";
    case (BINDMODE_TRANSMITTER) : return "TX";
    case (BINDMODE_TRANSRECV)   : return "TXRX";
    case (BINDMODE_TRANSCEIVER) : return "TRX";
    default : return "???";
    }
}

inline bool bindModeIsValid( BindMode bm )
{
    return ( bm >= BINDMODE_RECEIVER &&
             bm <= BINDMODE_TRANSCEIVER );
}

}
}

#endif
