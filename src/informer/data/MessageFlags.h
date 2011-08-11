#ifndef _INFORMER_MESSAGEFLAGS_H
#define _INFORMER_MESSAGEFLAGS_H

#include "informer/io/Typedefs.h"
#include "informer/io/TmpBuf.h"

namespace smsc {
namespace sms {
struct Address;
}
}

namespace eyeline {
namespace informer {

class MessageTags
{
protected:
    enum {
        TAG_TRANSACTIONAL = 1,
        TAG_USEDATASM = 2,
        TAG_REPLACEIFPRESENT = 3,
        TAG_FLASH = 4,
        TAG_SVCTYPE = 5,
        TAG_SOURCEADDRESS = 6,
        TAG_DELIVERYMODE = 7,
        TAG_EXTRATLV = 8
    };
};


class MessageFlagBuilder : protected MessageTags
{
#define BIT(x) (uint64_t(1) << x)
    enum {
        BIT_TRANSACTIONAL = BIT(0),
        BIT_USEDATASM = BIT(1),
        BIT_REPLACEIFPRESENT = BIT(2),
        BIT_FLASH = BIT(3),
        BIT_SVCTYPE = BIT(4),
        BIT_SOURCEADDRESS = BIT(5),
        BIT_DELIVERYMODE = BIT(6),
        BIT_EYELINEKEYWORD = BIT(7)
    };
#undef BIT(x)

public:
    MessageFlagBuilder() : bitmask_(0) {}
    inline void setTransactional( bool value ) {
        setBool(value,TAG_TRANSACTIONAL,BIT_TRANSACTIONAL);
    }
    inline void setUseDataSm( bool value ) {
        setBool(value,TAG_USEDATASM,BIT_USEDATASM);
    }
    inline void setReplaceIfPresent( bool value ) {
        setBool(value,TAG_REPLACEIFPRESENT,BIT_REPLACEIFPRESENT);
    }
    inline void setFlash( bool value ) {
        setBool(value,TAG_FLASH,BIT_FLASH);
    }
    void setSvcType( const char* value );
    void setSourceAddress( const smsc::sms::Address& oa );
    void setDeliveryMode( DlvMode dlvMode );
    void setEyelineKeywordTLV( const char* kw );

    inline size_t getSize() const { return buf_.GetPos(); }
    inline const char* getBuf() const { return buf_.get(); }

protected:
    void setBool(bool value, uint16_t tag, uint64_t bit);
    // void cutField(uint16_t tag);

private:
    TmpBuf<char,256> buf_;
    uint64_t         bitmask_;
};


/// message text.
/// FIXME: these flags should be masked by/checked against user permissions.
class MessageFlags : protected MessageTags
{
public:
    MessageFlags( const char* hexdump = 0 );
    // explicit MessageFlags( MessageFlagBuilder& mfb );

    inline ~MessageFlags() {
        if (flags_) { delete flags_; }
    }

    inline void swap( MessageFlags& mf ) {
        std::swap(flags_,mf.flags_);
        std::swap(flagsize_,mf.flagsize_);
    }

    void reset( const MessageFlagBuilder& mfb );

    inline char* buf() const { return flags_; }
    inline size_t bufsize() const { return flagsize_; }
    inline bool isEmpty() const { return !flags_; }

    inline bool hasTransactional( bool& value ) const {
        return hasBool(value,TAG_TRANSACTIONAL); 
    }
    inline bool hasUseDataSm( bool& value ) const {
        return hasBool(value,TAG_USEDATASM);
    }
    inline bool hasReplaceIfPresent( bool& value ) const {
        return hasBool(value,TAG_REPLACEIFPRESENT);
    }
    bool hasFlash( bool& value ) const {
        return hasBool(value,TAG_FLASH);
    }
    const char* getSvcType() const;
    bool getSourceAddress( smsc::sms::Address& oa ) const;
    bool getDeliveryMode( DlvMode& dlvMode ) const;

    /// return extra tlv size (or 0), and set pointer to the start of it.
    size_t getExtraTLV( const char*& ptr ) const;
    
private:
    bool hasBool( bool& value, uint16_t tag ) const;

private:
    char*    flags_; // owned []
    unsigned flagsize_;
};

} // informer
} // smsc

#endif
