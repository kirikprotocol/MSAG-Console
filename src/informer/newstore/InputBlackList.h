#ifndef _INFORMER_INPUTBLACKSTORAGE_H
#define _INFORMER_INPUTBLACKSTORAGE_H

#include "InputStorage.h"
#include "core/buffers/IntHash64.hpp"

namespace eyeline {
namespace informer {

/// persistent input storage.
class InputStorage::BlackList
{
    static const size_t itemsize = 8;
    static const size_t packsize = 64;
    static const uint64_t packbits[packsize];

public:
    BlackList( InputStorage& is );

    /// initialize with given minimal read msgid.
    void init( msgid_type minRlast );

    /// check if message is dropped.
    /// the message id is removed from hash.
    bool isMessageDropped( msgid_type msgid );

    /// add messages to dropped list.
    bool addMessages( std::vector<msgid_type>& dropped );

private:
    typedef smsc::core::buffers::IntHash64<uint64_t> DropMsgHash;
    typedef TmpBuf<char,2048> Buf;

    /// open black list file and position it after given message id.
    /// @param minRlast - msgid after which file should be seeked.
    /// @param isNew - fills with true if filename .new was opened.
    /// @return the size of the file.
    size_t openFile( FileGuard& fg, msgid_type minRlast, bool* isNew = 0 );

    /// read a chunk of items into buffer.
    /// @param return the number of bytes read.
    size_t readBuf( FileGuard& fg, TmpBufBase<char>& buf );

    /// read the next chunk from the file and fills the hash.
    /// @return true if the chunk has been read.
    bool readChunk( FileGuard& fg, TmpBufBase<char>& buf );

    /// writing activity log 'skip' records
    void writeActLog( unsigned sleepTime );

private:
    InputStorage& is_;
    smsc::core::synchronization::EventMonitor dropMon_;
    DropMsgHash                               dropMsgHash_;
    size_t                                    dropFileOffset_; // offset of the next read
    msgid_type                                minMsgId_; // minimal id in hash
    msgid_type                                maxMsgId_; // maximal id in hash
    bool                                      changing_; // true when change in progress
};

} // informer
} // smsc

#endif
