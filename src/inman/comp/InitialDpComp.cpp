static char const ident[] = "$Id$";
#include <vector>
#include <time.h>

#include "inman/codec/asn_internal.h"
#include "inman/codec/InitialDPSMSArg.h"
#include "comps.hpp"
#include "compsutl.hpp"
#include "inman/common/util.hpp"
using smsc::inman::common::dump;

#define OCTET_STRING_DECL(name, szo) unsigned char name##_buf[szo]; OCTET_STRING_t name
#define ZERO_OCTET_STRING(name)	{ memset(&name, 0, sizeof(name)); name.buf = name##_buf; }

#define Address2OCTET_STRING(octs, addr)	{ ZERO_OCTET_STRING(octs); \
	octs.size = packMAPAddress2OCTS(addr, (TONNPI_ADDRESS_OCTS *)(octs.buf)); }


namespace smsc {
namespace inman {
namespace comp {
using std::vector;


class PrivateInitialDPSMSArg
{
  public:
    PrivateInitialDPSMSArg(DeliveryMode_e idpMode);
    ~PrivateInitialDPSMSArg();

    InitialDPSMSArg_t	idp;
    EventTypeSMS_t	_dlvrMode;
    OCTET_STRING_DECL(_destSN, sizeof(TONNPI_ADDRESS_OCTS));
    OCTET_STRING_DECL(_clldPN, sizeof(TONNPI_ADDRESS_OCTS));
    OCTET_STRING_DECL(_clngPN, sizeof(TONNPI_ADDRESS_OCTS));
    OCTET_STRING_DECL(_iMSI, CAP_MAX_IMSILength);
    OCTET_STRING_DECL(_adrSMSC, sizeof(TONNPI_ADDRESS_OCTS));
    OCTET_STRING_DECL(_tmTz, CAP_MAX_TimeAndTimezoneLength);
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
};

PrivateInitialDPSMSArg::PrivateInitialDPSMSArg(DeliveryMode_e idpMode)
{
    memset(&idp, 0, sizeof(idp)); //clear _asn_ctx & optionals
    idp.serviceKey = 11;

    _dlvrMode = (EventTypeSMS_t)idpMode;
    idp.eventTypeSMS = &_dlvrMode;
}

PrivateInitialDPSMSArg::~PrivateInitialDPSMSArg() { }


/*
 * NOTE: The CAP4 specifies that the packed address in DestinationSubscriberNumber
 * may be up to 40 octets long, though actually, it doesn't exceed the SMS-AddressString
 * length, which is 10 octet. That's why the Address structure is used and 
 * calling the packMAPAddress2OCTS() is safe.
 */
void InitialDPSMSArg::setDestinationSubscriberNumber(const Address& addr)
{
    Address2OCTET_STRING(comp->_destSN, addr);
    comp->idp.destinationSubscriberNumber = &(comp->_destSN);
}

void InitialDPSMSArg::setDestinationSubscriberNumber(const char * text)
{
    Address	sadr(text);
    InitialDPSMSArg::setDestinationSubscriberNumber((const Address&)sadr);
}

void InitialDPSMSArg::setCalledPartyNumber(const Address& addr)
{
    Address2OCTET_STRING(comp->_clldPN, addr);
    comp->idp.calledPartyNumber = &(comp->_clldPN);
}

void InitialDPSMSArg::setCalledPartyNumber(const char * text)
{
    Address	sadr(text);
    InitialDPSMSArg::setCalledPartyNumber((const Address&)sadr);
}

void InitialDPSMSArg::setCallingPartyNumber(const Address& addr)
{
    Address2OCTET_STRING(comp->_clngPN, addr);
    comp->idp.callingPartyNumber = &(comp->_clngPN);
}

void InitialDPSMSArg::setCallingPartyNumber(const char * text)
{
    Address	sadr(text);
    InitialDPSMSArg::setCallingPartyNumber((const Address&)sadr);
}

//imsi contains sequence of ASCII digits
void InitialDPSMSArg::setIMSI(const std::string& imsi)
{
	
	smsc_log_debug( compLogger, "IMSI: %s (%d)", imsi.c_str(), imsi.length());
    assert(((imsi.length() + 1)/2) <= CAP_MAX_IMSILength);
    ZERO_OCTET_STRING(comp->_iMSI);
    comp->_iMSI.size = smsc::cvtutil::packNumString2BCD(comp->_iMSI.buf,
						imsi.c_str(), imsi.length());
    comp->idp.iMSI = &(comp->_iMSI);
}


void InitialDPSMSArg::setSMSCAddress(const Address& addr)
{
    Address2OCTET_STRING(comp->_adrSMSC, addr);
    comp->idp.sMSCAddress = &(comp->_adrSMSC);
}

void InitialDPSMSArg::setSMSCAddress(const char * text)
{
    Address	sadr(text);
    InitialDPSMSArg::setSMSCAddress((const Address&)sadr);
}


void InitialDPSMSArg::setTimeAndTimezone(time_t tmVal)
{
	struct tm *ptime;

	assert( tmVal );

	tzset();
    ptime = localtime( &tmVal );
    assert( ptime );

    char buff[32];

    sprintf( buff, "%04d%02d%02d%02d%02d%02d00",
    		ptime->tm_year + 1900,
    		ptime->tm_mon + 1,
    		ptime->tm_mday,
    		ptime->tm_hour,
    		ptime->tm_min,
    		ptime->tm_sec );

    smsc_log_debug( compLogger, "GMT time: %s", buff );


    ZERO_OCTET_STRING(comp->_tmTz);
    comp->_tmTz.size = smsc::cvtutil::packNumString2BCD(comp->_tmTz.buf, buff, strlen(buff));
    assert(comp->_tmTz.size == CAP_MAX_TimeAndTimezoneLength);
    comp->_tmTz.buf[comp->_tmTz.size - 1] = 0x42; // daylight saving is not counted !!!
    smsc_log_debug( compLogger, "BCD time: %s", dump(comp->_tmTz.size, comp->_tmTz.buf).c_str() );
    comp->idp.timeAndTimezone = &(comp->_tmTz);
}

void InitialDPSMSArg::setTPShortMessageSpecificInfo(unsigned char tPSMSI)
{
    ZERO_OCTET_STRING(comp->_tPSMSI);
    comp->_tPSMSI.buf[0] = tPSMSI;
    comp->idp.tPShortMessageSpecificInfo = &(comp->_tPSMSI);
}

void InitialDPSMSArg::setTPProtocolIdentifier(unsigned char tPPid)
{
    ZERO_OCTET_STRING(comp->_tPPid);
    comp->_tPPid.buf[0] = tPPid;
    comp->idp.tPProtocolIdentifier = &(comp->_tPPid);
}

void InitialDPSMSArg::setTPDataCodingScheme(unsigned char tPDCSch)
{
    ZERO_OCTET_STRING(comp->_tPDCSch);
    comp->_tPDCSch.buf[0] = tPDCSch;
    comp->idp.tPDataCodingScheme = &(comp->_tPDCSch);
}

void InitialDPSMSArg::setTPValidityPeriod(time_t vpVal, enum TP_VP_format fmt)
{
    switch (fmt) {
    case tp_vp_relative: {
	comp->_tPVP.size = 1;
	comp->_tPVP.buf[0] = smsc::cvtutil::packTP_VP_Relative(vpVal);
    }   break;
    case tp_vp_absolute: {
	comp->_tPVP.size = 7;
	if (smsc::cvtutil::packTimeT2BCD7((unsigned char (*)[7])(comp->_tPVP.buf), vpVal))
	    throw runtime_error(":INMAN: bad time value");
    }   break;
    default: //unsupported
	throw runtime_error(":INMAN: unsupported TP-VP format");
    }
    comp->idp.tPValidityPeriod = &(comp->_tPVP);
}

/* Sets VLR number and LocationNumber (duplicates it from VLR) */
void InitialDPSMSArg::setlocationInformationMSC(const Address& addr)
{
    memset(&(comp->_li), 0, sizeof(comp->_li)); //reset _asn_ctx & optionals

    /* NOTE: Address cann't be the alphanumeric */
    Address2OCTET_STRING(comp->_vlrNumber, addr);
    comp->_li.vlr_number = &(comp->_vlrNumber);
    /**/
    ZERO_OCTET_STRING(comp->_locNumber);
    comp->_locNumber.size = packMAPAddress2LocationOCTS(addr,
			    (LOCATION_ADDRESS_OCTS *)(comp->_locNumber.buf));
    comp->_li.locationNumber = &(comp->_locNumber);
    /**/
    //reset _asn_ctx & optionals
    memset(&(comp->_cGidOrSAIorLAI), 0, sizeof(comp->_cGidOrSAIorLAI));
    comp->_cGidOrSAIorLAI.present =
        CellGlobalIdOrServiceAreaIdOrLAI_PR_cellGlobalIdOrServiceAreaIdFixedLength;

    static const uint8_t _cgidorsaifl1_buf[] = { 0x52, 0xf0, 0x10, 0x97, 0xFE, 0x13, 0x89 };

    comp->_cGidOrSAIorLAI.choice.cellGlobalIdOrServiceAreaIdFixedLength.size = 7;
    comp->_cGidOrSAIorLAI.choice.cellGlobalIdOrServiceAreaIdFixedLength.buf  =
								comp->_cGidOrSAIfl;
    memcpy(comp->_cGidOrSAIorLAI.choice.cellGlobalIdOrServiceAreaIdFixedLength.buf,
							_cgidorsaifl1_buf, 7);
    comp->_li.cellGlobalIdOrServiceAreaIdOrLAI = &(comp->_cGidOrSAIorLAI);
    /**/
    comp->idp.locationInformationMSC = &(comp->_li);
}


InitialDPSMSArg::InitialDPSMSArg(DeliveryMode_e idpMode)
{
    compLogger = smsc::logger::Logger::getInstance("smsc.inman.comp.InitialDPSMSArg");
    comp = new PrivateInitialDPSMSArg(idpMode);
}
InitialDPSMSArg::~InitialDPSMSArg() { delete(comp); }


/* this method doesn't required */
//void InitialDPSMSArg::decode(const vector<unsigned char>& buf)
//{ throw DecodeError("Not implemented"); }

void InitialDPSMSArg::encode(vector<unsigned char>& buf)
{
    asn_enc_rval_t er;

    //for debug only:
    asn_fprint(stdout, &asn_DEF_InitialDPSMSArg, &comp->idp);

    er = der_encode(&asn_DEF_InitialDPSMSArg, &comp->idp, print2vec, &buf);

    INMAN_LOG_ENC(er, asn_DEF_InitialDPSMSArg);
}

}//namespace comp
}//namespace inman
}//namespace smsc

