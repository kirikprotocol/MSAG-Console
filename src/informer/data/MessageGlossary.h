#ifndef _INFORMER_MESSAGEGLOSSARY_H
#define _INFORMER_MESSAGEGLOSSARY_H

#include <list>
#include "logger/Logger.h"
#include "core/synchronization/Mutex.hpp"
#include "core/buffers/IntHash.hpp"
#include "MessageText.h"

namespace eyeline {
namespace informer {

class InputMessageSource;

class MessageGlossary
{
    friend class MessageTextPtr;
public:
    typedef std::list< MessageText* > TextList;
    MessageGlossary( InputMessageSource& ims );

    /// bind message to glossary.
    void bindMessage( MessageTextPtr& ptr );

    /// NOTE: texts will be empty on exit
    void registerMessages( InputMessageSource&  ims,
                           TextList&            texts );

private:
    struct Node {
        Node() {}
        Node(TextList::iterator i,TextList::iterator r) : iter(i), repl(r) {}
        TextList::iterator iter;
        TextList::iterator repl;
    };
    typedef smsc::core::buffers::IntHash< Node > TextHash;

    void registerFailed( TextList& texts, TextList::iterator upto );
    void ref( MessageText* ptr );
    void unref( MessageText* ptr );

private:
    smsc::logger::Logger*                              log_;
    smsc::core::synchronization::Mutex                 lock_;
    TextList                                           list_; // owned
    TextHash                                           hash_;
    int32_t                                            negMsgId_;
    int32_t                                            posMsgId_;
};


}
}

#endif
