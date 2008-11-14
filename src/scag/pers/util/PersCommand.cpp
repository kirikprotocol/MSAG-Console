#include "PersCommand.h"

namespace scag2 {
namespace pers {
namespace util {


/*
PersCommand* PersCommand::makeCommand( PersCmd cmd )
{
    switch (cmd) {
    case PC_DEL :
    case PC_SET :
    case PC_GET :
    case PC_INC_MOD :
    case PC_PING :
    case PC_INC_RESULT :
    case PC_BIND_ASYNCH :
        return new PersCommandSingle( cmd );
    case PC_MTBATCH :
        return new PersCommandBatch();
    default:
        throw PersClientException(COMMAND_NOTSUPPORT);
    }
}
 */


int PersCommandSingle::fillSB( SerialBuffer& sb )
{
    if ( cmdType() == PC_PING || cmdType() == PC_BIND_ASYNCH ) {
        return 0;
    }

    if ( cmdType() == PC_DEL || cmdType() == PC_GET ) {
        sb.WriteString( property().getName() );
        return 0;
    }

    if ( cmdType() == PC_INC_MOD ) sb.WriteInt32( result() );
    property().Serialize( sb );
    return 0;
}


int PersCommandSingle::readSB( SerialBuffer& sb )
{
    readStatus(sb);
    if ( ! status() ) {
        switch (cmdType()) {
        case PC_PING :
        case PC_BIND_ASYNCH :
        case PC_DEL :
        case PC_SET :
            break;
        case PC_INC_RESULT :
        case PC_INC_MOD :
            setResult( sb.ReadInt32() );
            break;
        case PC_GET :
            property().Deserialize( sb );
            break;
        default :
            setStatus(COMMAND_NOTSUPPORT);
        }
    }
    return status();
}


int PersCommandSingle::readStatus( SerialBuffer& sb )
{
    int stat = 0;
    switch ( PersServerResponseType(sb.ReadInt8()) ) {
    case (RESPONSE_OK) : break;
    case (RESPONSE_PROPERTY_NOT_FOUND) : stat = PROPERTY_NOT_FOUND; break;
    case (RESPONSE_ERROR) : stat = SERVER_ERROR; break;
    case (RESPONSE_BAD_REQUEST) : stat = BAD_REQUEST; break;
    case (RESPONSE_TYPE_INCONSISTENCE) : stat = TYPE_INCONSISTENCE; break;
    case (RESPONSE_PROFILE_LOCKED) : stat = PROFILE_LOCKED; break;
    default: stat = UNKNOWN_RESPONSE;
    }
    if ( stat ) setStatus( stat );
    return stat;
}

/*
bool PersCommandBatch::setBatch( const std::vector< PersCommandSingle >& cmds ) 
{
    if ( cmds.size() == 0 ) throw PersClientException(BATCH_ERROR);
    batch_ = cmds;
    return true;
}
 */


int PersCommandBatch::fillSB( SerialBuffer& sb )
{
    sb.WriteInt16( batch_.size() );
    sb.WriteInt8( transact_ );
    for ( std::vector< PersCommandSingle >::iterator i = batch_.begin();
          i != batch_.end();
          ++i ) {
        sb.WriteInt8( i->cmdType() );
        int stat = i->fillSB( sb );
        if ( stat > 0 ) {
            setStatus(stat, i-batch_.begin()+1);
            break;
        }
    }
    return status();
}


int PersCommandBatch::readSB( SerialBuffer& sb )
{
    for ( std::vector< PersCommandSingle >::iterator i = batch_.begin();
          i != batch_.end();
          ++i ) {
        int stat = i->readSB( sb );
        if ( stat > 0 && status() == 0 ) {
            setStatus(stat,i-batch_.begin()+1);
            if ( transact_ ) break;
        }
    }
    return status();
}


void PersCommandBatch::storeResults( re::actions::ActionContext& ctx )
{
    for ( std::vector< PersCommandSingle >::iterator i = batch_.begin();
          i != batch_.end();
          ++i ) {
        i->storeResults( ctx );
    }
    PersCommand::storeResults( ctx );
}

}
}
}
