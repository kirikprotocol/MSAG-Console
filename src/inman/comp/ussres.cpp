static char const ident[] = "$Id$";
#include <vector>
#include <assert.h>

#include "inman/codec_uss/USSD-Res.h"
#include "usscomp.hpp"
#include "compsutl.hpp"

namespace smsc {
namespace inman {
namespace usscomp {
		    
using smsc::inman::comp::smsc_log_component;


/* ************************************************************************** *
 * class ProcessUSSRequestArg implementation:
 * ************************************************************************** */

ProcessUSSRequestRes::ProcessUSSRequestRes()
{
    compLogger = smsc::logger::Logger::getInstance("smsc.inman.usscomp.ProcessUSSRequestRes");
}
ProcessUSSRequestRes::~ProcessUSSRequestRes() { }


void ProcessUSSRequestRes::decode(const vector<unsigned char>& buf) throw(CustomException)
{
    USSD_Res_t *        dcmd = NULL;    /* decoded structure */
    asn_dec_rval_t      drc;            /* Decoder return value  */

    drc = ber_decode(0, &asn_DEF_USSD_Res, (void **)&dcmd, &buf[0], buf.size());
    ASNCODEC_LOG_DEC(drc, asn_DEF_USSD_Res, "mapUSS");

    /* decode ussd string */
    assert(dcmd->ussd_DataCodingScheme.size == 1);
    this->setRAWUSSData(dcmd->ussd_DataCodingScheme.buf[0],
                        dcmd->ussd_String.buf, dcmd->ussd_String.size);

    smsc_log_component(compLogger, &asn_DEF_USSD_Res, dcmd);
    asn_DEF_USSD_Res.free_struct(&asn_DEF_USSD_Res, dcmd, 0);
    if (_uSSData.size() > MAP_MAX_USSD_StringLength)
        smsc_log_warn(compLogger, "ProcessUSSRequestRes: ussdata size is too large: %u",
                      _uSSData.size());
}

void ProcessUSSRequestRes::encode(vector<unsigned char>& buf) throw(CustomException)
{
    asn_enc_rval_t      er;
    /* construct USSD_Arg */
    USSD_Res_t          cmd;
    uint8_t             fdcsbuf;
    uint8_t             fussdsbuf[MAP_MAX_USSD_StringLength];


    memset(&cmd, 0x00, sizeof(cmd)); //clear optionals and asn_ctx
    cmd.ussd_DataCodingScheme.size = 1;
    cmd.ussd_DataCodingScheme.buf = &fdcsbuf;
    cmd.ussd_DataCodingScheme.buf[0] = _dCS;
    
    /* prepare ussd string */
    if (_uSSData.size() > MAP_MAX_USSD_StringLength)
        throw CustomException("ProcessUSSRequestRes: ussdata size is too large",
                              _uSSData.size(), NULL);

    cmd.ussd_String.size = _uSSData.size();
    cmd.ussd_String.buf = &fussdsbuf[0];
    memcpy(cmd.ussd_String.buf, &_uSSData[0], cmd.ussd_String.size);

    smsc_log_component(compLogger, &asn_DEF_USSD_Res, &cmd); 

    er = der_encode(&asn_DEF_USSD_Res, &cmd, print2vec, &buf);
    ASNCODEC_LOG_ENC(er, asn_DEF_USSD_Res, "mapUSS");
}

}//namespace usscomp
}//namespace inman
}//namespace smsc

