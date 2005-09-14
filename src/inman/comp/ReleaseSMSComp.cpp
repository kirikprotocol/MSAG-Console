static char const ident[] = "$Id$";
#include <vector>

#include "ReleaseSMSArg.h"
#include "comps.hpp"
#include "compsutl.hpp"


namespace smsc {
namespace inman {
namespace comp {
using std::vector;


ReleaseSMSArg::ReleaseSMSArg() { rPCause = 0; }
ReleaseSMSArg::~ReleaseSMSArg() { }

void ReleaseSMSArg::decode(const vector<unsigned char>& buf)
{
    RPCause_t *		dcmd = NULL;	/* decoded structure */
    asn_dec_rval_t	drc;		/* Decoder return value  */

    drc = ber_decode(0, &asn_DEF_ReleaseSMSArg, (void **)&dcmd, &buf[0], buf.size());
    INMAN_LOG_DEC(drc, asn_DEF_ReleaseSMSArg);

    rPCause = dcmd->buf[0];
    asn_DEF_ReleaseSMSArg.free_struct(&asn_DEF_ReleaseSMSArg, dcmd, 0);
}

/* this method doesn't required */
//void ReleaseSMSArg::encode(vector<unsigned char>& buf)
//{ throw EncodeError("Not implemented"); }

}//namespace comp
}//namespace inman
}//namespace smsc

