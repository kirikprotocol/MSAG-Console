#ifndef _INFORMER_MESSAGETEXT_H
#define _INFORMER_MESSAGETEXT_H

#include <cstring>
#include "util/int.h"

namespace eyeline {
namespace informer {

class MessageGlossary;
class MessageTextPtr;

static const unsigned MAX_ALLOWED_MESSAGE_LENGTH = 254;
static const unsigned MAX_ALLOWED_PAYLOAD_LENGTH = 65535;

/// message text.
class MessageText
{
    friend class MessageTextPtr;
    friend class MessageGlossary;
public:
    /// text is owned
    MessageText( const char* text = 0, int32_t id = 0 );
    ~MessageText(); // { if (text_) delete[] text_; }

    /// return 0 if non-glossary
    inline int32_t getTextId() const { return id_; }
    inline const char* getText() const { return text_; }

private:

    inline static char* copyText( const char* text ) {
        const size_t textlen = strlen(text)+1;
        char* ret = new char[textlen];
        memcpy(ret,text,textlen);
        return ret;
    }

private:
    MessageText( const MessageText& t );
    MessageText& operator = ( const MessageText& t );

private:
    const char*      text_;    // owned
    MessageGlossary* gloss_;   // ptr to glossary or 0
    int32_t          id_;      // text id (positive - anchored in glosary),
                               //          negative - dynamic in glossary
    int32_t          ref_;     // used when glossary is not 0
};


class MessageTextPtr
{
    friend class MessageGlossary;
public:
    MessageTextPtr( MessageText* ptr = 0 );
    MessageTextPtr( const MessageTextPtr& ptr );
    MessageTextPtr& operator = ( const MessageTextPtr& ptr );
    ~MessageTextPtr();
    void reset( MessageText* ptr );

    inline MessageText* get() { return ptr_; }
    inline const MessageText* get() const { return ptr_; }
    inline MessageText* operator -> () { return ptr_; }
    inline const MessageText* operator -> () const { return ptr_; }

private:
    MessageText* copy( MessageText* ptr );
    MessageText* ref( MessageText* ptr );
    void unref( MessageText* ptr );

private:
    MessageText* ptr_; // shared ownership
};

} // informer
} // smsc

#endif
