/* ************************************************************************* *
 * INMan Protocols: generic packet definition, packet serialization.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_INTERACTION_MESSAGES__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INTERACTION_MESSAGES__

#include "util/Factory.hpp"

#include "inman/interaction/serializer.hpp"
#include "inman/interaction/INPMsgDefs.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

//INMan Protocol CommandSet: factory of commands and their subobjects
class INPCommandSetAC;

typedef SerializableObjectAC INPHeaderAC;

class INPCommandAC: public SerializableObjectAC {
protected:
  virtual ~INPCommandAC()
  { }

public:
  INPCommandAC(unsigned short use_id) : SerializableObjectAC(use_id)
  { }

  // ----------------------------------
  // -- INPCommandAC interface methods
  // ----------------------------------
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
class INPPacketAC : public SerializablePacket_T<2> { //[0] = header, [1] = cmdObject
protected:
  INPPacketAC()
  { }

public:
  virtual ~INPPacketAC()
  { }

  INPHeaderAC * pHdr(void) const { return static_cast<INPHeaderAC*>(getObj(0)); }
  INPCommandAC * pCmd(void) const { return static_cast<INPCommandAC*>(getObj(1)); }

  // --------------------------------------------------
  // -- SerializablePacketAC interface implementation
  // --------------------------------------------------
  virtual void serialize(ObjectBuffer& out_buf) const throw(SerializerException)
  {
    out_buf << getObj(1)->Id();
    out_buf << getObj(0)->Id();
    getObj(0)->save(out_buf);
    getObj(1)->save(out_buf);
  }
};

//Template class for solid packet construction.
template< class _Header /* : public INPHeaderAC*/,
          class _Command /* : public INPCommandAC*/
>
class INPSolidPacketT : public INPPacketAC {
protected:
  _Header  pckHdr;
  _Command pckCmd;

public:
  INPSolidPacketT() : INPPacketAC()
  {
    referObj(0, pckHdr); referObj(1, pckCmd);
  }
  //constructor for copying
  INPSolidPacketT(const INPSolidPacketT &org_pck) : INPPacketAC(org_pck)
  {
    referObj(0, pckHdr); referObj(1, pckCmd);
  }
  ~INPSolidPacketT()
  { }

  _Header &  Hdr() { return pckHdr; }
  _Command & Cmd() { return pckCmd; }

  const _Header &  Hdr() const { return pckHdr; }
  const _Command & Cmd() const { return pckCmd; }
};

//INMan Protocol CommandSet: factory of commands and their subobjects
class INPCommandSetAC {
private:
  const INProtocol::CSId  _csId;

protected:
  typedef smsc::util::FactoryT<uint32_t, INPPacketAC> PckFactory;
  PckFactory  _pckFct;

  uint32_t mkPckIdx(unsigned short cmd_id, unsigned short hdr_frm) const
  {
    return ((cmd_id << 16) | (hdr_frm & 0xFFFF));
  }

  INPCommandSetAC(INProtocol::CSId cs_id) : _csId(cs_id)
  { }
  virtual ~INPCommandSetAC()
  { }

public:
  //Packet deserialization modes
  enum INPLoadMode {
      lmHeader = 0,   //deserialize only header
      lmFull          //deserialize full packet
  };

  INProtocol::CSId  CsId(void) const { return _csId; }
  const char *      CsName(void) const { return INProtocol::csName(_csId); }

  // ----------------------------------------
  // -- INPCommandSetAC interface methods
  // ----------------------------------------
  virtual INPLoadMode loadMode(unsigned short cmd_id) const = 0;

  //creates solid packet (header + command)
  INPPacketAC * createPck(unsigned short cmd_id, unsigned short hdr_frm) const
  {
    return _pckFct.create(mkPckIdx(cmd_id, hdr_frm));
  }
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

    bool registerCmdSet(const INPCommandSetAC * p_cs)
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

