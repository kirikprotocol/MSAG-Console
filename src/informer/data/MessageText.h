#ifndef _INFORMER_MESSAGETEXT_H
#define _INFORMER_MESSAGETEXT_H

#include <cstring>
#include "util/int.h"

namespace eyeline {
namespace informer {

class MessageTextPtr;

class Glossary 
{
    const char* getText( uint32_t id ) const { return "FIXME"; }
};


/// message text.
/// FIXME: hige optimization required.
class MessageText
{
    friend class MessageTextPtr;
public:
    /// text is owned
    MessageText( const char* text = 0, int32_t id = 0 );
    inline ~MessageText() { if (text_) delete[] text_; }

    /// return 0 if non-glossary
    inline uint32_t getTextId() const { return id_; }
    inline const char* getText() const { return text_; }

    // reset message, ref count resets to 1.
    /*
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
     */

    // merge two message texts, used to merge
    // freshly loaded message and glossary message.
    // void merge( MessageText& t );

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
    const char* text_;    // owned
    Glossary*   gloss_;   // ptr to glossary or 0
    int32_t     id_;      // text id (positive - anchored in glosary),
                          //          negative - dynamic in glossary
    int32_t    ref_;      // used when glossary is not 0
};


class MessageTextPtr
{
public:
    MessageTextPtr( MessageText* ptr = 0 );
    MessageTextPtr( MessageTextPtr& ptr );
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
