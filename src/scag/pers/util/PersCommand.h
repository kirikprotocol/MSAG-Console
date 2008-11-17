#ifndef _SCAG_PERS_UTIL_PERSCOMMAND_H
#define _SCAG_PERS_UTIL_PERSCOMMAND_H

#include <vector>
#include "scag/util/storage/SerialBuffer.h"
#include "PersClientException.h"
#include "Types.h"
#include "Property.h"

namespace scag2 {

namespace re {
namespace actions {
class ActionContext;
}
}

using util::storage::SerialBuffer;

namespace pers {
namespace util {

class PersCommand;

/// This class is a link from a pers command to a RE action.
/// Use it as a mixin-iface.
class PersCommandCreator
{
public:
    virtual PersCmd cmdType() const = 0;
    virtual void storeResults( re::actions::ActionContext& ctx, PersCommand& cmd ) = 0;
};


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
    virtual ~PersCommand() {}

    /// fill serial buffer (w/o cmdType) and return the status
    virtual int fillSB( SerialBuffer& sb ) = 0;

    /// read from SB and return the status
    virtual int readSB( SerialBuffer& sb ) = 0;

    /// store results of command processing via command creator
    virtual void storeResults( re::actions::ActionContext& ctx ) {
        creator_->storeResults( ctx, *this );
    }

    // the status of the last action
    inline int status() const { return status_; }
    inline int setStatus( int stat ) { return status_ = int8_t(stat); }

    virtual int failIndex() const { return 0; }

protected:
    PersCommand( PersCommandCreator& c, PersCmd cmd ) : creator_(&c), cmdType_(cmd), status_(0) {}
    PersCommand() : creator_(0), cmdType_(PC_UNKNOWN), status_(0) {}
    // default is ok
    // PersCommand( const PersCommand& );
    // PersCommand& operator = ( const PersCommand& );

protected:
    PersCommandCreator*               creator_;
    PersCmd                           cmdType_;  // the type of pers cmd

private:
    int8_t                            status_;   // the status of the last action
};



class PersCommandSingle : public PersCommand
{
public:
    PersCommandSingle() : PersCommand() {}
    PersCommandSingle( PersCommandCreator& c, PersCmd cmd ) : PersCommand(c,cmd) {}
    Property& property() { return property_; }
    int32_t result() const { return result_; }
    void setResult( int32_t res ) { result_ = res; }
    virtual int fillSB( SerialBuffer& sb );
    virtual int readSB( SerialBuffer& sb );
    inline void setCreator( PersCommandCreator& c ) { creator_ = &c; cmdType_ = c.cmdType(); }
protected:
    int readStatus( SerialBuffer& sb );
private:
    Property  property_;
    int32_t   result_;
};


class PersCommandBatch : public PersCommand
{
public:
    PersCommandBatch( PersCommandCreator& c,
                      const std::vector< PersCommandSingle >& cmds,
                      bool trans ) :
    PersCommand(c,PC_MTBATCH), batch_(cmds), transact_(trans), index_(0) {}
    virtual ~PersCommandBatch() {}
    virtual int fillSB( SerialBuffer& sb );
    virtual int readSB( SerialBuffer& sb );
    virtual void storeResults( re::actions::ActionContext& ctx );

    virtual int failIndex() const { return index_; }

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
