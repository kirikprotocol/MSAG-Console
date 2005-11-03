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

//NOTE: USSman considers the USS request encoding with DCS == 0xF4 as plain LATIN1 text
#define USSMAN_LATIN1_DCS 0xF4 // 1111 0100

namespace smsc  {
namespace inman {
namespace interaction {

struct USS2CMD {
    enum _TAG {
        PROCESS_USS_REQUEST_TAG = 1,
        PROCESS_USS_RESULT_TAG = 2
    };
    enum _STATUS {
        STATUS_USS_REQUEST_OK = 0,      //request successfully processed
        STATUS_USS_REQUEST_DENIAL = 1,  //request is already in process
        STATUS_USS_REQUEST_FAILED = 2   //request failed because of USSMan encounter an error
    };
};

//serializer for USSman commands, transferred over TCP socket
class SerializerUSS : public SerializerITF, public FactoryT< unsigned short, SerializableObject >
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

// 1. USSRequest         ( client --> USSMAN )
// 2. USSRequestResult   ( client <-- USSMAN )
// 3. USSRequestDenial   ( client <-- USSMAN )


/*
class USSCommandHandler;
class USSCommand : public SerializableObject
{
    virtual void handle( USSCommandHandler* ) = 0;
};
*/
class USSMessageBase : public SerializableObject //USSCommand not used for now
{
public:
    USSMessageBase(unsigned short msgTag) { setObjectId(msgTag); _status = 0; }
    virtual ~USSMessageBase() {}

    //USSCommand interface: not used for now
    //virtual void handle( USSCommandHandler* );

    //SerializableObject interface:
    void load(ObjectBuffer &in);
    void save(ObjectBuffer &out);

    //assigns USS data, that is plain LATIN1 text,
    void setUSSData(const unsigned char * data, unsigned size);
    //assigns USS data encoded according to CBS coding scheme (UCS2, GSM 7bit, etc)
    void setRAWUSSData(unsigned char dcs, const USSDATA_T& ussdata);

    void setMSISDNadr(const Address& msadr);
    void setMSISDNadr(const char * adrStr);

    void setStatus(const unsigned short& status);

    const USSDATA_T& getUSSData(void) const;
    const Address&   getMSISDNadr(void) const;
    unsigned char    getDCS(void) const;
    unsigned short   getStatus(void) const;

protected:
    unsigned char   _dCS;
    USSDATA_T       _ussData;
    Address         _msAdr;
    unsigned short  _status; //only for PROCESS_USS_RESULT_TAG
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

    bool  getUSSDataAsLatin1Text(std::string & str);
};

class USSCommandHandler
{
    public:
        virtual void onProcessUSSRequest(USSRequestMessage* req) = 0;
        virtual void onDenyUSSRequest(USSRequestMessage* req) = 0;
};



} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_INTERACTION_USSMESSAGES__ */
