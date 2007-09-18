#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include <vector>
#include <assert.h>

#include "inman/codec_inc/map/USSD-Arg.h"
#include "inman/comp/map_uss/MapUSSComps.hpp"
#include "inman/comp/compsutl.hpp"
#include "util/vformat.hpp"
using smsc::util::format;

using smsc::inman::comp::smsc_log_component;
using smsc::inman::comp::OCTET_STRING_2_Address;

using smsc::cbs::CBS_DCS;
using smsc::cbs::parseCBS_DCS;
using smsc::cvtutil::unpack7BitPadded2Text;
using smsc::cvtutil::packTextAs7BitPadded;
using smsc::cvtutil::estimateTextAs7Bit;

namespace smsc {
namespace inman {
namespace comp {
namespace uss {
/* ************************************************************************** *
 * class MAPUSS2CompAC implementation:
 * ************************************************************************** */
bool  MAPUSS2CompAC::getUSSDataAsLatin1Text(std::string & str) const
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

void MAPUSS2CompAC::setUSSData(const unsigned char * data, unsigned size/* = 0*/) throw(CustomException)
{
    if (!size) 
        size = strlen((const char*)data);
    unsigned ussdStrSz = estimateTextAs7Bit((const char*)data, size, NULL);
    if (ussdStrSz > MAP_MAX_USSD_StringLength)
        throw CustomException("MAPUSS2CompAC: ussdStrSz is too large: %u", ussdStrSz);

    uint8_t ussdStr[MAP_MAX_USSD_StringLength];
    ussdStrSz = packTextAs7BitPadded((const char*)data, size, ussdStr);

    _uSSData.clear();
    _uSSData.insert(_uSSData.begin(), ussdStr, ussdStr + ussdStrSz);
    _dCS = 0x0F; //GSM 7-bit, arbitrary language
}

void MAPUSS2CompAC::setRAWUSSData(unsigned char dcs, const unsigned char * data, unsigned size)
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
    compLogger = smsc::logger::Logger::getInstance("smsc.inman.comp.uss");
    _alrt = alertingNotSet;
}

bool ProcessUSSRequestArg::msISDNadr_present(void) const
{
    return (_msAdr.signals[0] && (_msAdr.signals[1] || _msAdr.signals[0] != '0')) ? true : false;
}

bool ProcessUSSRequestArg::msAlerting_present(void) const
{
    return (_alrt == alertingNotSet) ? false : true;
}

void ProcessUSSRequestArg::setMSISDNadr(const char* adrStr) throw(CustomException)
{
    if (!_msAdr.fromText(adrStr))
        throw CustomException(-1, "ProcessUSSRequestArg: invalid msisdn", adrStr);
}

void ProcessUSSRequestArg::decode(const std::vector<unsigned char>& buf) throw(CustomException)
{
    USSD_Arg_t *  dcmd = NULL;  /* decoded structure */
    asn_dec_rval_t  drc;    /* Decoder return value  */

    drc = ber_decode(0, &asn_DEF_USSD_Arg, (void **)&dcmd, &buf[0], buf.size());
    ASNCODEC_LOG_DEC(drc, asn_DEF_USSD_Arg, "mapUSS");
    smsc_log_component(compLogger, &asn_DEF_USSD_Arg, dcmd);

    /* decode ussd string */
    assert(dcmd->ussd_DataCodingScheme.size == 1);
    this->setRAWUSSData(dcmd->ussd_DataCodingScheme.buf[0],
                        dcmd->ussd_String.buf, dcmd->ussd_String.size);

    if (dcmd->msisdn) {
        if (!OCTET_STRING_2_Address(dcmd->msisdn, _msAdr))
            smsc_log_error(compLogger, "ProcessUSSRequestArg: bad msISDN adr");
    } else
        _msAdr.clear();

    if (dcmd->alertingPattern) {
        assert(dcmd->alertingPattern->size == 1);
        _alrt = (AlertingPattern_e)(dcmd->alertingPattern->buf[0]);
    } else
        _alrt = alertingNotSet;
    
    asn_DEF_USSD_Arg.free_struct(&asn_DEF_USSD_Arg, dcmd, 0);

    //check for constraints not specified in ASN.1 notation
    if (_uSSData.size() > MAP_MAX_USSD_StringLength)
        smsc_log_warn(compLogger, "ProcessUSSRequestArg: ussdStrSz is too large: %u",
                      _uSSData.size());
}

void ProcessUSSRequestArg::encode(std::vector<unsigned char>& buf) const throw(CustomException)
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
    if (_uSSData.size() > MAP_MAX_USSD_StringLength)
        throw CustomException("ProcessUSSRequestArg: ussdata size is too large: %u",
                              _uSSData.size());

    cmd.ussd_String.size = _uSSData.size();
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

} //uss
}//namespace comp
}//namespace inman
}//namespace smsc
