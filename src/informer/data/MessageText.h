#ifndef _INFORMER_MESSAGETEXT_H
#define _INFORMER_MESSAGETEXT_H

#include <string.h>
#include "util/int.h"

namespace eyeline {
namespace informer {

static const unsigned MAX_ALLOWED_MESSAGE_LENGTH = 254;
static const unsigned MAX_ALLOWED_PAYLOAD_LENGTH = 65535;

/// message text.
class MessageText
{
public:
    /// a special text id which delimits input ids and real ids.
    /// >uniqueId are glossary text ids visible to users/clients;
    /// =uniqueId - non-glossary text id, i.e. the text is unique;
    /// <uniqueId are internal glossary text ids not visible to users/clients.
    static const int32_t uniqueId = -1;

    /// text is owned
    inline explicit MessageText( const char* text = 0, int32_t id = uniqueId ) :
    text_( id != uniqueId ? text : copyText(text)), id_(id) {}

    inline ~MessageText() {
        if (id_ == uniqueId && text_) delete [] text_;
    }

    inline MessageText( const MessageText& txt ) :
    text_( txt.id_ != uniqueId ? txt.text_ : copyText(txt.text_)), id_(txt.id_) {}

    inline MessageText& operator = ( const MessageText& t ) {
        if (t.text_ != text_) {
            if (id_ == uniqueId && text_) delete [] text_;
            id_ = t.id_;
            text_ = t.id_ != uniqueId ? t.text_ : copyText(t.text_);
        }
        return *this;
    }

    inline void swap( MessageText& t ) {
        std::swap(id_,t.id_);
        std::swap(text_,t.text_);
    }

    /// return 0 if non-glossary
    inline int32_t getTextId() const { return id_; }
    inline const char* getText() const { return text_; }

    /// copy text, create char[] buffer via new
    inline static char* copyText( const char* text ) {
        if (!text) return 0;
        const size_t textlen( ::strlen(text) + 1 );
        char* ret = new char[textlen];
        ::memcpy(ret,text,textlen);
        return ret;
    }

protected:
    const char*      text_;    // owned
    int32_t          id_;      // text id (positive - anchored in glosary),
};

} // informer
} // smsc

#endif
