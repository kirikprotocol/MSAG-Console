static char const ident[] = "$Id$";
#include <vector>
#include <assert.h>

#include "inman/codec_uss/USSD-Arg.h"
#include "inman/comp/usscomp.hpp"
#include "inman/comp/compsutl.hpp"
#include "inman/common/util.hpp"


typedef std::runtime_error EncodeError;
typedef std::runtime_error DecodeError;

using smsc::inman::comp::CompFactory;
using smsc::inman::comp::OperationFactory;

using smsc::inman::common::format;
using smsc::inman::comp::smsc_log_component;

using smsc::cbs::CBS_DCS;
using smsc::cbs::parseCBS_DCS;
using smsc::cvtutil::unpack7BitPadded2Text;
using smsc::cvtutil::packTextAs7BitPadded;
using smsc::cvtutil::estimateTextAs7Bit;

namespace smsc {
namespace inman {
namespace usscomp {

//FactoryInitFunction implementation
OperationFactory * initMAPUSS2Components(OperationFactory * fact)
{
    if (!fact) { //called from ApplicationContextFactory::getFactory()
        //getInstance() calls FIF in turn
        fact = smsc::ac::MAPUSS2Factory::getInstance();
    } else {
        fact->setLogger(Logger::getInstance("smsc.inman.usscomp.ComponentFactory"));
        fact->registerArg(MAPUSS_OpCode::processUSS_Request,
          new CompFactory::ProducerT<smsc::inman::usscomp::ProcessUSSRequestArg>() );
        fact->registerRes(MAPUSS_OpCode::processUSS_Request,
          new CompFactory::ProducerT<smsc::inman::usscomp::ProcessUSSRequestRes>() );
        fact->bindErrors(MAPUSS_OpCode::processUSS_Request, 5, 
                         ERR_ProcessUSS_Request::callBarred,
                         ERR_ProcessUSS_Request::dataMissing,
                         ERR_ProcessUSS_Request::systemFailure,
                         ERR_ProcessUSS_Request::unexpectedDataValue,
                         ERR_ProcessUSS_Request::unknownAlphabet
                         );
    }
    return fact;
}


/* ************************************************************************** *
 * class MAPUSS2Comp implementation:
 * ************************************************************************** */
unsigned char MAPUSS2Comp::getDCS(void) const
{
    return _dCS;
}

const vector<unsigned char>& MAPUSS2Comp::getUSSData(void) const
{
    return _uSSData;
}

bool  MAPUSS2Comp::getUSSDataAsLatin1Text(std::string & str) const
{
    CBS_DCS    parsedDCS;

    if (parseCBS_DCS(_dCS, parsedDCS) == CBS_DCS::dcGSM7Bit) {
        unsigned ussdLen = unpack7BitPadded2Text(&_uSSData[0], _uSSData.size(), str);
        //skip language prefix
        if (parsedDCS.lngPrefix == CBS_DCS::lng4GSM7Bit)
            str.erase(0, 3);
        return true;
    }
    return false;
}

void MAPUSS2Comp::setUSSData(const unsigned char * data, unsigned size)
{
    unsigned ussdStrSz = estimateTextAs7Bit((const char*)data, size, NULL);
    assert(ussdStrSz <= MAP_MAX_USSD_StringLength);

    uint8_t ussdStr[MAP_MAX_USSD_StringLength];
    ussdStrSz = packTextAs7BitPadded((const char*)data, size, ussdStr);

    _uSSData.clear();
    _uSSData.insert(_uSSData.begin(), ussdStr, ussdStr + ussdStrSz);
    _dCS = 0x0F; //GSM 7-bit, arbitrary language
}

void MAPUSS2Comp::setRAWUSSData(unsigned char dcs, const unsigned char * data, unsigned size)
{
    _uSSData.clear();
    _uSSData.insert(_uSSData.begin(), data, data + size);
    _dCS = dcs;
}


/* ************************************************************************** *
 * class ProcessUSSRequestArg implementation:
 * ************************************************************************** */
ProcessUSSRequestArg::ProcessUSSRequestArg()
{
    compLogger = smsc::logger::Logger::getInstance("smsc.inman.usscomp.ProcessUSSRequestArg");
    _alrt = alertingNotSet;
}
ProcessUSSRequestArg::~ProcessUSSRequestArg() { }

bool ProcessUSSRequestArg::msISDNadr_present(void)
{
    return (_msAdr.value[0] && (_msAdr.value[1] || _msAdr.value[0] != '0')) ? true : false;
}

bool ProcessUSSRequestArg::msAlerting_present(void)
{
    return (_alrt == alertingNotSet) ? false : true;
}

const Address& ProcessUSSRequestArg::getMSISDNadr(void) const
{
    return _msAdr;
}


void ProcessUSSRequestArg::setAlertingPattern(enum AlertingPattern alrt)
{
    _alrt = alrt;
}

enum AlertingPattern ProcessUSSRequestArg::getAlertingPattern(void) const
{
    return _alrt;
}

void ProcessUSSRequestArg::setMSISDNadr(const Address& msadr)
{
    _msAdr = msadr;
}

void ProcessUSSRequestArg::setMSISDNadr(const char* adrStr)
{
    Address  msadr(adrStr);
    _msAdr = msadr;
}


void ProcessUSSRequestArg::decode(const vector<unsigned char>& buf)
{
    USSD_Arg_t *  dcmd = NULL;  /* decoded structure */
    asn_dec_rval_t  drc;    /* Decoder return value  */

    drc = ber_decode(0, &asn_DEF_USSD_Arg, (void **)&dcmd, &buf[0], buf.size());
    ASNCODEC_LOG_DEC(drc, asn_DEF_USSD_Arg, "mapUSS");

    /* decode ussd string */
    assert(dcmd->ussd_DataCodingScheme.size == 1);
    assert(dcmd->ussd_String.size <= MAP_MAX_USSD_StringLength);
    this->setRAWUSSData(dcmd->ussd_DataCodingScheme.buf[0],
                        dcmd->ussd_String.buf, dcmd->ussd_String.size);

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
    asn_enc_rval_t  er;
    /* construct USSD_Arg */
    USSD_Arg_t    cmd;
    /* optionals: */
    unsigned char alrt_buf[1];
    OCTET_STRING_t  alrt;
    uint8_t fdcsbuf;
    uint8_t fussdsbuf[MAP_MAX_USSD_StringLength];

    unsigned char isdn_buf[1 + MAP_MAX_ISDN_AddressLength];
    OCTET_STRING_t  isdn;


    memset(&cmd, 0x00, sizeof(cmd)); //clear optionals and asn_ctx
    cmd.ussd_DataCodingScheme.size = 1;
    cmd.ussd_DataCodingScheme.buf = &fdcsbuf;
    cmd.ussd_DataCodingScheme.buf[0] = _dCS;

    /* prepare ussd string */
    cmd.ussd_String.size = _uSSData.size();
    assert(cmd.ussd_String.size < MAP_MAX_USSD_StringLength);
    cmd.ussd_String.buf = &fussdsbuf[0];
    memcpy(cmd.ussd_String.buf, &_uSSData[0], cmd.ussd_String.size);

    if (_alrt != alertingNotSet) {
        memset(&alrt, 0, sizeof(alrt));
        alrt_buf[0] = (unsigned char)_alrt;
        alrt.buf = alrt_buf;
        alrt.size = 1;
        cmd.alertingPattern = &alrt;
    }

    if (msISDNadr_present()) {
        memset(&isdn, 0, sizeof(isdn));
        isdn.size = packMAPAddress2OCTS(_msAdr, (TONNPI_ADDRESS_OCTS *)isdn_buf);
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
