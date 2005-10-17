#ident "$Id$"

#ifndef __SMSC_INMAN_INTERACTION_USSMESSAGES__
#define __SMSC_INMAN_INTERACTION_USSMESSAGES__

#include "sms/sms.h"
#include "inman/interaction/serializer.hpp"
#include "inman/common/factory.hpp"

using std::runtime_error;
using smsc::sms::Address;
using smsc::inman::interaction::ObjectBuffer;
using smsc::inman::interaction::SerializableObject;
using smsc::inman::interaction::SerializerITF;
using smsc::inman::common::FactoryT;

namespace smsc  {
namespace inman {
namespace interaction {

struct USS2CMD {
    enum {
        PROCESS_USS_REQUEST_TAG = 1,
        PROCESS_USS_RESULT_TAG = 2
    };
};

//serializer for USSman commands, transferred over TCP socket
class SerializerUSS : public SerializerITF, public FactoryT< USHORT_T, SerializableObject >
{
public:
    virtual ~SerializerUSS();

    SerializableObject * deserialize(ObjectBuffer &);
    void                 serialize(SerializableObject *, ObjectBuffer &out);

    static SerializerUSS* getInstance();

protected:
    SerializerUSS();
};


typedef std::vector<unsigned char> USSDATA_T;

class USSMessageBase : public SerializableObject
{
public:
    USSMessageBase(unsigned char msgTag) { setObjectId(msgTag); }
    virtual ~USSMessageBase() {}

    void load(ObjectBuffer &in);
    void save(ObjectBuffer &out);

    void setUSSData(unsigned char * data, unsigned size);
    void setMSISDNadr(const Address& msadr);
    void setMSISDNadr(const char * adrStr);
    void setDCS(const unsigned char& dcs);

    const USSDATA_T& getUSSData(void) const;
    const Address&   getMSISDNadr(void) const;
    unsigned char getDCS(void) const;

protected:
    unsigned char   _dCS;
    USSDATA_T       _ussData;
    Address         _msAdr;
    unsigned        _reqId;
};


class USSRequestMessage : public USSMessageBase
{
public:
    USSRequestMessage() : USSMessageBase(USS2CMD::PROCESS_USS_REQUEST_TAG) {}
    ~USSRequestMessage() {}
};


class USSResultMessage : public USSMessageBase
{
public:
    USSResultMessage() : USSMessageBase(USS2CMD::PROCESS_USS_RESULT_TAG) {}
    ~USSResultMessage() {};
};



} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_INTERACTION_USSMESSAGES__ */
