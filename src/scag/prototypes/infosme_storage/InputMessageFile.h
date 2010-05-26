#ifndef _SCAG_PROTOTYPES_INFOSME_INPUTMESSAGEFILE_H
#define _SCAG_PROTOTYPES_INFOSME_INPUTMESSAGEFILE_H

#include "Message.h"
#include "DataFile.h"
#include "core/buffers/CyclicQueue.hpp"

namespace scag2 {
namespace prototypes {
namespace infosme {


/// cached input from input data file.
/// The format of the input file:
/// vvxxxyyyzzz,79537699490,textid,userDatauserDatauserData
///
/// The file may be:
///  1. being read
///  2. being updated
class InputMessageFile
{
private:
    static const unsigned MAXINPUTMESSAGELEN = 64;
    static const unsigned BUFLEN = 512;
public:
    InputMessageFile( const char* path );

    /// @return the unique id of the file.
    uint32_t getId() const;

    /// get the next message.
    /// @param msg is filled with next message, file pointer is moved forward.
    /// @return  0 if there are no more messages;
    ///         -1 if messages are not ready;
    ///         >0 on success.
    int getNextMessage( PosMessage& msg );

    /// check if there are more messages
    bool hasMessages() const;

private:
    bool readRecord(Message& pm, const char* buf, size_t buflen);

private:
    char        buf_[MAXINPUTMESSAGELEN];  // the rest of the input buffer
    smsc::core::buffers::CyclicQueue<PosMessage> cache_;
    DataFile    df_;
    unsigned    buflen_;
};

}
}
}

#endif /* !_SCAG_PROTOTYPES_INFOSME_MESSAGEFILE_H */
