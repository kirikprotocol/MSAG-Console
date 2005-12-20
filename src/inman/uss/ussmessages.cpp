static char const ident[] = "$Id$";

#include <assert.h>
#include <string>
#include <stdexcept>

#include "inman/uss/ussmessages.hpp"
#include "inman/common/util.hpp"
#include "inman/common/cvtutil.hpp"

using std::runtime_error;
using smsc::inman::common::format;
using smsc::cbs::CBS_DCS;
using smsc::cbs::parseCBS_DCS;
using smsc::cvtutil::unpack7BitPadded2Text;

namespace smsc  {
namespace inman {
namespace interaction {

/* ************************************************************************** *
 * class SerializerUSS implementation:
 * ************************************************************************** */

SerializerUSS::SerializerUSS()
{
    registerProduct( USS2CMD::PROCESS_USS_REQUEST_TAG, new ProducerT< USSRequestMessage >() );
    registerProduct( USS2CMD::PROCESS_USS_RESULT_TAG, new ProducerT< USSResultMessage>() );
}

SerializerUSS::~SerializerUSS()
{
}

SerializerUSS* SerializerUSS::getInstance()
{
    static SerializerUSS instance;
    return &instance;
}

/*
 * USS messages are transferred with length prefix and have the following
 * serialization formats:

Request (PROCESS_USS_REQUEST_TAG):
--------
  1b        4b        1b       1b        up to 160b   1b     up to 32
------   ---------   -----  ----------   ----------   ---  -----------------------
 cmdId : requestId  : DCS : ussDataLen  :  ussData  : len : ms ISDN address string
                    |                                                            |
                    -------- processed by load() method --------------------------


Result (PROCESS_USS_RESULT_TAG):
--------

  1b        4b          2b      1b       1b        up to 160b   1b     up to 32
------   ---------   -------   ---   ----------   ----------   ---  -----------------------
 cmdId : requestId  : status : DCS : ussDataLen  :  ussData  : len : ms ISDN address string
                             |                                                             |
                              -------- present if status == 0 -----------------------------
                    |                                                                      |
                    -------- processed by save() method ------------------------------------
*/

void SerializerUSS::serialize(SerializableObject* obj, ObjectBuffer& out)
{
    assert( obj );
    out << (unsigned char) obj->getObjectId();
    out << (unsigned int) obj->getDialogId();
    obj->save( out );
}


SerializableObject* SerializerUSS::deserialize(ObjectBuffer& in)
{
    unsigned char objectId;
    unsigned int  reqId;

    in >> objectId;
    SerializableObject* obj = create((unsigned short)objectId);
    if( !obj ) 
        throw runtime_error(format("SerializerUSS: unknown object, id: 0x%X", (unsigned)objectId));

    in >> reqId;
    obj->setObjectId((unsigned short)objectId);
    obj->setDialogId(reqId);
    obj->load(in);
    return obj;
}

/* ************************************************************************** *
 * class USSMessageBase implementation:
 * ************************************************************************** */
/* SerializableObject interface:
 * ************************************************************************** */
void USSMessageBase::save(ObjectBuffer& out)
{
    if (getObjectId() == USS2CMD::PROCESS_USS_RESULT_TAG) {
        out << _status;
        if (_status)
            return;
    }
    out << _dCS;
    out << _ussData;
    out << _msAdr.toString();
}

void USSMessageBase::load(ObjectBuffer& in)
{
    if (getObjectId() == USS2CMD::PROCESS_USS_RESULT_TAG) {
        in >> _status;
        if (_status)
            return;
    }
    in >> _dCS;
    in >> _ussData;

//  read msISDN address
    unsigned char len;
    in >> len;
    char* strBuf = new char[len + 1];
    in.Read(strBuf, len);
    strBuf[len] = 0;

    Address msadr(strBuf);
    _msAdr = msadr;
}

/* ************************************************************************** *
 * USSCommand interface:
 * ************************************************************************** */
/* Not used for now:
void USSMessageBase::handle( USSCommandHandler* handler)
{
    assert( handler );
    handler->onProcessUSSRequest( this );

}
*/
/* ************************************************************************** *
 * Own methods:
 * ************************************************************************** */

void USSMessageBase::setRAWUSSData(unsigned char dcs, const USSDATA_T& ussdata)
{
    _ussData.clear();
    _ussData = ussdata;
    _dCS = dcs;
}

void USSMessageBase::setUSSData(const unsigned char * data, unsigned size)
{
    _ussData.clear();
    _ussData.insert(_ussData.begin(), data, data + size);
    _dCS = USSMAN_LATIN1_DCS;
}

void USSMessageBase::setMSISDNadr(const char * adrStr)
{
    Address	msadr(adrStr);
    _msAdr = msadr;
}

void USSMessageBase::setMSISDNadr(const Address& msadr)
{
    _msAdr = msadr;
}

void USSMessageBase::setStatus(const unsigned short& status)
{
    _status = status;
}

const USSDATA_T& USSMessageBase::getUSSData(void) const
{
    return _ussData;
}

const Address&   USSMessageBase::getMSISDNadr(void) const
{
    return _msAdr;
}

unsigned char USSMessageBase::getDCS(void) const
{
    return _dCS;
}

unsigned short USSMessageBase::getStatus(void) const
{
    return _status;
}

/* ************************************************************************** *
 * class USSResultMessage implementation:
 * ************************************************************************** */
bool  USSResultMessage::getUSSDataAsLatin1Text(std::string & str)
{
    CBS_DCS    parsedDCS;

    if (parseCBS_DCS(_dCS, parsedDCS) == CBS_DCS::dcGSM7Bit) {
        unsigned ussdLen = unpack7BitPadded2Text(&_ussData[0], _ussData.size(), str);
        //skip language prefix
        if (parsedDCS.lngPrefix == CBS_DCS::lng4GSM7Bit)
            str.erase(0, 3);
        return true;
    }
    return false;
}

} //interaction
} //inman
} //smsc

