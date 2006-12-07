#ident "$Id$"
#ifndef __SMSC_INMAN_INTERACTION_MESSAGES__
#define __SMSC_INMAN_INTERACTION_MESSAGES__

#include "util/Factory.hpp"
using smsc::util::FactoryT;

#include "inman/interaction/serializer.hpp"

namespace smsc  {
namespace inman {
namespace interaction {
//NOTE: the plain numbering of object Ids through all
//command sets is used for simplicity!
typedef enum {
    csBilling  = 1, //SMS Charging dialog, objIds [1..5]
    csAbntContract,  //Subscriber contract and gsmSCF parms determination [6..9]
    csSession,      //Session establishment and control, objIds [10..15]
    csControl,      //INman service control (radmin), objIds [16...] 
    //...
    csUndefined  // just a max cap, reserved
} INPCommandSetId;

static INPCommandSetId csIdbyCmdId(unsigned cmd_id)
{
    if (cmd_id >= 16)
        return csControl;
    if (cmd_id >= 10)
        return csSession;
    if (cmd_id >= 6)
        return csAbntContract;
    return cmd_id ? csBilling : csUndefined;
}

static const char * csNamebyCmdId(unsigned cmd_id)
{
    if (cmd_id >= 16)
        return "csControl";
    if (cmd_id >= 10)
        return "csSession";
    if (cmd_id >= 6)
        return "csAbntContract";
    return cmd_id ? "csBilling" : "csUndefined";
}

//INMan Protocol CommandSet: factory of commands and their subobjects
class INPCommandSetAC;

typedef SerializableObjectAC INPHeaderAC;

class INPCommandAC: public SerializableObjectAC {
public:
    INPCommandAC(unsigned short use_id) : SerializableObjectAC(use_id)
    { }
    virtual INPCommandSetAC * commandSet(void) const = 0;
};
/*
 * Inman messages are transferred as length prefixed packet of two
 * serialized objects(header and command) and have the following
 * serialization format:

  2b        2b       up to ...b          up to ...b
-------  ---------   ----------   -------------------------------------
 msgId : hdrFormat : hdrData    :  message data                        |
                     |          |                                      |                                    |
                      ---------  -- processed by load()/save() method --
*/
class INPPacketAC : public SerializablePacketAC { //[0] = header, [1] = cmdObject
public:
    INPPacketAC() { Resize(2); }
   
    INPHeaderAC * pHdr(void) { return at(0); }
    INPCommandAC * pCmd(void) { return static_cast<INPCommandAC*>(at(1)); }

    //SerializablePacketAC interface implementation
    void serialize(ObjectBuffer& out_buf) throw(SerializerException)
    {
        out_buf << at(1)->Id();
        out_buf << at(0)->Id();
        at(0)->save(out_buf);
        at(1)->save(out_buf);
    }
};

//Template class for solid packet construction.
template<class _Header /* : public INPHeaderAC*/,
         class _Command /* : public INPCommandAC*/>
class INPSolidPacketT : public INPPacketAC {
protected:
    _Header  pckHdr;
    _Command pckCmd;

public:
    INPSolidPacketT() : INPPacketAC()
        { referObj(0, pckHdr); referObj(1, pckCmd); }
    //constructor for copying
    INPSolidPacketT(const INPSolidPacketT &org_pck) : *this(org_pck)
        { referObj(0, pckHdr); referObj(1, pckCmd); } //fix at([01]) references

    _Header &  Hdr() { return pckHdr; }
    _Command & Cmd() { return pckCmd; }
};

//INMan Protocol CommandSet: factory of commands and their subobjects
class INPCommandSetAC {
protected:
    typedef FactoryT<uint32_t, INPPacketAC> PckFactory;
    PckFactory  pckFct;

    inline uint32_t mkPckIdx(unsigned short cmd_id, unsigned short hdr_frm)
        { return ((cmd_id << 16) | (hdr_frm & 0xFFFF)); }

//    typedef FactoryT<unsigned short, INPCommandAC> ObjFactory;
//    ObjFactory  objFct;
//    typedef FactoryT<unsigned short, INPHeaderAC> HdrFactory; 
//    HdrFactory  hdrFct;

public:
    //Packet deserialization modes
    typedef enum {
        lmHeader = 0,   //deserialize only header
        lmFull          //deserialize full packet
    } INPLoadMode;

    virtual INPCommandSetId CsId(void) const = 0;
    virtual const char *    CsName(void) const = 0;
    virtual INPLoadMode loadMode(unsigned short cmd_id) const = 0;

    //creates solid packet (header + command)
    INPPacketAC * createPck(unsigned short cmd_id, unsigned short hdr_frm)
        { return pckFct.create(mkPckIdx(cmd_id, hdr_frm)); }

//    INPHeaderAC * createHdr(unsigned short hdr_frm)
//        { return hdrFct.create(hdr_frm); }
//    INPCommandAC * createObj(unsigned short cmd_id)
//        { return objFct.create(obj_id); }
};


//DeSerializer for Inman packets, transferred over TCP connect
class INPSerializer : public SerializerITF {
public:
    virtual ~INPSerializer() { }

    INPCommandSetAC * commandSet(unsigned short cmd_id);
    //SerializerITF interface:
    INPPacketAC *   deserialize(ObjectBuffer& in) throw(SerializerException); 
    INPPacketAC *   deserialize(std::auto_ptr<ObjectBuffer>& p_in) throw(SerializerException);

    static INPSerializer* getInstance();

protected:
    INPSerializer();
    INPCommandSetAC* loadHdr(ObjectBuffer & in, std::auto_ptr<INPPacketAC> & pck)
                        throw(SerializerException);
    //Returns false if deserialization of ObjectBuffer is deferred.
    bool            loadObj(INPCommandSetAC * cmd_set, INPPacketAC * pck,
                            ObjectBuffer * in, bool ownBuf = false)
                        throw(SerializerException);

private:
    typedef std::map<INPCommandSetId, INPCommandSetAC*> INPCsMap;

    INPCsMap cmdSets;
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_INTERACTION_MESSAGES__ */

