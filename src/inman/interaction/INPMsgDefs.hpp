/* ************************************************************************* *
 * INMan Protocols definition.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_INTERACTION_PROTOCOLS__
#ident "@(#)$Id$"
#define __SMSC_INMAN_INTERACTION_PROTOCOLS__

namespace smsc  {
namespace inman {
namespace interaction {

struct INProtocol {
//NOTE: the plain numbering of object Ids through all
//command sets is used for simplicity!
    enum CSId {
        csBilling  = 1, //SMS/USSD Charging protocol, objIds [1..5]
        csAbntContract, //Subscriber contract and gsmSCF parms determination [6..9]
        csSession,      //Session establishment and control, objIds [10..15]
        csControl,      //INMan service control (radmin), objIds [16...] 
        //...
        csUndefined  // just a max cap, reserved
    };

    static CSId csIdbyCmdId(unsigned cmd_id)
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
};


} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_INTERACTION_PROTOCOLS__ */

