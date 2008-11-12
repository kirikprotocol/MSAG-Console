#include <cassert>
#include "ProfileProxy.h"
#include "scag/re/base/ActionContext2.h"
#include "PersClient2.h"
#include "core/synchronization/Mutex.hpp"
#include "logger/Logger.h"

namespace {
smsc::logger::Logger*              log_;
smsc::core::synchronization::Mutex logMtx;
}


namespace scag2 {

typedef util::properties::Property REProperty;

using namespace util::storage;

namespace pers {
namespace util {

using namespace exceptions;


bool PropertyProxyBase::canProcessRequest( re::actions::ActionContext& context )
{
    int status = pers::util::PersClient::Instance().getClientStatus();
    if ( status > 0 ) {
        setStatus( context, status );
        return false;
    }
    return true;
}


void PropertyProxyBase::setStatus( re::actions::ActionContext& context,
                                   int status,
                                   int actionIdx )
{
    REProperty *statusProp = context.getProperty(statusName());
    if (statusProp) {
        statusProp->setInt(status);
    }
    REProperty *msgProp = context.getProperty(msgName());
    while (msgProp) {
        const char* m = "Ok";
        if (status != 0) {
            if ( status < 0 ) status = 0;
            m = pers::util::strs[status];
            if (actionIdx > 0) {
                std::string msg = m;
                char idx_buffer[40];
                snprintf(idx_buffer, sizeof(idx_buffer), " in action %d", actionIdx);
                msg += idx_buffer;
                msgProp->setStr(msg.c_str());
                break;
            }
        }
        msgProp->setStr(m);
        break;
    }
}


int PropertyProxyBase::readServerStatus( re::actions::ActionContext& ctx,
                                         SerialBuffer& sb )
{
    int status = 0;
    switch ( PersServerResponseType(sb.ReadInt8()) ) {
    case (RESPONSE_OK) : break;
    case (RESPONSE_PROPERTY_NOT_FOUND) : status = PROPERTY_NOT_FOUND; break;
    case (RESPONSE_ERROR) : status = SERVER_ERROR; break;
    case (RESPONSE_BAD_REQUEST) : status = BAD_REQUEST; break;
    case (RESPONSE_TYPE_INCONSISTENCE) : status = TYPE_INCONSISTENCE; break;
    case (RESPONSE_PROFILE_LOCKED) : status = PROFILE_LOCKED; break;
    default: status = UNKNOWN_RESPONSE;
    }
    if ( status ) {
        setStatus( ctx, status );
    }
    return status;
}


// =================================================================

PersCallParams::PersCallParams( ProfileType pt, const std::string& statusName, const std::string& msgName ) :
error(0),
ikey_(0),
proxy_(pt, buf_),
single_(0), batch_(0), statusName_(statusName), msgName_(msgName) 
{
    if ( !log_ ) {
        MutexGuard mg(logMtx);
        if (!log_) log_ = smsc::logger::Logger::getInstance("pers.dump");
    }
}


void PersCallParams::setKey( const std::string& key )
{
    assert( buf_.getPos() == 0 );
    skey_ = key; // ikey_ = 0;
    proxy_.setKey( PersKey(skey_.c_str()) );
}


void PersCallParams::setKey( int32_t key )
{
    assert( buf_.getPos() == 0 );
    ikey_ = key; // skey_.clear();
    proxy_.setKey( PersKey(key) );
}


void PersCallParams::batch( std::vector< PropertyProxy* >& proxies, bool transact )
{
    assert( buf_.getPos() == 0 );
    single_ = 0;
    batch_ = &proxies;
    transact_ = transact;
    // check batch
    assert( batch_->size() );
    for ( std::vector< PropertyProxy* >::const_iterator i = proxies.begin();
          i != proxies.end();
          ++i ) {
        if ( ! *i ) throw SCAGException( "PersCallParams: no proxy specified" );
    }
}


void PersCallParams::single( PropertyProxy& proxy ) 
{
    assert( buf_.getPos() == 0 );
    single_ = &proxy;
    batch_ = 0;
}


bool PersCallParams::fillSB( re::actions::ActionContext& ctx )
{
    return ( fillSB( ctx, buf_ ) == 0 );
}

void PersCallParams::readSB( re::actions::ActionContext& ctx )
{
    try {
        readSB( ctx, buf_ );
    } catch ( SerialBufferOutOfBounds& ) {
        setStatus( ctx, BAD_RESPONSE );
    }
}


pers::util::PersCmd PersCallParams::cmdType() const
{
    return ( single_ ? single_->cmdType() : PC_MTBATCH );
}



int PersCallParams::fillSB( re::actions::ActionContext& ctx, SerialBuffer& sb )
{
    assert( sb.getPos() == 0 );
    assert( single_ || batch_ );
    // fill the header
    sb.Empty();
    sb.WriteInt32(0);
    const PersCmd cmd = cmdType();
    sb.WriteInt8( uint8_t(cmd) );
    sb.WriteInt8( uint8_t(proxy_.getType()) );
    if ( proxy_.getType() == PT_ABONENT ) {
        sb.WriteString( proxy_.getKey().skey );
    } else {
        sb.WriteInt32( proxy_.getKey().ikey );
    }

    int status = 0;
    if ( single_ ) {
        // single property
        status = single_->fillSB( ctx, sb );
        if ( status > 0 ) {
            // failed
            // FIXME: not needed as status was already set in single
            // setStatus( ctx, status );
        }
    } else {
        // batch
        sb.WriteInt16( batch_->size() );
        sb.WriteInt8( uint8_t(transact_) );

        for ( std::vector< PropertyProxy* >::const_iterator i = batch_->begin();
              i != batch_->end();
              ++i ) {
            sb.WriteInt8( uint8_t((*i)->cmdType()) );
            status = (*i)->fillSB( ctx, sb );
            if ( status > 0 ) {
                setStatus( ctx, status, i - batch_->begin() + 1 );
                break;
            }
        }

    }
    if ( status == 0 ) {
        // ok, writing the length of the buffer
        const uint32_t len = sb.getPos();
        sb.SetPos(0);
        sb.WriteInt32(len);
        sb.SetPos(len);
    }
    return status;
}


int PersCallParams::readSB( re::actions::ActionContext& ctx, SerialBuffer& sb )
{
    assert( single_ || batch_ );
    // int status = readServerStatus(ctx,sb);
    // if ( status ) {
    // smsc_log_debug( log_, "server status is %d", status );
    // return status;
    // }
    int status = 0;

    // reading data
    if ( single_ ) {
        status = single_->readSB( ctx, sb );
    } else {
        // batch
        for ( std::vector< PropertyProxy* >::const_iterator i = batch_->begin();
              i != batch_->end();
              ++i ) {
            int stat = (*i)->readSB( ctx, sb );
            if ( stat > 0 && status == 0 ) {
                status = stat;
                setStatus( ctx, stat, i - batch_->begin() + 1 );
                if ( transact_ ) break;
            }
        }
        if ( status == 0 ) {
            setStatus( ctx, status );
        }
    }
    return status;
}


}
}
}
