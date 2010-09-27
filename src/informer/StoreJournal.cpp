#include "StoreJournal.h"
#include "Message.h"
#include "core/buffers/TmpBuf.hpp"
#include "informer/io/HexDump.h"
#include "informer/io/IOConverter.h"

namespace eyeline {
namespace informer {

void StoreJournal::journalMessage( dlvid_type     dlvId,
                                   regionid_type  regionId,
                                   const Message& msg )
{
    smsc::core::buffers::TmpBuf<unsigned char,200> buf;
    if (msg.text.get() && msg.text->getTextId()<=0) {
        buf.setSize(200+strlen(msg.text->getText()));
    }
    io::ToBuf tb(buf.get());
    tb.set32(dlvId);
    tb.set32(regionId);
    const unsigned buflen = unsigned(msg.toBuf(version_,tb.getPtr()) - buf.get());
    if (log_->isDebugEnabled()) {
        io::HexDump hd;
        io::HexDump::string_type dump;
        dump.reserve(buflen*5);
        hd.hexdump(dump,buf.get(),buflen);
        hd.utfdump(dump,buf.get(),buflen);
        smsc_log_debug(log_,"buffer to save(%u): %s",buflen,hd.c_str(dump));
    }
    // FIXME: save buffer
}

}
}
