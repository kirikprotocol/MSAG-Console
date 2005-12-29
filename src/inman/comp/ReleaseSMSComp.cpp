static char const ident[] = "$Id$";

#include "ReleaseSMSArg.h"
#include "comps.hpp"
#include "compsutl.hpp"

namespace smsc {
namespace inman {
namespace comp {

ReleaseSMSArg::ReleaseSMSArg()
{
    rPCause = 0;
    compLogger = smsc::logger::Logger::getInstance("smsc.inman.comp.ReleaseSMSArg");
}
ReleaseSMSArg::~ReleaseSMSArg() { }

void ReleaseSMSArg::decode(const vector<unsigned char>& buf) throw(CustomException)
{
    RPCause_t *		dcmd = NULL;	/* decoded structure */
    asn_dec_rval_t	drc;		/* Decoder return value  */

    drc = ber_decode(0, &asn_DEF_ReleaseSMSArg, (void **)&dcmd, &buf[0], buf.size());
    INMAN_LOG_DEC(drc, asn_DEF_ReleaseSMSArg);

    rPCause = dcmd->buf[0];

    //NOTE: there is a deficiency in asn1c printing:
    //it doesn't print name for type defined via primitive type
    if (asn_DEF_ReleaseSMSArg.name)
        smsc_log_debug(compLogger, "%s ::= ", asn_DEF_ReleaseSMSArg.name);
    smsc_log_component(compLogger, &asn_DEF_ReleaseSMSArg, dcmd);
    asn_DEF_ReleaseSMSArg.free_struct(&asn_DEF_ReleaseSMSArg, dcmd, 0);
}

}//namespace comp
}//namespace inman
}//namespace smsc

