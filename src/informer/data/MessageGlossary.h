#ifndef _INFORMER_MESSAGEGLOSSARY_H
#define _INFORMER_MESSAGEGLOSSARY_H

#include <list>
#include "logger/Logger.h"
#include "core/synchronization/Mutex.hpp"
#include "core/buffers/IntHash.hpp"
#include "MessageText.h"

namespace eyeline {
namespace informer {

class MessageGlossary
{
    friend class MessageTextPtr;
public:
    MessageGlossary();

    /// bind message to glossary.
    void bindMessage( MessageTextPtr& ptr );

private:
    typedef std::list< MessageText* >                          TextList;
    typedef smsc::core::buffers::IntHash< TextList::iterator > TextHash;

    void ref( MessageText* ptr );
    void unref( MessageText* ptr );

private:
    smsc::logger::Logger*                              log_;
    smsc::core::synchronization::Mutex                 lock_;
    TextList                                           list_; // owned
    TextHash                                           hash_;
    int32_t                                            lastMsgId_;
};


}
}

#endif
