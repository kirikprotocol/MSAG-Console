#ifndef _SCAG_PROTOTYPES_INFOSME_INPUTSTORAGE_HPP
#define _SCAG_PROTOTYPES_INFOSME_INPUTSTORAGE_HPP

#include <ctime>
#include "Message.h"

namespace scag2 {
namespace prototypes {
namespace infosme {

class InputStorage
{
public:
    virtual ~InputStorage() {}

    /// get the next message for given region.
    /// the message may be not ready.
    /// @return true if the message was found.
    virtual bool getNextMessage( time_t curTime, int region, Message& msg ) = 0;
    
    /// mark the message as processed.
    /// This method is invoked at the start of infosme to set up the correct
    /// message id.
    virtual void messageIsProcessed( int region, int32_t msgId ) = 0;

    /// add a new message to the storage
    /// @return a new message id.
    virtual int32_t addMessage( Message& msg ) = 0;
};

}
}
}

#endif /* !_SCAG_PROTOTYPES_INFOSME_INPUTSTORAGE_HPP */
