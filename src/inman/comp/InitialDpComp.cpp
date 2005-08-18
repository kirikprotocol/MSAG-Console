static char const ident[] = "$Id$";
#include "comps.hpp"
#include <vector>
#include <InitialDPSMSArg.h>

namespace smsc {
namespace inman {
namespace comp{
using std::vector;

extern "C" static int print2vec(const void *buffer, size_t size, void *app_key);

class InternalInitialDPSMSArg
{
  public:
    InternalInitialDPSMSArg();

    InitialDPSMSArg_t idp;
    LocationInformation_t li;
};

#define ZERO_asn_struct_ctx_t   { 0, 0, 0, NULL, 0}
#define OCTET_STRING_OBJ(buf)   { buf, sizeof(buf), ZERO_asn_struct_ctx_t }

static uint8_t _dsn1_buf[] = { 0x91, 0x13, 0x11, 0x33 };
static OCTET_STRING_t _dSN1 = OCTET_STRING_OBJ(_dsn1_buf);
void InitialDPSMSArg::setDestinationSubscriberNumber()
{
  internal->idp.destinationSubscriberNumber = &_dSN1;
}

static uint8_t _cpn1_buf[] = { 0x91, 0x79, 0x13, 0x91, 0x63, 0x39, 0x3f };
static OCTET_STRING_t _cPN1 = OCTET_STRING_OBJ(_cpn1_buf);
void InitialDPSMSArg::setCallingPartyNumber()
{
  internal->idp.callingPartyNumber = &_cPN1;
}

static EventTypeSMS_t   _etSMS1 = EventTypeSMS_sms_CollectedInfo;
void InitialDPSMSArg::setMode()
{
  internal->idp.eventTypeSMS = &_etSMS1;
}

static uint8_t _imsi1_buf[] = { 0x25, 0x00, 0x13, 0x90, 0x13, 0x88, 0x78, 0x0f };
static OCTET_STRING_t _iMSI1 = OCTET_STRING_OBJ(_imsi1_buf);
void InitialDPSMSArg::setIMSI()
{
  internal->idp.iMSI = &_iMSI1;
}

static uint8_t _vlrn1_buf[] = { 0x91, 0x79, 0x13, 0x98, 0x60, 0x00, 0x1f };
static OCTET_STRING_t _vLRN1 = OCTET_STRING_OBJ(_vlrn1_buf);
static uint8_t _locn1_buf[] = { 0x81, 0x01, 0x79, 0x13, 0x98, 0x60, 0x00, 0x1f };
static OCTET_STRING_t _locN1 = OCTET_STRING_OBJ(_locn1_buf);
static uint8_t _cgidorsaifl1_buf[] = { 0x25, 0x0f, 0x10, 0x97, 0xFE, 0x13, 0x89 };
static OCTET_STRING_t _cGidOrSAIFL1 = OCTET_STRING_OBJ(_cgidorsaifl1_buf);
static CellGlobalIdOrServiceAreaIdOrLAI _cGidOrSAIorLAI1 = {CellGlobalIdOrServiceAreaIdOrLAI_PR_cellGlobalIdOrServiceAreaIdFixedLength
,OCTET_STRING_OBJ(_cgidorsaifl1_buf),ZERO_asn_struct_ctx_t};
void InitialDPSMSArg::setlocationInformationMSC()
{
  internal->li.vlr_number = &_vLRN1;
  internal->li.locationNumber = &_locN1;
  _cGidOrSAIorLAI1.present =
        CellGlobalIdOrServiceAreaIdOrLAI_PR_cellGlobalIdOrServiceAreaIdFixedLength;
  internal->li.cellGlobalIdOrServiceAreaIdOrLAI = &_cGidOrSAIorLAI1;
  internal->idp.locationInformationMSC = &internal->li;
}

static uint8_t _sMSCAdr1_buf[] = { 0x91, 0x79, 0x02, 0x98, 0x69, 0x99, 0x0f };
static OCTET_STRING_t _sMSCAdr1 = OCTET_STRING_OBJ(_sMSCAdr1_buf);
void InitialDPSMSArg::setSMSCAddress()
{
  internal->idp.sMSCAddress = &_sMSCAdr1;
}

static uint8_t _tImeZone1_buf[] = { 0x20, 0x05, 0x04, 0x22, 0x15, 0x03, 0x26, 0x42 };
static OCTET_STRING_t _tImeZone1 = OCTET_STRING_OBJ(_tImeZone1_buf);
void InitialDPSMSArg::setTimeAndTimezone()
{
  internal->idp.timeAndTimezone = &_tImeZone1;
}

static uint8_t _tPSMSI1_buf[] = { 0x11 };
static OCTET_STRING_t _tPSMSI1 = OCTET_STRING_OBJ(_tPSMSI1_buf);
void InitialDPSMSArg::setTPShortMessageSpecificInfo()
{
  internal->idp.tPShortMessageSpecificInfo = &_tPSMSI1;
}

static uint8_t _tPPid1_buf[] = { 0x00 };
static OCTET_STRING_t _tPPid1 = OCTET_STRING_OBJ(_tPPid1_buf);
void InitialDPSMSArg::setTPProtocolIdentifier()
{
  internal->idp.tPProtocolIdentifier = &_tPPid1;
}

static uint8_t _tPDCSch1_buf[] = { 0x08 };
static OCTET_STRING_t _tPDCSch1 = OCTET_STRING_OBJ(_tPDCSch1_buf);
void InitialDPSMSArg::setTPDataCodingScheme()
{
  internal->idp.tPDataCodingScheme = &_tPDCSch1;
}

static uint8_t _tPVP1_buf[] = { 0xda };
static OCTET_STRING_t _tPVP1 = OCTET_STRING_OBJ(_tPVP1_buf);
void InitialDPSMSArg::setTPValidityPeriod()
{
  internal->idp.tPValidityPeriod = &_tPVP1;
}


InternalInitialDPSMSArg::InternalInitialDPSMSArg()
{
  idp.serviceKey = 11;
  idp.destinationSubscriberNumber = NULL; //Address
  idp.callingPartyNumber = NULL; //Address
  idp.eventTypeSMS = NULL; //enum
  idp.iMSI = NULL; //Address
  idp.locationInformationMSC = NULL;
  idp.locationInformationGPRS = NULL;
  idp.sMSCAddress = NULL; //Address
  idp.timeAndTimezone = NULL; //time_t
  idp.tPShortMessageSpecificInfo = NULL;//int
  idp.tPProtocolIdentifier = NULL; //int
  idp.tPDataCodingScheme = NULL;// int
  idp.tPValidityPeriod = NULL; //time_t
  idp.extensions =NULL;
  idp.smsReferenceNumber = NULL;
  idp.mscAddress = NULL;
  idp.sgsn_Number  = NULL;
  idp.ms_Classmark2 = NULL;
  idp.gPRSMSClass = NULL;
  idp.iMEI = NULL;
  idp.calledPartyNumber = NULL;

  li.ageOfLocationInformation = NULL;
  li.geographicalInformation = NULL;
  li.vlr_number = NULL;
  li.locationNumber = NULL;
  li.cellGlobalIdOrServiceAreaIdOrLAI = NULL;
  li.extensionContainer = NULL;
  li.selectedLSA_Id = NULL;
  li.msc_Number = NULL;
  li.geodeticInformation = NULL;
  li.currentLocationRetrieved = NULL;
  li.sai_Present = NULL;
}

InitialDPSMSArg::InitialDPSMSArg()
{
//allocate internal
  internal = new InternalInitialDPSMSArg();
  this->setDestinationSubscriberNumber();
  this->setCallingPartyNumber();
  this->setMode();
  this->setIMSI();
  //
  this->setSMSCAddress();
  this->setTimeAndTimezone();
  this->setTPShortMessageSpecificInfo();
  this->setTPProtocolIdentifier();
  this->setTPDataCodingScheme();
  this->setTPValidityPeriod();
}
InitialDPSMSArg::~InitialDPSMSArg()
{
//deallocate internal
  delete(internal);
}
int InitialDPSMSArg::decode(const vector<unsigned char>& buf)
{

  return -1; //not implemented yet
}
int InitialDPSMSArg::encode(vector<unsigned char>& buf)
{
  asn_enc_rval_t er;
  er = der_encode(&asn_DEF_InitialDPSMSArg, &internal->idp,print2vec, &buf);

/*  if(er.encoded == -1) {
    smsc_log_error(MtSmsProcessorLogger,
                   "Cannot encode %s",
                   er.failed_type->name);
  }*/
  return (er.encoded == -1);
}

static int print2vec(const void *buffer, size_t size, void *app_key) {
  std::vector<unsigned char> *stream = (std::vector<unsigned char> *)app_key;
  unsigned char *buf = (unsigned char *)buffer;

  stream->insert(stream->end(),buf, buf + size);

  return 0;
}

}//namespace comps
}//namespace inman
}//namespace smsc
