static char const ident[] = "$Id$";

#include <assert.h>
#include <string>
#include <stdexcept>

#include "inman/interaction/messages.hpp"
#include "inman/interaction/MsgBilling.hpp"
#include "inman/interaction/MsgContract.hpp"

namespace smsc  {
namespace inman {
namespace interaction {
/* ************************************************************************** *
 * class INPSerializer implementation:
 * ************************************************************************** */
INPSerializer::INPSerializer()
{
    cmdSets.insert(INPCsMap::value_type(csBilling, INPCSBilling::getInstance()));
    cmdSets.insert(INPCsMap::value_type(csAbntContract, INPCSAbntContract::getInstance()));
}

INPSerializer* INPSerializer::getInstance()
{
    static INPSerializer instance;
    return &instance;
}

INPCommandSetAC * INPSerializer::commandSet(unsigned short cmd_id)
{
    INPCommandSetAC *   cmdSet = NULL;
    INPCommandSetId     csId = csIdbyCmdId(cmd_id);
    if (csId != csUndefined) {
        INPCsMap::iterator cit = cmdSets.find(csId);
        assert(cit != cmdSets.end()); //verify INPSerializer()
        cmdSet = (*cit).second;
    }
    return cmdSet;
}

INPPacketAC* INPSerializer::deserialize(ObjectBuffer& in) throw(SerializerException)
{
    std::auto_ptr<INPPacketAC> pck; 
    INPCommandSetAC * cmdSet = loadHdr(in, pck);     //throws
    loadObj(cmdSet, pck.get(), &in, false);     //throws
    return pck.release();
}

INPPacketAC* INPSerializer::deserialize(std::auto_ptr<ObjectBuffer>& p_in)
                                        throw(SerializerException)
{
    ObjectBuffer * in = p_in.get();
    std::auto_ptr<INPPacketAC> pck;             //throws
    INPCommandSetAC * cmdSet = loadHdr(*in, pck);    //throws
    if (!loadObj(cmdSet, pck.get(), in, true))  //throws
        p_in.release(); //take ownership of ObjectBuffer for deferred deserialization!
    return pck.release();
}

/* -------------------------------------------------------------------------- *
 * Private/Protected methods:
 * -------------------------------------------------------------------------- */
INPCommandSetAC* INPSerializer::loadHdr(ObjectBuffer & in, std::auto_ptr<INPPacketAC> & pck)
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
    INPCommandSetAC * cmdSet = commandSet(cmdId);
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
bool INPSerializer::loadObj(INPCommandSetAC * cmd_set, INPPacketAC * pck,
                            ObjectBuffer * in, bool ownBuf/* = false*/)
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
