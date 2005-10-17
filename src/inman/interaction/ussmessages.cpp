static char const ident[] = "$Id$";

#include <assert.h>
#include <string>
#include <stdexcept>

#include "inman/interaction/ussmessages.hpp"

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


void SerializerUSS::serialize(SerializableObject* obj, ObjectBuffer& out)
{
    assert( obj );
    out << (uint32_t) obj->getObjectId();
    obj->save( out );
}


SerializableObject* SerializerUSS::deserialize(ObjectBuffer& in)
{
    uint32_t objectId;

    in >> objectId;
    SerializableObject* obj = create(objectId);
    if( !obj ) 
        throw runtime_error(format("SerializerUSS: unknown object, id: 0x%X", objectId));

    obj->setObjectId(objectId);
    obj->load(in);
    return obj;
}



/* ************************************************************************** *
 * class USSMessageBase implementation:
 * ************************************************************************** */
#ifndef MAP_MAX_ISDN_AddressLength
#define MAP_MAX_ISDN_AddressLength  8
#endif
/*
 * all USS messages has the similar serialization format:

  1b        4b        1b       1b        up to 160b   1b     up to 32
------   ---------   -----  ----------   ----------   ---  -----------------------
 cmdId : requestId  : DCS : ussDataLen  :  ussData  : len : ms ISDN address string
       |                                                                          |
       -------- processed by load()/save() methods -------------------------------
*/

void USSMessageBase::save(ObjectBuffer& out)
{
    out << _reqId;
    out << _dCS;
    out << _ussData;
    out << _msAdr.toString();
}

void USSMessageBase::load(ObjectBuffer& in)
{
    in >> _reqId;
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

void USSMessageBase::setUSSData(unsigned char * data, unsigned size)
{
    _ussData.clear();
    _ussData.insert(_ussData.begin(), data, data + size);
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


void USSMessageBase::setDCS(const unsigned char& dcs)
{
    _dCS = dcs;
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



} //interaction
} //inman
} //smsc

