#ifndef _INFORMER_MESSAGETEXT_H
#define _INFORMER_MESSAGETEXT_H

#include <cstring>

namespace smsc {
namespace informer {

/// message text
class MessageText
{
public:
    MessageText( const char* text = 0, uint32_t id = 0 ) :
    text_(0), textId_(0), ref_(0) {
        resetText(text,id);
    }
    // MessageText( const MessageText& t );
    // MessageText& operator = ( const MessageText& t );
    ~MessageText() {
        // FIXME
        delete[] text_;
    }

    /// return 0 if non-glossary
    uint32_t getTextId() const { return textId_; }
    const char* getText() const { return text_; }

    /// reset message, ref count resets to 1.
    void resetText( const char* text, uint32_t textId ) {
        // FIXME: sync
        delete[] text_;
        textId_ = textId;
        if (text) {
            const size_t textlen = strlen(text)+1;
            text_ = new char[textlen];
            memcpy(text_,text,textlen);
        } else {
            text_ = 0;
        }
        ref_ = 1;
    }

    /// merge two message texts, used to merge
    /// freshly loaded message and glossary message.
    void merge( MessageText& t );

private:
    char*    text_;
    uint32_t textId_;    // used in glossary (unique)
    uint32_t ref_;
};

} // informer
} // smsc

#endif
