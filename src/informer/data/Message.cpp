#include "Message.h"
#include "informer/io/InfosmeException.h"
#include "informer/io/IOConverter.h"
#include <cstdio>

namespace eyeline {
namespace informer {

ToBuf& Message::toBuf( uint16_t version, ToBuf& tb ) const
{
    if ( version != 1 ) {
        throw InfosmeException("wrong version %u in Message::toBuf", version);
    }
    tb.set64(subscriber);
    tb.set64(msgId);
    tb.set32(lastTime);
    tb.set32(timeLeft);
    tb.copy(USERDATA_LENGTH,userData.c_str());
    if (isTextUnique()) {
        tb.set8(state | 0x80);
        tb.setCString(text->getText());
    } else {
        tb.set8(state & 0x7f);
        tb.set32(text->getTextId());
    }
    return tb;
}


FromBuf& Message::fromBuf( uint16_t version, FromBuf& tb )
{
    if ( version != 1 ) {
        throw InfosmeException("wrong version %u in Message::fromBuf", version);
    }
    subscriber = tb.get64();
    msgId = tb.get64();
    lastTime = tb.get32();
    timeLeft = tb.get32();
    userData = reinterpret_cast<const char*>(tb.skip(USERDATA_LENGTH));
    state = tb.get8();
    if (state & 0x80) {
        state &= 0x7f;
        text.reset(new MessageText(tb.getCString(),0));
    } else {
        // FIXME: optimize if textptr already has glossary!
        text.reset(new MessageText(0,tb.get32()));
    }
    return tb;
}


char* makeDeliveryPath( dlvid_type dlvId, char* buf )
{
    const unsigned chunk = unsigned(dlvId/100)*100;
    const int rv = sprintf(buf,"deliveries/%010u/%u/",chunk,dlvId);
    if (rv<0) {
        throw InfosmeException("cannot form delivery path D=%u, chunk=%u",dlvId,chunk);
    }
    return buf + rv;
}


}
}
