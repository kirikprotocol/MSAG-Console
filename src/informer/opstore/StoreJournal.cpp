#include "StoreJournal.h"
#include "informer/data/Message.h"
#include "core/buffers/TmpBuf.hpp"
#include "informer/io/HexDump.h"
#include "informer/io/IOConverter.h"

namespace eyeline {
namespace informer {

StoreJournal::StoreJournal( const std::string& path ) :
log_(smsc::logger::Logger::getInstance("storelog")),
path_(path),
version_(1)
{
    // FIXME: read store journal
    fg_.create((path_ + "operative/.journal").c_str(),true,true);
}


void StoreJournal::journalMessage( dlvid_type     dlvId,
                                   regionid_type  regionId,
                                   const Message& msg )
{
    smsc::core::buffers::TmpBuf<unsigned char,200> buf;
    if (msg.isTextUnique()) {
        buf.setSize(90+strlen(msg.text->getText()));
    }
    ToBuf tb(buf.get(),buf.getSize());
    tb.skip(4);
    tb.set32(dlvId);
    tb.set32(regionId);
    const unsigned buflen = unsigned(msg.toBuf(version_,tb).buf - buf.get());
    tb.setPos(0);
    tb.set32(buflen-4);
    if (log_->isDebugEnabled()) {
        HexDump hd;
        HexDump::string_type dump;
        dump.reserve(buflen*5);
        hd.hexdump(dump,buf.get(),buflen);
        hd.strdump(dump,buf.get(),buflen);
        smsc_log_debug(log_,"buffer to save(%u): %s",buflen,hd.c_str(dump));
    }
    fg_.write(buf.get(),buflen);
}

}
}
