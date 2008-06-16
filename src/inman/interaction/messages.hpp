#pragma ident "$Id$"
#ifndef __SMSC_INMAN_INTERACTION_MESSAGES__
#define __SMSC_INMAN_INTERACTION_MESSAGES__

#include "util/Factory.hpp"
using smsc::util::FactoryT;

#include "inman/interaction/serializer.hpp"
#include "inman/interaction/INPMsgDefs.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

//INMan Protocol CommandSet: factory of commands and their subobjects
class INPCommandSetAC;

typedef SerializableObjectAC INPHeaderAC;

class INPCommandAC: public SerializableObjectAC {
public:
    INPCommandAC(unsigned short use_id) : SerializableObjectAC(use_id)
    { }
    virtual const INPCommandSetAC * commandSet(void) const = 0;
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
    INPSolidPacketT(const INPSolidPacketT &org_pck) : INPPacketAC(org_pck)
        { referObj(0, pckHdr); referObj(1, pckCmd); } //fix at([01]) references

    _Header &  Hdr() { return pckHdr; }
    _Command & Cmd() { return pckCmd; }
};

//INMan Protocol CommandSet: factory of commands and their subobjects
class INPCommandSetAC {
protected:
    typedef FactoryT<uint32_t, INPPacketAC> PckFactory;
    PckFactory  pckFct;

    inline uint32_t 
        mkPckIdx(unsigned short cmd_id, unsigned short hdr_frm) const
    {
        return ((cmd_id << 16) | (hdr_frm & 0xFFFF));
    }

//    typedef FactoryT<unsigned short, INPCommandAC> CmdFactory;
//    CmdFactory  cmdFct;
//    typedef FactoryT<unsigned short, INPHeaderAC> HdrFactory; 
//    HdrFactory  hdrFct;

public:
    //Packet deserialization modes
    typedef enum {
        lmHeader = 0,   //deserialize only header
        lmFull          //deserialize full packet
    } INPLoadMode;

    virtual INProtocol::CSId CsId(void) const = 0;
    virtual const char *    CsName(void) const = 0;
    virtual INPLoadMode loadMode(unsigned short cmd_id) const = 0;

    //creates solid packet (header + command)
    inline INPPacketAC * 
        createPck(unsigned short cmd_id, unsigned short hdr_frm) const
    {
        return pckFct.create(mkPckIdx(cmd_id, hdr_frm));
    }

//    inline INPHeaderAC * createHdr(unsigned short hdr_frm) const
//        { return hdrFct.create(hdr_frm); }
//    inline INPCommandAC * createCmd(unsigned short cmd_id) const
//        { return cmdFct.create(obj_id); }
};


//DeSerializer for Inman packets, transferred over TCP connect
class INPSerializer : public SerializerITF {
private:
    typedef std::map<INProtocol::CSId, const INPCommandSetAC*> INPCsMap;

    INPCsMap cmdSets;

protected:
    INPSerializer()
    { }
    const INPCommandSetAC *
        loadHdr(ObjectBuffer & in, std::auto_ptr<INPPacketAC> & pck) const
            throw(SerializerException);
    //Returns false if deserialization of ObjectBuffer is deferred.
    bool loadObj(const INPCommandSetAC * cmd_set, INPPacketAC * pck,
                        ObjectBuffer * in, bool ownBuf = false) const
            throw(SerializerException);
public:
    virtual ~INPSerializer()
    { }

    inline bool registerCmdSet(const INPCommandSetAC * p_cs)
    {
        std::pair<INPCsMap::iterator, bool> res =
            cmdSets.insert(INPCsMap::value_type(p_cs->CsId(), p_cs));
        return res.second;
    }
    const INPCommandSetAC * commandSet(unsigned short cmd_id) const;
    //SerializerITF interface:
    INPPacketAC *   deserialize(ObjectBuffer& in) const throw(SerializerException); 
    INPPacketAC *   deserialize(std::auto_ptr<ObjectBuffer>& p_in) const throw(SerializerException);

    static INPSerializer* getInstance();
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_INTERACTION_MESSAGES__ */

