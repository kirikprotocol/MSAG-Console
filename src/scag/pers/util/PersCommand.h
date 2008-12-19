#ifndef _SCAG_PERS_UTIL_PERSCOMMAND_H
#define _SCAG_PERS_UTIL_PERSCOMMAND_H

#include <vector>
#include "scag/util/storage/SerialBuffer.h"
#include "PersClientException.h"
#include "Types.h"
#include "Property.h"

namespace scag2 {
namespace pers {
namespace util {

using scag2::util::storage::SerialBuffer;

class PersCommandSingle;
class PersCommandBatch;

/// an interface to a perscmd
/// Command           IN                OUT
/// PC_DEL            propname          bool
/// PC_SET            property          bool
/// PC_GET            propname          property
/// PC_INC            ---deprecated---
/// PC_INC_MOD        mod+property      int
/// PC_PING           none              bool
/// PC_BATCH          ---deprecated---
/// PC_TRANSACT_BATCH ---deprecated---
/// PC_INC_RESULT     property          int
/// PC_MTBATCH        various           bool
/// PC_BIND_ASYNCH    none              bool
class PersCommand
{
public:
    inline PersCmd cmdType() const { return cmdType_; }
    inline void setType( PersCmd cmdtype ) { cmdType_ = cmdtype; }
    virtual ~PersCommand() {}

    /// fill serial buffer (w/o cmdType) and return the status
    virtual int fillSB( SerialBuffer& sb ) = 0;

    /// read from SB and return the status
    virtual int readSB( SerialBuffer& sb ) = 0;

    // the status of the last action
    inline int status() const { return status_; }
    inline int setStatus( int stat ) { return status_ = int8_t(stat); }
    virtual int failIndex() const { return 0; }

    virtual PersCommandSingle* castSingle() { return 0; }
    virtual PersCommandBatch* castBatch() { return 0; }

protected:
    PersCommand() : cmdType_(PC_UNKNOWN), status_(0) {}
    PersCommand( PersCmd cmdtype ) : cmdType_(cmdtype), status_(0) {}
    // default is ok
    // PersCommand( const PersCommand& );
    // PersCommand& operator = ( const PersCommand& );

protected:
    PersCmd                           cmdType_;  // the type of pers cmd

private:
    int8_t                            status_;   // the status of the last action
};



class PersCommandSingle : public PersCommand
{
public:
    PersCommandSingle() : PersCommand() {}
    PersCommandSingle( PersCmd cmdtype ) : PersCommand( cmdtype ) {}
    Property& property() { return property_; }
    int32_t result() const { return result_; }
    void setResult( int32_t res ) { result_ = res; }
    virtual int fillSB( SerialBuffer& sb );
    virtual int readSB( SerialBuffer& sb );
    virtual PersCommandSingle* castSingle() { return this; }
protected:
    int readStatus( SerialBuffer& sb );
private:
    Property  property_;
    int32_t   result_;
};


class PersCommandBatch : public PersCommand
{
public:
    PersCommandBatch( const std::vector< PersCommandSingle >& cmds,
                      bool trans ) :
    PersCommand(PC_MTBATCH), batch_(cmds), transact_(trans), index_(0) {}
    virtual ~PersCommandBatch() {}
    virtual int fillSB( SerialBuffer& sb );
    virtual int readSB( SerialBuffer& sb );
    virtual int failIndex() const { return index_; }
    virtual PersCommandBatch* castBatch() { return this; }

protected:
    inline int setStatus( int stat ) { return PersCommand::setStatus(stat); }
    inline int setStatus( int stat, int idx ) { index_ = idx; return setStatus(stat); }

private:
    std::vector< PersCommandSingle > batch_;
    bool                             transact_;
    int                              index_;
};

}
}
}

#endif /* !_SCAG_PERS_UTIL_PERSCOMMAND_H */
