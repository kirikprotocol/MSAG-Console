#include <cassert>
#include "PersCallParams.h"

namespace scag2 {
namespace pers {
namespace util {

int PersCallParams::fillSB( SerialBuffer& sb, int32_t serial )
{
    assert( sb.getPos() == 0 );
    sb.Empty();
    sb.WriteInt32(0); // length to be
    if ( serial ) sb.WriteInt32( serial );
    sb.WriteInt8( uint8_t( cmd_->cmdType() ) );
    sb.WriteInt8( uint8_t( getType() ) );
    if ( getType() == PT_ABONENT ) {
        sb.WriteString( getStringKey() );
    } else {
        sb.WriteInt32( getIntKey() );
    }
    cmd_->fillSB( sb );
    if ( 0 == cmd_->status() ) {
        const uint32_t len = sb.getPos();
        sb.SetPos(0);
        sb.WriteInt32(len);
        sb.SetPos(len);
    }
    return cmd_->status();
}


int PersCallParams::readSB( SerialBuffer& sb )
{
    // sb.SetPos(0);
    // sb.ReadInt32();
    return cmd_->readSB(sb);
}


/*
void PersCallParams::storeResults( re::actions::ActionContext& ctx )
{
    if ( cmd_->creator() ) cmd_->creator()->storeResults( ctx, * cmd_.get() );
}
 */

void PersCallParams::setStatus( int stat, const char* what )
{
    cmd_->setStatus( stat );
    if ( what ) {
        exception = what;
    } else if ( stat > 0 ) {
        exception = strs[stat];
    }
}

}
}
}
