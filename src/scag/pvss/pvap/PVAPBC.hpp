// NOTE: Do NOT edit this file (it is auto-generated).
// Instead, edit proto-cpp.tmpl and regenerate.

#ifndef __SCAG_PVSS_PVAP_PVAPBC_HPP__
#define __SCAG_PVSS_PVAP_PVAPBC_HPP__

#include <memory>
#include "Serializer.h"
#include "Exceptions.h"
#include "TypeId.h"
#include "BC_DEL.hpp"
#include "BC_DEL_RESP.hpp"
#include "BC_SET.hpp"
#include "BC_SET_RESP.hpp"
#include "BC_GET.hpp"
#include "BC_GET_RESP.hpp"
#include "BC_INC.hpp"
#include "BC_INC_RESP.hpp"
#include "BC_INC_MOD.hpp"
#include "BC_INC_MOD_RESP.hpp"

namespace scag {
namespace pvss {
namespace pvap {

class PVAPBC 
{
public:
    enum {
        tag_BC_DEL=1,
        tag_BC_DEL_RESP=32769,
        tag_BC_SET=2,
        tag_BC_SET_RESP=32770,
        tag_BC_GET=3,
        tag_BC_GET_RESP=32771,
        tag_BC_INC=4,
        tag_BC_INC_RESP=32772,
        tag_BC_INC_MOD=5,
        tag_BC_INC_MOD_RESP=32773,
        tag_NO_TAG = 0xffff
    };

    class Handler
    {
    public:
        virtual bool hasSeqNum( uint32_t seqNum ) const = 0;
        virtual void handle( std::auto_ptr<BC_DEL> obj ) = 0;
        virtual void handle( std::auto_ptr<BC_DEL_RESP> obj ) = 0;
        virtual void handle( std::auto_ptr<BC_SET> obj ) = 0;
        virtual void handle( std::auto_ptr<BC_SET_RESP> obj ) = 0;
        virtual void handle( std::auto_ptr<BC_GET> obj ) = 0;
        virtual void handle( std::auto_ptr<BC_GET_RESP> obj ) = 0;
        virtual void handle( std::auto_ptr<BC_INC> obj ) = 0;
        virtual void handle( std::auto_ptr<BC_INC_RESP> obj ) = 0;
        virtual void handle( std::auto_ptr<BC_INC_MOD> obj ) = 0;
        virtual void handle( std::auto_ptr<BC_INC_MOD_RESP> obj ) = 0;
    };

    PVAPBC() : handler(0) {}

    PVAPBC( Handler* newHandler ) : handler(newHandler) {}

    void assignHandler( Handler* newHandler)
    {
        handler=newHandler;
    }

    void decodeMessage( Serializer& ss ) const throw (PvapException)
    {
        const uint32_t seqNum = ss.readInt32();
        if ( ! handler->hasSeqNum(seqNum) ) throw UnexpectedSeqNumException(seqNum);
        int tag = ss.readTag();
        switch(tag)
        {
        case tag_BC_DEL: {
            // printf( "tag %d (%s)\n", tag, "BC_DEL" );
            std::auto_ptr<BC_DEL> msg(new BC_DEL);
            msg->setSeqNum(seqNum);
            msg->deserialize(*this,ss);
            handler->handle(msg);
            break;
        }
        case tag_BC_DEL_RESP: {
            // printf( "tag %d (%s)\n", tag, "BC_DEL_RESP" );
            std::auto_ptr<BC_DEL_RESP> msg(new BC_DEL_RESP);
            msg->setSeqNum(seqNum);
            msg->deserialize(*this,ss);
            handler->handle(msg);
            break;
        }
        case tag_BC_SET: {
            // printf( "tag %d (%s)\n", tag, "BC_SET" );
            std::auto_ptr<BC_SET> msg(new BC_SET);
            msg->setSeqNum(seqNum);
            msg->deserialize(*this,ss);
            handler->handle(msg);
            break;
        }
        case tag_BC_SET_RESP: {
            // printf( "tag %d (%s)\n", tag, "BC_SET_RESP" );
            std::auto_ptr<BC_SET_RESP> msg(new BC_SET_RESP);
            msg->setSeqNum(seqNum);
            msg->deserialize(*this,ss);
            handler->handle(msg);
            break;
        }
        case tag_BC_GET: {
            // printf( "tag %d (%s)\n", tag, "BC_GET" );
            std::auto_ptr<BC_GET> msg(new BC_GET);
            msg->setSeqNum(seqNum);
            msg->deserialize(*this,ss);
            handler->handle(msg);
            break;
        }
        case tag_BC_GET_RESP: {
            // printf( "tag %d (%s)\n", tag, "BC_GET_RESP" );
            std::auto_ptr<BC_GET_RESP> msg(new BC_GET_RESP);
            msg->setSeqNum(seqNum);
            msg->deserialize(*this,ss);
            handler->handle(msg);
            break;
        }
        case tag_BC_INC: {
            // printf( "tag %d (%s)\n", tag, "BC_INC" );
            std::auto_ptr<BC_INC> msg(new BC_INC);
            msg->setSeqNum(seqNum);
            msg->deserialize(*this,ss);
            handler->handle(msg);
            break;
        }
        case tag_BC_INC_RESP: {
            // printf( "tag %d (%s)\n", tag, "BC_INC_RESP" );
            std::auto_ptr<BC_INC_RESP> msg(new BC_INC_RESP);
            msg->setSeqNum(seqNum);
            msg->deserialize(*this,ss);
            handler->handle(msg);
            break;
        }
        case tag_BC_INC_MOD: {
            // printf( "tag %d (%s)\n", tag, "BC_INC_MOD" );
            std::auto_ptr<BC_INC_MOD> msg(new BC_INC_MOD);
            msg->setSeqNum(seqNum);
            msg->deserialize(*this,ss);
            handler->handle(msg);
            break;
        }
        case tag_BC_INC_MOD_RESP: {
            // printf( "tag %d (%s)\n", tag, "BC_INC_MOD_RESP" );
            std::auto_ptr<BC_INC_MOD_RESP> msg(new BC_INC_MOD_RESP);
            msg->setSeqNum(seqNum);
            msg->deserialize(*this,ss);
            handler->handle(msg);
            break;
        }
        default:
            throw InvalidMessageTypeException(tag);
        }
    }

