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
    void init( msgid_type minRlast );
    bool isMessageDropped( msgid_type msgid );
    bool addMessages( std::vector<msgid_type>& dropped );

private:
    typedef smsc::core::buffers::IntHash64<uint64_t> DropMsgHash;
    typedef smsc::core::buffers::TmpBuf<char, 2048> Buf;

    size_t openFile( FileGuard& fg, msgid_type minRlast, bool* isNew = 0 );
    size_t readBuf( FileGuard& fg, Buf& buf );
    void readChunk( FileGuard& fg );
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
