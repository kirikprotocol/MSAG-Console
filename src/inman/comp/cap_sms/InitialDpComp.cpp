#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/asn1rt/asn_internal.h"
#include "inman/codec_inc/cap/InitialDPSMSArg.h"
#include "inman/comp/cap_sms/CapSMSComps.hpp"
#include "inman/comp/compsutl.hpp"
#include "inman/common/adrutil.hpp"
#include "inman/common/cvtutil.hpp"

using smsc::util::MAPConst;

using smsc::cvtutil::packTimeT2BCD7;
using smsc::cvtutil::packTimeT2BCD8;
using smsc::cvtutil::packTP_VP_Relative;
using smsc::cvtutil::packNumString2BCD;
using smsc::cvtutil::packMAPAddress2OCTS;
using smsc::cvtutil::packMAPAddress2LocationOCTS;
using smsc::cvtutil::TONNPI_ADDRESS_OCTS;
using smsc::cvtutil::LOCATION_ADDRESS_OCTS;


#define OCTET_STRING_DECL(name, szo) unsigned char name##_buf[szo]; OCTET_STRING_t name
#define ZERO_OCTET_STRING(name)	{ memset(&name, 0, sizeof(name)); name.buf = name##_buf; }

#define Address2OCTET_STRING(octs, addr)	{ ZERO_OCTET_STRING(octs); \
	octs.size = packMAPAddress2OCTS(addr, octs.buf); }


namespace smsc {
namespace inman {
namespace comp {

class PrivateInitialDPSMSArg {
public:
    InitialDPSMSArg_t	idp;
    EventTypeSMS_t	_dlvrMode;
    OCTET_STRING_DECL(_destSN, sizeof(TONNPI_ADDRESS_OCTS));
    OCTET_STRING_DECL(_clldPN, sizeof(TONNPI_ADDRESS_OCTS));
    OCTET_STRING_DECL(_clngPN, sizeof(TONNPI_ADDRESS_OCTS));
    OCTET_STRING_DECL(_iMSI, MAPConst::MAX_IMSI_AddressLength);
    OCTET_STRING_DECL(_adrSMSC, sizeof(TONNPI_ADDRESS_OCTS));
    OCTET_STRING_DECL(_tmTz, CAPConst::MAX_TimeAndTimezoneLength);
    OCTET_STRING_DECL(_tPSMSI, 1);
    OCTET_STRING_DECL(_tPPid, 1);
    OCTET_STRING_DECL(_tPDCSch, 1);
    OCTET_STRING_DECL(_tPVP, 7); // 1 or 7 bytes
    //Location Information members
    LocationInformation_t	_li;
    OCTET_STRING_DECL(_vlrNumber, sizeof(TONNPI_ADDRESS_OCTS));
    OCTET_STRING_DECL(_locNumber, sizeof(LOCATION_ADDRESS_OCTS));
    CellGlobalIdOrServiceAreaIdOrLAI_t	_cGidOrSAIorLAI;
    unsigned char			_cGidOrSAIfl[7];

    PrivateInitialDPSMSArg(void)
    {
        memset(&idp, 0, sizeof(idp)); //clear _asn_ctx & optionals
    }
    ~PrivateInitialDPSMSArg()
    { }

