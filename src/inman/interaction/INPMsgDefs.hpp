/* ************************************************************************* *
 * INMan Protocols definition.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_INTERACTION_PROTOCOLS__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INTERACTION_PROTOCOLS__

namespace smsc  {
namespace inman {
namespace interaction {

struct INProtocol {
//NOTE: the plain numbering of object Ids through all
//command sets is used just for simplicity!
  enum CSId {
    csUndefined = 0
    , csBilling       //SMS/USSD Charging protocol, objIds [1..5]
    , csAbntContract  //Subscriber contract and gsmSCF parms determination [6..9]
//  , csSession,      //Session establishment and control, objIds [10..15]
//  , csAdmin,        //INMan service administration, objIds [16...] 
    //...
    , csReserved      // just a max cap, reserved
  };

  struct CSParams {   //protocol(command set) parameters
    unsigned      maxCmdId;
    const char *  nmProto;

    explicit CSParams(unsigned max_cmd = 0, const char * nm_proto = NULL)
      : maxCmdId(max_cmd), nmProto(nm_proto)
    { }
  };

  static const CSParams _csParm[];

  static const char * csName(CSId cs_id)
  {
    return (cs_id < csReserved) ? _csParm[cs_id].nmProto : _csParm[0].nmProto;
  }
  static CSId csIdbyCmdId(unsigned cmd_id)
  {
    for (unsigned csId = csUndefined; csId < csReserved; ++csId) {
      if (cmd_id <= _csParm[csId].maxCmdId)
        return static_cast<CSId>(csId);
    }
    return csUndefined;
  }

  static const char * csNamebyCmdId(unsigned cmd_id)
  {
    return csName(csIdbyCmdId(cmd_id));
  }
};


} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_INTERACTION_PROTOCOLS__ */

