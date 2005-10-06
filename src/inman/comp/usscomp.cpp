static char const ident[] = "$Id$";
#include <vector>
#include <assert.h>

#include "inman/codec_uss/USSD-Arg.h"
#include "usscomp.hpp"
#include "compsutl.hpp"
#include "inman/common/util.hpp"


typedef std::runtime_error EncodeError;
typedef std::runtime_error DecodeError;

using smsc::inman::comp::CompFactory;
using smsc::inman::comp::OperationFactory;

using smsc::inman::common::format;
using smsc::inman::comp::smsc_log_component;


namespace smsc {
namespace inman {
namespace usscomp {

//FactoryInitFunction implementation
void initMAPUSS2Components(OperationFactory * fact)
{
    fact->setLogger(Logger::getInstance("smsc.inman.usscomp.ComponentFactory"));
    fact->registerArg(MAPUSS_OpCode::processUSS_Request,
	    new CompFactory::ProducerT<smsc::inman::usscomp::ProcessUSSRequestArg>() );
    fact->registerRes(MAPUSS_OpCode::processUSS_Request,
	    new CompFactory::ProducerT<smsc::inman::usscomp::ProcessUSSRequestRes>() );
}



ProcessUSSRequestArg::ProcessUSSRequestArg()
{
    compLogger = smsc::logger::Logger::getInstance("smsc.inman.usscomp.ProcessUSSRequestArg");
    _alrt = alertingNotSet;
}
ProcessUSSRequestArg::~ProcessUSSRequestArg() { }

int ProcessUSSRequestArg::msISDNadr_present(void)
{
    return (_msAdr.value[0] && (_msAdr.value[1] || _msAdr.value[0] != '0'));
}

const Address& ProcessUSSRequestArg::getMSISDNadr(void)
{
    return _msAdr;
}

const vector<unsigned char>& ProcessUSSRequestArg::getUSSData(void)
{
    return _uSSdata;
}

void ProcessUSSRequestArg::setUSSData(unsigned char * data, unsigned size)
{
    _uSSdata.resize(size);
    _uSSdata.insert(_uSSdata.begin(), data, data + size);
}

void ProcessUSSRequestArg::setAlertingPattern(enum AlertingPattern alrt)
{
    _alrt = alrt;
}

void ProcessUSSRequestArg::setMSISDNadr(const Address& msadr)
{
    _msAdr = msadr;
}

void ProcessUSSRequestArg::decode(const vector<unsigned char>& buf)
{
    USSD_Arg_t *	dcmd = NULL;	/* decoded structure */
    asn_dec_rval_t	drc;		/* Decoder return value  */

    drc = ber_decode(0, &asn_DEF_USSD_Arg, (void **)&dcmd, &buf[0], buf.size());
    ASNCODEC_LOG_DEC(drc, asn_DEF_USSD_Arg, "mapUSS");

    assert(dcmd->ussd_DataCodingScheme.size == 1);
    _dCS = dcmd->ussd_DataCodingScheme.buf[0];

    /* decode ussd string */
    this->setUSSData(dcmd->ussd_String.buf, dcmd->ussd_String.size);

    if (dcmd->msisdn) {
	_msAdr = smsc::inman::comp::OCTET_STRING_2_Addres(dcmd->msisdn);
	assert(_msAdr.length < MAP_MAX_ISDN_AddressLength);
    } else
	_msAdr.setValue(0, NULL);

    if (dcmd->alertingPattern) {
	assert(dcmd->alertingPattern->size == 1);
	_alrt = (AlertingPattern_e)(dcmd->alertingPattern->buf[0]);
    } else
	_alrt = alertingNotSet;

    smsc_log_component(compLogger, &asn_DEF_USSD_Arg, dcmd);
    asn_DEF_USSD_Arg.free_struct(&asn_DEF_USSD_Arg, dcmd, 0);
}

void ProcessUSSRequestArg::encode(vector<unsigned char>& buf)
{
    asn_enc_rval_t	er;
    /* construct USSD_Arg */
    USSD_Arg_t	 	cmd;
    /* optionals: */
    unsigned char	alrt_buf[1];
    OCTET_STRING_t	alrt;

    unsigned char	isdn_buf[1 + MAP_MAX_ISDN_AddressLength];
    OCTET_STRING_t	isdn;


    memset(&cmd, 0x00, sizeof(cmd)); //clear optionals and asn_ctx
    cmd.ussd_DataCodingScheme.size = 1;
    cmd.ussd_DataCodingScheme.buf[0] = _dCS;
    
    /* prepare ussd string */
    cmd.ussd_String.size = _uSSdata.size();
    assert(cmd.ussd_String.size < MAP_MAX_USSD_StringLength);
    memcpy(cmd.ussd_String.buf, &_uSSdata[0], cmd.ussd_String.size);

    if (_alrt != alertingNotSet) {
	memset(&alrt, 0, sizeof(alrt));
	alrt_buf[0] = (unsigned char)_alrt;
	alrt.buf = alrt_buf;
	alrt.size = 1;
	cmd.alertingPattern = &alrt;
    }

    if (msISDNadr_present()) {
	memset(&isdn, 0, sizeof(isdn));
	isdn.size = packMAPAddress2OCTS(_msAdr, (smsc::inman::comp::TONNPI_ADDRESS_OCTS *)isdn_buf);
	isdn.buf = isdn_buf;
	cmd.msisdn = &isdn;
    }

    smsc_log_component(compLogger, &asn_DEF_USSD_Arg, &cmd); 

    er = der_encode(&asn_DEF_USSD_Arg, &cmd, print2vec, &buf);
    ASNCODEC_LOG_ENC(er, asn_DEF_USSD_Arg, "mapUSS");
}

}//namespace usscomp
}//namespace inman
}//namespace smsc