    void encodeMessage( const BC_DEL& msg, Serializer& ss ) const
        throw (PvapException)
    {
        ss.writeInt32(msg.getSeqNum());
        ss.writeTag(tag_BC_DEL);
        msg.serialize(*this,ss);
    }

    void encodeMessage( const BC_DEL_RESP& msg, Serializer& ss ) const
        throw (PvapException)
    {
        ss.writeInt32(msg.getSeqNum());
        ss.writeTag(tag_BC_DEL_RESP);
        msg.serialize(*this,ss);
    }

    void encodeMessage( const BC_SET& msg, Serializer& ss ) const
        throw (PvapException)
    {
        ss.writeInt32(msg.getSeqNum());
        ss.writeTag(tag_BC_SET);
        msg.serialize(*this,ss);
    }

    void encodeMessage( const BC_SET_RESP& msg, Serializer& ss ) const
        throw (PvapException)
    {
        ss.writeInt32(msg.getSeqNum());
        ss.writeTag(tag_BC_SET_RESP);
        msg.serialize(*this,ss);
    }

    void encodeMessage( const BC_GET& msg, Serializer& ss ) const
        throw (PvapException)
    {
        ss.writeInt32(msg.getSeqNum());
        ss.writeTag(tag_BC_GET);
        msg.serialize(*this,ss);
    }

    void encodeMessage( const BC_GET_RESP& msg, Serializer& ss ) const
        throw (PvapException)
    {
        ss.writeInt32(msg.getSeqNum());
        ss.writeTag(tag_BC_GET_RESP);
        msg.serialize(*this,ss);
    }

    void encodeMessage( const BC_INC& msg, Serializer& ss ) const
        throw (PvapException)
    {
        ss.writeInt32(msg.getSeqNum());
        ss.writeTag(tag_BC_INC);
        msg.serialize(*this,ss);
    }

    void encodeMessage( const BC_INC_RESP& msg, Serializer& ss ) const
        throw (PvapException)
    {
        ss.writeInt32(msg.getSeqNum());
        ss.writeTag(tag_BC_INC_RESP);
        msg.serialize(*this,ss);
    }

    void encodeMessage( const BC_INC_MOD& msg, Serializer& ss ) const
        throw (PvapException)
    {
        ss.writeInt32(msg.getSeqNum());
        ss.writeTag(tag_BC_INC_MOD);
        msg.serialize(*this,ss);
    }

    void encodeMessage( const BC_INC_MOD_RESP& msg, Serializer& ss ) const
        throw (PvapException)
    {
        ss.writeInt32(msg.getSeqNum());
        ss.writeTag(tag_BC_INC_MOD_RESP);
        msg.serialize(*this,ss);
    }

    void encodeCastMessage( const BC_CMD& msg, Serializer& ss ) const
        throw (PvapException)
    {
        const int msgId = msg.getId();
        if ( msgId == TypeId<BC_DEL>::getId() ) {
            encodeMessage(static_cast<const BC_DEL&>(msg), ss);
        } else
        if ( msgId == TypeId<BC_DEL_RESP>::getId() ) {
            encodeMessage(static_cast<const BC_DEL_RESP&>(msg), ss);
        } else
        if ( msgId == TypeId<BC_SET>::getId() ) {
            encodeMessage(static_cast<const BC_SET&>(msg), ss);
        } else
        if ( msgId == TypeId<BC_SET_RESP>::getId() ) {
            encodeMessage(static_cast<const BC_SET_RESP&>(msg), ss);
        } else
        if ( msgId == TypeId<BC_GET>::getId() ) {
            encodeMessage(static_cast<const BC_GET&>(msg), ss);
        } else
        if ( msgId == TypeId<BC_GET_RESP>::getId() ) {
            encodeMessage(static_cast<const BC_GET_RESP&>(msg), ss);
        } else
        if ( msgId == TypeId<BC_INC>::getId() ) {
            encodeMessage(static_cast<const BC_INC&>(msg), ss);
        } else
        if ( msgId == TypeId<BC_INC_RESP>::getId() ) {
            encodeMessage(static_cast<const BC_INC_RESP&>(msg), ss);
        } else
        if ( msgId == TypeId<BC_INC_MOD>::getId() ) {
            encodeMessage(static_cast<const BC_INC_MOD&>(msg), ss);
        } else
        if ( msgId == TypeId<BC_INC_MOD_RESP>::getId() ) {
            encodeMessage(static_cast<const BC_INC_MOD_RESP&>(msg), ss);
        } else
        {
            throw InvalidMessageTypeException(msgId);
        }
    }
protected:
    Handler* handler; // unowned
};

}
}
}

#endif
