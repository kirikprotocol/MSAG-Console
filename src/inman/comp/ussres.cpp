static char const ident[] = "$Id$";
#include <vector>
#include <assert.h>

#include "inman/codec_uss/USSD-Res.h"
#include "usscomp.hpp"
#include "compsutl.hpp"
#include "inman/common/util.hpp"


typedef std::runtime_error EncodeError;
typedef std::runtime_error DecodeError;

namespace smsc {
namespace inman {
namespace usscomp {
		    
using smsc::inman::common::format;
using smsc::inman::comp::smsc_log_component;

ProcessUSSRequestRes::ProcessUSSRequestRes()
{
    compLogger = smsc::logger::Logger::getInstance("smsc.mapuss.comp.ProcessUSSRequestRes");
}
ProcessUSSRequestRes::~ProcessUSSRequestRes() { }


const vector<unsigned char>& ProcessUSSRequestRes::getUSSData(void)
{
    return _uSSdata;
}

void ProcessUSSRequestRes::setUSSData(unsigned char * data, unsigned size)
{
    _uSSdata.resize(size);
    _uSSdata.insert(_uSSdata.begin(), data, data + size);
}

void ProcessUSSRequestRes::decode(const vector<unsigned char>& buf)
{
    USSD_Res_t *	dcmd = NULL;	/* decoded structure */
    asn_dec_rval_t	drc;		/* Decoder return value  */

    drc = ber_decode(0, &asn_DEF_USSD_Res, (void **)&dcmd, &buf[0], buf.size());
    ASNCODEC_LOG_DEC(drc, asn_DEF_USSD_Res, "mapUSS");

    assert(dcmd->ussd_DataCodingScheme.size == 1);
    _dCS = dcmd->ussd_DataCodingScheme.buf[0];

    /* decode ussd string */
    this->setUSSData(dcmd->ussd_String.buf, dcmd->ussd_String.size);

    smsc_log_component(compLogger, &asn_DEF_USSD_Res, dcmd);
    asn_DEF_USSD_Res.free_struct(&asn_DEF_USSD_Res, dcmd, 0);
}

void ProcessUSSRequestRes::encode(vector<unsigned char>& buf)
{
    asn_enc_rval_t	er;
    /* construct USSD_Arg */
    USSD_Res_t	 	cmd;


    memset(&cmd, 0x00, sizeof(cmd)); //clear optionals and asn_ctx
    cmd.ussd_DataCodingScheme.size = 1;
    cmd.ussd_DataCodingScheme.buf[0] = _dCS;
    
    /* prepare ussd string */
    cmd.ussd_String.size = _uSSdata.size();
    assert(cmd.ussd_String.size < MAP_MAX_USSD_StringLength);
    memcpy(cmd.ussd_String.buf, &_uSSdata[0], cmd.ussd_String.size);

    smsc_log_component(compLogger, &asn_DEF_USSD_Res, &cmd); 

    er = der_encode(&asn_DEF_USSD_Res, &cmd, print2vec, &buf);
    ASNCODEC_LOG_ENC(er, asn_DEF_USSD_Res, "mapUSS");
}

}//namespace usscomp
}//namespace inman
}//namespace smsc

