#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/interaction/messages.hpp"

namespace smsc  {
namespace inman {
namespace interaction {
/* ************************************************************************** *
 * class INProtocol implementation:
 * ************************************************************************** */
const INProtocol::CSParams INProtocol::_csParm[] = {
    INProtocol::CSParams(0, "csUndefined")
  , INProtocol::CSParams(5, "csBilling")
  , INProtocol::CSParams(9, "csAbntContract")
  , INProtocol::CSParams((unsigned)(-1), "csUndefined")
};


/* ************************************************************************** *
 * class INPSerializer implementation:
 * ************************************************************************** */
INPSerializer* INPSerializer::getInstance()
{
    static INPSerializer instance;
    return &instance;
}

const INPCommandSetAC *
    INPSerializer::commandSet(unsigned short cmd_id) const
{
    const INPCommandSetAC * cmdSet = NULL;
    INProtocol::CSId    csId = INProtocol::csIdbyCmdId(cmd_id);
    if (csId != INProtocol::csUndefined) {
        INPCsMap::const_iterator cit = cmdSets.find(csId);
        if (cit != cmdSets.end())
            cmdSet = cit->second;
    }
    return cmdSet;
}

INPPacketAC * INPSerializer::deserialize(ObjectBuffer& in) const
        throw(SerializerException)
{
    std::auto_ptr<INPPacketAC> pck; 
    const INPCommandSetAC * cmdSet = loadHdr(in, pck);     //throws
    loadObj(cmdSet, pck.get(), &in, false);     //throws
    return pck.release();
}

INPPacketAC * 
    INPSerializer::deserialize(std::auto_ptr<ObjectBuffer>& p_in) const
        throw(SerializerException)
{
    ObjectBuffer * in = p_in.get();
    std::auto_ptr<INPPacketAC> pck;             //throws
    const INPCommandSetAC * cmdSet = loadHdr(*in, pck);    //throws
    if (!loadObj(cmdSet, pck.get(), in, true))  //throws
        p_in.release(); //take ownership of ObjectBuffer for deferred deserialization!
    return pck.release();
}

/* -------------------------------------------------------------------------- *
 * Private/Protected methods:
 * -------------------------------------------------------------------------- */
const INPCommandSetAC *
    INPSerializer::loadHdr(ObjectBuffer & in, std::auto_ptr<INPPacketAC> & pck) const
        throw(SerializerException)
{
    unsigned short cmdId = 0, hdrFrm = 0;
    try {
        in >> cmdId;
        in >> hdrFrm;
    } catch (SerializerException & exc) {
        throw SerializerException("INPSrlzr: invalid packet structure",
                                  SerializerException::invPacket, exc.what());
    }
    const INPCommandSetAC * cmdSet = commandSet(cmdId);
    if (!cmdSet)
        throw SerializerException("INPSrlzr: illegal command",
                                  SerializerException::invObject);

    pck.reset(cmdSet->createPck(cmdId, hdrFrm));
    if (!pck.get())
        throw SerializerException("INPSrlzr: illegal header",
                                  SerializerException::invObject);
    try { pck->pHdr()->load(in); }
    catch (SerializerException & exc) {
        throw SerializerException("INPSrlzr: corrupted header",
                                  SerializerException::invObjData, exc.what());
    }
    return cmdSet;
}

//Returns false if deserialization of ObjectBuffer is deferred.
bool INPSerializer::loadObj(const INPCommandSetAC * cmd_set, INPPacketAC * pck,
                            ObjectBuffer * in, bool ownBuf/* = false*/) const
        throw(SerializerException)
{
    if (cmd_set->loadMode((pck->pCmd())->Id()) == INPCommandSetAC::lmHeader) {
        (pck->pCmd())->setDataBuf(in, ownBuf);
        return false;
    }
    try { (pck->pCmd())->load(*in); 
    } catch (SerializerException & exc) {
        throw SerializerException("INPSrlzr: corrupted command",
                                  SerializerException::invObjData, exc.what());
    }
    return true;
}


} //interaction
} //inman
} //smsc
