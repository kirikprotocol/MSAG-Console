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
    USSMessageBase(unsigned char msgTag) { setObjectId(msgTag); _status = 0; }
    virtual ~USSMessageBase() {}

    //USSCommand interface: not used for now
    //virtual void handle( USSCommandHandler* );

    //SerializableObject interface:
    void load(ObjectBuffer &in);
    void save(ObjectBuffer &out);

    //own methods
    void setUSSData(unsigned char * data, unsigned size);
    void setUSSData(USSDATA_T& ussdata);
    void setMSISDNadr(const Address& msadr);
    void setMSISDNadr(const char * adrStr);
    void setDCS(const unsigned char& dcs);
    void setReqId(const unsigned int& req_id);
    void setStatus(const unsigned short& status);

    const USSDATA_T& getUSSData(void) const;
    const Address&   getMSISDNadr(void) const;
    unsigned char    getDCS(void) const;
    unsigned int     getReqId(void) const;
    unsigned short   getStatus(void) const;

protected:
    unsigned char   _dCS;
    USSDATA_T       _ussData;
    Address         _msAdr;
    unsigned int    _reqId;
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