    void setIDPParms(DeliveryMode_e idpMode, unsigned int serviceKey)
    {
        idp.serviceKey = serviceKey;
        _dlvrMode = (EventTypeSMS_t)idpMode;
        idp.eventTypeSMS = &_dlvrMode;
    }
};

SMSInitialDPArg::SMSInitialDPArg(Logger * use_log/* = NULL*/)
    : compLogger(use_log ? use_log : Logger::getInstance("smsc.inman.comp.IDPSmsArg"))
    , servKey(0), comp(new PrivateInitialDPSMSArg())
{ }

SMSInitialDPArg::~SMSInitialDPArg()
{
    delete(comp);
}

void SMSInitialDPArg::setIDPParms(DeliveryMode_e idpMode, unsigned int serviceKey)
{
    comp->setIDPParms(idpMode, servKey = serviceKey);
}

/*
 * NOTE: The CAP4 specifies that the packed address in DestinationSubscriberNumber
 * may be up to 40 octets long, though actually, it doesn't exceed the SMS-AddressString
 * length, which is 10 octet. That's why the TonNpiAddress structure is used and 
 * calling the packMAPAddress2OCTS() is safe.
 */
void SMSInitialDPArg::setDestinationSubscriberNumber(const TonNpiAddress& addr)
{
    Address2OCTET_STRING(comp->_destSN, addr);
    comp->idp.destinationSubscriberNumber = &(comp->_destSN);
}

void SMSInitialDPArg::setDestinationSubscriberNumber(const char * text)
{
    TonNpiAddress   sadr;
    if (sadr.fromText(text))
        SMSInitialDPArg::setDestinationSubscriberNumber((const TonNpiAddress&)sadr);
}

void SMSInitialDPArg::setCalledPartyNumber(const TonNpiAddress& addr)
{
    Address2OCTET_STRING(comp->_clldPN, addr);
    comp->idp.calledPartyNumber = &(comp->_clldPN);
}

void SMSInitialDPArg::setCalledPartyNumber(const char * text)
{
    TonNpiAddress   sadr;
    if (sadr.fromText(text))
        SMSInitialDPArg::setCalledPartyNumber((const TonNpiAddress&)sadr);
}

void SMSInitialDPArg::setCallingPartyNumber(const TonNpiAddress& addr)
{
    Address2OCTET_STRING(comp->_clngPN, addr);
    comp->idp.callingPartyNumber = &(comp->_clngPN);
}

void SMSInitialDPArg::setCallingPartyNumber(const char * text)
{
    TonNpiAddress   sadr;
    if (sadr.fromText(text))
        SMSInitialDPArg::setCallingPartyNumber((const TonNpiAddress&)sadr);
}

//imsi contains sequence of ASCII digits
void SMSInitialDPArg::setIMSI(const std::string& imsi) throw(CustomException)
{
    //smsc_log_debug( compLogger, "IMSI: %s (%d)", imsi.c_str(), imsi.length());
    if (((imsi.length() + 1)/2) > MAPConst::MAX_IMSI_AddressLength)
        throw CustomException("IDPMSArg: IMSI length is too long: %u", imsi.length());

    ZERO_OCTET_STRING(comp->_iMSI);
    comp->_iMSI.size = packNumString2BCD(comp->_iMSI.buf, imsi.c_str(), (unsigned)imsi.length());
    comp->idp.iMSI = &(comp->_iMSI);
}


void SMSInitialDPArg::setSMSCAddress(const TonNpiAddress& addr)
{
    Address2OCTET_STRING(comp->_adrSMSC, addr);
    comp->idp.sMSCAddress = &(comp->_adrSMSC);
}

void SMSInitialDPArg::setSMSCAddress(const char * text)
{
    TonNpiAddress   sadr;
    if (sadr.fromText(text))
        SMSInitialDPArg::setSMSCAddress((const TonNpiAddress&)sadr);
}

//requires the preceeding call of tzset()
void SMSInitialDPArg::setTimeAndTimezone(time_t tmVal) throw(CustomException)
{
    ZERO_OCTET_STRING(comp->_tmTz);
    int res = packTimeT2BCD8((unsigned char (*)[8])(comp->_tmTz.buf), tmVal);
    if (res)
        throw CustomException(res, "IDPSmsArg: bad timeTZ");
    comp->_tmTz.size = 8;
    //smsc_log_debug( compLogger, "BCD time: %s", dump(comp->_tmTz.size, comp->_tmTz.buf).c_str() );
    comp->idp.timeAndTimezone = &(comp->_tmTz);
}

void SMSInitialDPArg::setTPShortMessageSpecificInfo(unsigned char tPSMSI)
{
    ZERO_OCTET_STRING(comp->_tPSMSI);
    comp->_tPSMSI.buf[0] = tPSMSI;
    comp->_tPSMSI.size = 1;
    comp->idp.tPShortMessageSpecificInfo = &(comp->_tPSMSI);
    //smsc_log_debug( compLogger, "TPSMSInfo: %x", (unsigned)(comp->idp.tPShortMessageSpecificInfo->buf[0]));
}

void SMSInitialDPArg::setTPProtocolIdentifier(unsigned char tPPid)
{
    ZERO_OCTET_STRING(comp->_tPPid);
    comp->_tPPid.buf[0] = tPPid;
    comp->_tPPid.size = 1;
    comp->idp.tPProtocolIdentifier = &(comp->_tPPid);
    //smsc_log_debug( compLogger, "TPProtoId: %x", (unsigned)(comp->idp.tPProtocolIdentifier->buf[0]));
}

void SMSInitialDPArg::setTPDataCodingScheme(unsigned char tPDCSch)
{
    ZERO_OCTET_STRING(comp->_tPDCSch);
    comp->_tPDCSch.buf[0] = tPDCSch;
    comp->_tPDCSch.size = 1;
    comp->idp.tPDataCodingScheme = &(comp->_tPDCSch);
    //smsc_log_debug( compLogger, "TPDCS: %x", (unsigned)(comp->idp.tPDataCodingScheme->buf[0]));
}

//requires the preceeding call of tzset()
void SMSInitialDPArg::setTPValidityPeriod(time_t vpVal, enum TP_VP_format fmt) throw(CustomException)
{
    ZERO_OCTET_STRING(comp->_tPVP);
    switch (fmt) {
    case tp_vp_relative: {
	comp->_tPVP.size = 1;
	comp->_tPVP.buf[0] = packTP_VP_Relative(vpVal);
	//smsc_log_debug( compLogger, "TP-VP: %x", (unsigned)(comp->_tPVP.buf[0]));
    }   break;
    case tp_vp_absolute: {
        int res;
	comp->_tPVP.size = 7;
	if ((res = packTimeT2BCD7((unsigned char (*)[7])(comp->_tPVP.buf), vpVal)) != 0)
	    throw CustomException(res, "IDPSmsArg: bad time value");
    }   break;
    default:
	throw CustomException("IDPSmsArg: unsupported TP-VP format: %u", (unsigned)fmt);
    }
    comp->idp.tPValidityPeriod = &(comp->_tPVP);
}

/* Sets VLR number and LocationNumber (duplicates it from VLR) */
void SMSInitialDPArg::setLocationInformationMSC(const TonNpiAddress& sadr) throw(CustomException)
{
    TonNpiAddress addr = sadr;
    /* NOTE: _vlrNumber may be only the ISDN INTERNATIONAL address */
    if (!addr.fixISDN())
        throw CustomException(-1, "IDPSmsArg: invalid VLR address",
                              addr.toString().c_str());

    memset(&(comp->_li), 0, sizeof(comp->_li)); //reset _asn_ctx & optionals
    Address2OCTET_STRING(comp->_vlrNumber, addr);
    comp->_li.vlr_number = &(comp->_vlrNumber);
    /**/
    ZERO_OCTET_STRING(comp->_locNumber);
    comp->_locNumber.size = packMAPAddress2LocationOCTS(addr, comp->_locNumber.buf);
    if (comp->_locNumber.size)
        comp->_li.locationNumber = &(comp->_locNumber);
    /**/
    //reset _asn_ctx & optionals
    memset(&(comp->_cGidOrSAIorLAI), 0, sizeof(comp->_cGidOrSAIorLAI));
/*
    comp->_cGidOrSAIorLAI.present =
        CellGlobalIdOrServiceAreaIdOrLAI_PR_cellGlobalIdOrServiceAreaIdFixedLength;

    static const uint8_t _cgidorsaifl1_buf[] = { 0x52, 0xf0, 0x10, 0x97, 0xFE, 0x13, 0x89 };

    comp->_cGidOrSAIorLAI.choice.cellGlobalIdOrServiceAreaIdFixedLength.size = 7;
    comp->_cGidOrSAIorLAI.choice.cellGlobalIdOrServiceAreaIdFixedLength.buf  =
								comp->_cGidOrSAIfl;
    memcpy(comp->_cGidOrSAIorLAI.choice.cellGlobalIdOrServiceAreaIdFixedLength.buf,
							_cgidorsaifl1_buf, 7);
    comp->_li.cellGlobalIdOrServiceAreaIdOrLAI = &(comp->_cGidOrSAIorLAI);
*/
    /**/
    comp->idp.locationInformationMSC = &(comp->_li);
}

void SMSInitialDPArg::setLocationInformationMSC(const char* text) throw(CustomException)
{
    TonNpiAddress   sadr;
    if (!sadr.fromText(text))
        throw CustomException(-1, "IDPSmsArg: invalid VLR address",
                              sadr.toString().c_str());
    SMSInitialDPArg::setLocationInformationMSC(sadr);
}

void SMSInitialDPArg::encode(std::vector<unsigned char>& buf) const throw(CustomException)
{
    asn_enc_rval_t er;

    //debug: print structure content
    smsc_log_component(compLogger, &asn_DEF_InitialDPSMSArg, &comp->idp); 
    er = der_encode(&asn_DEF_InitialDPSMSArg, &comp->idp, print2vec, &buf);
    ASNCODEC_LOG_ENC(er, asn_DEF_InitialDPSMSArg, "IDPSmsArg");
}

}//namespace comp
}//namespace inman
}//namespace smsc

