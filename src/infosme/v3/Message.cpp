#include "Message.h"
#include "InfosmeException.h"
#include "infosme/v3/io/IOConverter.h"
#include <cstdio>

namespace smsc {
namespace infosme {

unsigned char* Message::toBuf( uint16_t version, unsigned char* buf )
{
    if ( version != 1 ) {
        throw InfosmeException("wrong version %u in Message::toBuf", version);
    }
    io::ToBuf tb(buf);
    tb.set64(subscriber);
    tb.set32(msgId);
    tb.set32(lastTime);
    tb.set32(timeLeft);
    tb.set32(textId);
    tb.copy(USERDATA_LENGTH,userData.c_str());
    tb.set8(state);
    return tb.buf;
}


const unsigned char* Message::fromBuf( uint16_t version, const unsigned char* buf )
{
    if ( version != 1 ) {
        throw InfosmeException("wrong version %u in Message::fromBuf", version);
    }
    io::FromBuf tb(buf);
    subscriber = tb.get64();
    msgId = tb.get32();
    lastTime = tb.get32();
    timeLeft = tb.get32();
    textId = tb.get32();
    userData = reinterpret_cast<const char*>(tb.skip(USERDATA_LENGTH));
    state = tb.get8();
    return tb.buf;
}


char* Message::printToBuf( size_t bufsize, char* buf )
{
    char lbuf[20];
    char tbuf[20];
    if ( state == MsgState::input ) {
        // new message
        sprintf(lbuf,"%u",unsigned(lastTime));
        sprintf(tbuf,"%u",unsigned(timeLeft));
    } else {
        formatMsgTime(lbuf,lastTime);
        formatMsgTime(tbuf,timeLeft);
    }
    register int printed = snprintf( buf, bufsize,
                                     "%llu,%u,%s,%s,%u,%s,%s",
                                     static_cast<unsigned long long>(subscriber),
                                     unsigned(msgId),
                                     lbuf, tbuf,
                                     unsigned(textId),userData.c_str(),
                                     MsgState::toString(state));
    if ( printed < 0 ) {
        throw InfosmeException("snprintf failed: %d",printed);
    }
    return buf + (printed < int(bufsize) ? printed : bufsize-1);
}


}
}
