#ifndef _INFORMER_MESSAGEGLOSSARY_H
#define _INFORMER_MESSAGEGLOSSARY_H

#include <list>
#include <vector>
#include "logger/Logger.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/buffers/IntHash.hpp"
#include "MessageText.h"
#include "informer/io/Typedefs.h"

namespace eyeline {
namespace informer {

class MessageGlossary
{
    class ChangeGuard;
    struct Node : public MessageText {
        Node( const char* text, unsigned id ) :
        MessageText(text) {
            id_ = id;
        }
    };

public:
    typedef std::list< Node > TextList;

    MessageGlossary();

    ~MessageGlossary();

    void init( dlvid_type dlvId );

    /// fetch message text from glossary.
    /// @param returnRealId - return with id replaced with real id.
    void fetchText( MessageText& text, bool returnRealId = false );

    /// NOTE: texts will be empty on exit
    void setTexts( const std::vector< std::string >& texts );

    void getTexts( std::vector< std::string >& texts ) const;

private:
    typedef smsc::core::buffers::IntHash< TextList::iterator > TextHash;
    void readGlossaryFailed( unsigned txtId, const char* msg );
    void cleanList( TextList& list );

private:
    smsc::logger::Logger*                              log_;
    mutable smsc::core::synchronization::EventMonitor  lock_;
    dlvid_type                                         dlvId_;
    TextList                                           list_; // owned
    TextHash*                                          hash_; // owned
    int32_t                                            lastRealId_;
    mutable bool                                       changing_;
};


}
}

#endif
