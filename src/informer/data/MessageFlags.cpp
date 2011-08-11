#include <string.h>
#include "MessageFlags.h"
#include "informer/io/IOConverter.h"
#include "informer/io/InfosmeException.h"
#include "sms/sms.h"

namespace eyeline {
namespace informer {

namespace {
class MessageFlagParser
{
public:
    MessageFlagParser( const char* buf, size_t bufsize ) :
    fb_(buf,bufsize) {}
        
    bool findTag( uint16_t tag, size_t* pos = 0) {
        fb_.setPos(0);
        while ( fb_.getPos() < fb_.getLen() ) {
            if (pos) { *pos = fb_.getPos(); }
            if ( fb_.getc16() == tag ) return true;
            fb_.skip(fb_.getc16());
        }
        return false;
    }

    uint16_t getLen() {
        return fb_.getc16();
    }

    inline size_t getPos() const { return fb_.getPos(); }

    // NOTE: must be invoked after findTag
    uint8_t getInt8() {
        fb_.getc16();
        return fb_.get8();
    }

    const char* getCString() {
        fb_.getc16(); // skip length
        return fb_.getCString();
    }

    const char* getOString( size_t& len ) {
        len = fb_.getc16();
        return reinterpret_cast<const char*>(fb_.skip(len));
    }

private:
    FromBuf fb_;
};
}


#define SETBIT(x) if ((bitmask_&x)) { \
    throw InfosmeException(EXC_LOGICERROR,"bit %llu is already set in mfb",x); \
} else bitmask_ |= x


void MessageFlagBuilder::setBool(bool value, uint16_t tag, uint64_t bit)
{
    SETBIT(bit);
    /*
    MessageFlagParser mp(buf_.get(),buf_.GetPos());
    if (mp.findTag(tag)) {
        mp.getLen();
        buf_.get()[mp.getPos()] = char(value ? 1 : 0);
        return;
    }
     */
    buf_.reserve(buf_.GetPos()+10);
    ToBuf tb(buf_.GetCurPtr(),buf_.getSize()-buf_.GetPos());
    tb.setc16(tag);
    tb.setc16(1);
    tb.set8(value ? 1 : 0);
    buf_.SetPos(buf_.GetPos()+tb.getPos());
}

/*
void MessageFlagBuilder::cutField(uint16_t tag)
{
    MessageFlagParser mp(buf_.get(),buf_.GetPos());
    size_t pos;
    if (mp.findTag(tag,&pos)) {
        const uint16_t len = mp.getLen();
        char* o = buf_.get() + pos;
        const char* i = buf_.get() + mp.getPos() + len;
        const char* e = buf_.GetCurPtr();
        for ( ; i != e; ++i, ++o ) {
            *o = *i;
        }
        buf_.SetPos( o - buf_.get() );
    }
}
 */


void MessageFlagBuilder::setSvcType( const char* value )
{
    if (!value || !value[0]) {
        throw InfosmeException(EXC_LOGICERROR,"empty svctype in mfb");
    }
    const size_t vallen = strlen(value);
    if (vallen >= DLV_SVCTYPE_LENGTH) {
        throw InfosmeException(EXC_LOGICERROR,"too long svctype in mfb");
    }
    SETBIT(BIT_SVCTYPE);
    // cutField(TAG_SVCTYPE);
    buf_.reserve(buf_.GetPos()+10+vallen);
    ToBuf tb(buf_.GetCurPtr(),buf_.getSize()-buf_.GetPos());
    tb.setc16(TAG_SVCTYPE);
    tb.setc16(uint16_t(vallen+1));
    tb.setCString(value);
    buf_.SetPos(buf_.GetPos()+tb.getPos());
}


void MessageFlagBuilder::setSourceAddress( const smsc::sms::Address& oa )
{
    char oabuf[32];
    oa.toString(oabuf,sizeof(oabuf));
    const size_t oalen = strlen(oabuf);
    SETBIT(BIT_SOURCEADDRESS);
    // cutField(TAG_SOURCEADDRESS);
    buf_.reserve(buf_.GetPos()+10+oalen);
    ToBuf tb(buf_.GetCurPtr(),buf_.getSize()-buf_.GetPos());
    tb.setc16(TAG_SOURCEADDRESS);
    tb.setc16(uint16_t(oalen+1));
    tb.setCString(oabuf);
    buf_.SetPos(buf_.GetPos()+tb.getPos());
}


void MessageFlagBuilder::setDeliveryMode( DlvMode dlvMode )
{
    // cutField(TAG_DELIVERYMODE);
    SETBIT(BIT_DELIVERYMODE);
    buf_.reserve(buf_.GetPos()+10);
    ToBuf tb(buf_.GetCurPtr(),buf_.getSize()-buf_.GetPos());
    tb.setc16(TAG_DELIVERYMODE);
    tb.setc16(1);
    tb.set8(uint8_t(dlvMode));
    buf_.SetPos(buf_.GetPos()+tb.getPos());
}


void MessageFlagBuilder::setEyelineKeywordTLV( const char* value )
{
    if (!value || !value[0]) {
        throw InfosmeException(EXC_LOGICERROR,"empty eyelinekeyword in mfb");
    }
    const size_t vallen = strlen(value);
    if (vallen >= MSG_EYELINEKEYWORD_LENGTH) {
        throw InfosmeException(EXC_LOGICERROR,"too long eyelinekeyword in mfb");
    }
    SETBIT(BIT_EYELINEKEYWORD);
    MessageFlagParser mp(buf_.get(),buf_.GetPos());
    size_t pos;
    if (mp.findTag(TAG_EXTRATLV,&pos)) {
        const uint16_t len = mp.getLen();
        if (mp.getPos()+len < buf_.getPos()) {
            // there is a tail, move it
            // copy tlvs into temporary buf
            TmpBuf<char,512> tmpbuf;
            tmpbuf.append(buf_.get()+mp.getPos(),len);
            // move the tail
            char* o = buf_.get() + pos;
            const char* i = buf_.get() + mp.getPos() + len;
            const char* e = buf_.GetCurPtr();
            for ( ; i != e; ++i, ++o ) {
                *o = *i;
            }
            // append the buffer
            buf_.setPos(o - buf_.get());
            tmpbuf.reserve(tmpbuf.getPos()+vallen+10);
            ToBuf tb(tmpbuf.GetCurPtr(),tmpbuf.getSize()-tmpbuf.GetPos());
            tb.set16(0x4906);
            tb.set16(vallen+1);
            tb.setCString(value);
            buf_.reserve( buf_.getPos() + tmpbuf.getPos() + 10 );
            ToBuf tb2(buf_.getCurPtr(),buf_.getSize()-buf_.getPos());
            tb2.setc16(TAG_EXTRATLV);
            tb2.setc16(tmpbuf.getPos());
            buf_.setPos(buf_.GetPos()+tb2.getPos());
            buf_.append(tmpbuf.get(),tmpbuf.getPos());
        }
    } else {
        buf_.reserve(buf_.getPos()+vallen+10);
        ToBuf tb(buf_.GetCurPtr(),buf_.getSize()-buf_.GetPos());
        tb.setc16(TAG_EXTRATLV);
        tb.setc16(uint16_t(vallen+1+2+2));
        tb.set16(0x4906);
        tb.set16(vallen+1);
        tb.setCString(value);
        buf_.SetPos(buf_.GetPos()+tb.getPos());
    }
}


MessageFlags::MessageFlags( const char* hexdump ) :
flags_(0), flagsize_(0)
{
    if (!hexdump || !hexdump[0]) {
        return;
    }
    const size_t len = strlen(hexdump);
    if ( (len & 1) != 0 ) {
        throw InfosmeException(EXC_IOERROR,"bad msgflags (not even)");
    }
    flagsize_ = unsigned(len / 2);
    flags_ = new char[flagsize_];
    for ( char* o = flags_; *hexdump != '\0' ; ++o ) {
        uint8_t res;
        const char vh = uint8_t(*hexdump++);
        if ( vh >= '0' && vh <= '9' ) {
            res = (vh-'0');
        } else if ( (vh >= 'a' && vh <= 'f') ||
                    (vh >= 'A' && vh <= 'f') ) {
            res = (vh & 0x7) + 9;
        } else {
            delete flags_;
            flags_ = 0;
            flagsize_ = 0;
            throw InfosmeException(EXC_IOERROR,"bad msgflags (not a digit)");
        }
        res <<= 4;
        uint8_t vl = uint8_t(*hexdump++);
        if ( vl >= '0' && vl <= '9' ) {
            res += (vl-'0');
        } else if ( (vl >= 'a' && vl <= 'f') ||
                    (vl >= 'A' && vl <= 'F') ) {
            res += (vl & 0x7) + 9;
        } else {
            delete flags_;
            flags_ = 0;
            flagsize_ = 0;
            throw InfosmeException(EXC_IOERROR,"bad msgflags (not a digit)");
        }
        *o++ = char(res);
    }
}


void MessageFlags::reset( const MessageFlagBuilder& mfb )
{
    if (flags_) { delete flags_; }
    flagsize_ = unsigned(mfb.getSize());
    if ( !flagsize_ ) {
        flags_ = 0;
        return;
    }
    flags_ = new char[flagsize_];
    memcpy(flags_,mfb.getBuf(),flagsize_);
}


bool MessageFlags::hasBool( bool& value, uint16_t tag ) const
{
    if (!flags_) return false;
    MessageFlagParser mp(flags_,flagsize_);
    if (!mp.findTag(tag)) return false;
    value = mp.getInt8();
    return true;
}


const char* MessageFlags::getSvcType() const
{
    if (!flags_) return 0;
    MessageFlagParser mp(flags_,flagsize_);
    if (!mp.findTag(TAG_SVCTYPE)) return 0;
    return mp.getCString();
}


bool MessageFlags::getSourceAddress( smsc::sms::Address& oa ) const
{
    if (!flags_) return false;
    MessageFlagParser mp(flags_,flagsize_);
    if (!mp.findTag(TAG_SOURCEADDRESS)) return false;
    try {
        oa = smsc::sms::Address(mp.getCString());
    } catch ( std::exception& e ) {
        return false;
    }
    return true;
}


bool MessageFlags::getDeliveryMode( DlvMode& dlvMode ) const
{
    if (!flags_) return false;
    MessageFlagParser mp(flags_,flagsize_);
    if (!mp.findTag(TAG_DELIVERYMODE)) return false;
    dlvMode = DlvMode(mp.getInt8());
    return true;
}


size_t MessageFlags::getExtraTLV( const char*& ptr ) const
{
    if (!flags_) return 0;
    MessageFlagParser mp(flags_,flagsize_);
    if (!mp.findTag(TAG_EXTRATLV)) return false;
    size_t len;
    ptr = mp.getOString(len);
    return len;
}

}
}
