static char const ident[] = "$Id$";
#include "TCO.hpp"
#include "SRI4SMTSM.hpp"
#include "mtsmsme/processor/util.hpp"
#include "mtsmsme/comp/SendRoutingInfoForSM.hpp"
#include "mtsmsme/comp/ReportSmDeliveryStatus.hpp"
#include <string>
#include "sms/sms.h"
#include <asn_application.h>

namespace smsc{namespace mtsmsme{namespace processor{


using smsc::mtsmsme::processor::util::packSCCPAddress;
using smsc::mtsmsme::processor::util::dump;
using smsc::mtsmsme::comp::SendRoutingInfoForSMResp;
using smsc::mtsmsme::comp::EmptyComp;
using smsc::mtsmsme::processor::util::getAddressDescription;
using smsc::mtsmsme::comp::SendRoutingInfoForSMInd;
using smsc::mtsmsme::comp::SendRoutingInfoForSMRespV1;
using smsc::mtsmsme::comp::ReportSmDeliveryStatusInd;
using smsc::sms::Address;
using std::string;

SRI4SMTSM::SRI4SMTSM(TrId _ltrid,AC& ac,TCO* _tco):TSM(_ltrid,ac,_tco)
{
  logger = Logger::getInstance("mt.sme.sri4sm");
  smsc_log_debug(logger,"tsm otid=%s create SendRoutingInfoForSM or reportSMDeliveryStatus",ltrid.toString().c_str());
  temp_errcode = 0;
}
SRI4SMTSM::SRI4SMTSM(TrId _ltrid,AC& ac,TCO* _tco, const char* _imsi, const char* _msc,const char* _vlr, const char* _mgt):TSM(_ltrid,ac,_tco)
{
  logger = Logger::getInstance("mt.sme.sri4sm");
  smsc_log_debug(logger,"tsm otid=%s create SendRoutingInfoForSM",ltrid.toString().c_str());
  imsi = _imsi;
  msc = _msc;
  vlr = _vlr;
  mgt = _mgt;
  laddrlen = packSCCPAddress(laddr, 1 /* E.164 */, _vlr /* VLR E.164 */, 7 /* VLR SSN */);
  raddrlen = packSCCPAddress(raddr, 7 /* E.214 */, _mgt /* MS  E.214 */, 6 /* HLR SSN */);
}
SRI4SMTSM::~SRI4SMTSM()
{
  smsc_log_debug(logger,"tsm otid=%s delete SendRoutingInfoForSM or reportSMDeliveryStatus",ltrid.toString().c_str());
}
static int SRI4SMCOUNT = 0;
void SRI4SMTSM::BEGIN(Message& msg)
{
  laddrlen = packSCCPAddress(laddr, 1 /* E.164 */, tco->hlrnumber /* HLR E.164 */, 6 /* HLR SSN */);
  smsc_log_debug(logger,
                      "SRI4SMTSM's modified Cd(%s)",
                      getAddressDescription(laddrlen,laddr).c_str());
  int iid = 1;
  if( msg.isComponentPresent() && (msg.getOperationCode() == 45))
  {
    smsc_log_debug(logger,
                   "tsm otid=%s receive BEGIN with component, send END with SendRoutingInfoForSM",ltrid.toString().c_str());
    iid = msg.getInvokeId();
    //if (msg.getOperationCode() == 47)
    //  smsc_log_error(logger,"tsm otid=%s receive BEGIN with REPORTSM-DELIVERYSTATUS",ltrid.toString().c_str());
    std::vector<unsigned char> sri4smbuf;
    sri4smbuf = msg.getComponent();
    SendRoutingInfoForSMInd sri4sm(logger);
    sri4sm.decode(sri4smbuf);
    HLROAM* hlr = tco->getHLROAM();
    if ( hlr )
    {
      Address msisdn;
      Address _imsi;
      Address _msc;
      msisdn.setValue((uint8_t)strlen(sri4sm.getMSISDN()),sri4sm.getMSISDN());
      //if ( hlr->lookup(msisdn,_imsi) )
      if ( hlr->lookup(msisdn,_imsi,_msc) )
      {
        //if ( appcntx == shortMsgGatewayContext_v1)
        //{
        //  SendRoutingInfoForSMRespV1 resp(_imsi.value, tco->mscnumber);
        //  TResultLReq( iid /* invokeId */, 45 /* sendRoutingInfoForSM operation */, resp);
        //  smsc_log_debug(logger,
        //                 "tsm otid=%s receive BEGIN with component MAP V1, "
        //                 "send END with MWD=FALSE",
        //                 ltrid.toString().c_str());
        //}
        //else
        //{
        //  SendRoutingInfoForSMResp resp(_imsi.value, tco->mscnumber);
          SendRoutingInfoForSMResp resp(_imsi.value, _msc.value);
          TResultLReq( iid /* invokeId */, 45 /* sendRoutingInfoForSM operation */, resp);
        //}
      }
      else
      {
        EmptyComp err;
        TUErrorReq(iid /* invokeId */, 1 /* unknownSubscriber ERROR */, err);
      }
    }
  }
  if( msg.isComponentPresent() && (msg.getOperationCode() == 47))
  {
    smsc_log_debug(logger,
                   "tsm otid=%s receive BEGIN with reportSMDeliveryStatus component, send END",ltrid.toString().c_str());
    iid = msg.getInvokeId();
    std::vector<unsigned char> rsmdsbuf;
    rsmdsbuf = msg.getComponent();
    ReportSmDeliveryStatusInd rsmds(logger);
    rsmds.decode(rsmdsbuf);
    EmptyComp resp;
    TResultLReq( iid /* invokeId */, 47 /* reportSMDeliveryStatus operation */, resp);
  }
  TEndReq();
  tco->TSMStopped(ltrid);
}
void SRI4SMTSM::CONTINUE_received(uint8_t cdlen,
                              uint8_t *cd, /* called party address */
                              uint8_t cllen,
                              uint8_t *cl, /* calling party address */
                              Message& msg)
{
  //first response may contain address specified by remote side, copy
  raddrlen = cllen;
  memcpy(&raddr[0],cl,cllen);
  rtrid = msg.getOTID();

  if(msg.isComponentPresent())
  {
    ContMsg cont;
    int iid = msg.getInvokeId();
    cont.setOTID(ltrid);
    cont.setDTID(rtrid);
    cont.setComponent(0,iid);
    std::vector<unsigned char> rsp;
    tco->encoder.encode_resp(cont,rsp);
    tco->SCCPsend(raddrlen,&raddr[0],laddrlen,laddr,(uint16_t)rsp.size(),&rsp[0]);

    smsc_log_debug(logger,
                   "tsm.SRI4SMTSM otid=%s receive CONTINUE with component, INVOKE_RES sent",
                   ltrid.toString().c_str());
  }
}
void SRI4SMTSM::TInvokeReq(int8_t invokeId, uint8_t opcode, CompIF& arg)
{
  arg.encode(temp_arg);
  temp_opcode = opcode;
  temp_invokeId = invokeId;
}
void SRI4SMTSM::TResultLReq(uint8_t invokeId, uint8_t opcode, CompIF& arg)
{
  arg.encode(temp_arg);
  temp_opcode = opcode;
  temp_invokeId = invokeId;
}
void SRI4SMTSM::TUErrorReq(int invokeId, uint8_t errcode, CompIF& arg)
{
  arg.encode(temp_arg);
  temp_errcode = errcode;
  temp_invokeId = invokeId;
}
void SRI4SMTSM::TEndReq()
{
  EndMsg end(logger);
   end.setTrId(rtrid);
   end.setDialog(appcntx);
   //TODO remove ugly code of choice between result and error
   if (temp_errcode)
   {
     smsc_log_debug(logger,"tsm.sri4sm otid=%s setting unknown sibscriber ERROR",
                         ltrid.toString().c_str());
     end.setError(temp_invokeId,temp_errcode,temp_arg);
   }
   else
   {
     end.setReturnResultL(temp_invokeId, temp_opcode, temp_arg);
   }
   vector<unsigned char> data;
   end.encode(data);
   tco->SCCPsend(raddrlen,raddr,laddrlen,laddr,(uint16_t)data.size(),&data[0]);

   smsc_log_debug(logger,
                  "tsm.sri4sm otid=%s receive BEGIN, END sent "
                  "CALLED[%d]={%s} "
                  "CALLING[%d]={%s} "
                  "SRI4SM[%d]={%s}",
                  ltrid.toString().c_str(),
                  raddrlen,dump(raddrlen,raddr).c_str(),
                  laddrlen,dump(laddrlen,laddr).c_str(),
                  (uint16_t)data.size(),dump((uint16_t)data.size(),&data[0]).c_str());
}
void SRI4SMTSM::TBeginReq(uint8_t  cdlen, uint8_t* cd, uint8_t  cllen, uint8_t* cl)
{
  // set SCCP adresses

  raddrlen = cdlen;
  memcpy(raddr,cd,cdlen);
  laddrlen = cllen;
  memcpy(laddr,cl,cllen);

  TCMessage_t begin;
  EXT_t         dp;
  // set BEGIN

  begin.present = TCMessage_PR_begin;

  // use transaction Id from TSM object

  begin.choice.begin.otid.buf = ltrid.buf;
  begin.choice.begin.otid.size = ltrid.size;

  begin.choice.begin.dialoguePortion = 0;
  begin.choice.begin.components = 0;

  //initialize empty application context

  dp.encoding.choice.single_ASN1_type.choice.dialogueRequest.application_context_name.size=0;
  dp.encoding.choice.single_ASN1_type.choice.dialogueRequest.application_context_name.buf=0;

  // initialize application context
  dp.direct_reference = &pduoid;
  dp.indirect_reference = 0;
  dp.data_value_descriptor = 0;
  dp.encoding.present = encoding_PR_single_ASN1_type;
  dp.encoding.choice.single_ASN1_type.present = DialoguePDU_PR_dialogueRequest;
  AARQ_apdu_t& r = dp.encoding.choice.single_ASN1_type.choice.dialogueRequest;
  r.protocol_version = &tcapversion;

  // use applicatopn context from TSM
  OBJECT_IDENTIFIER_set_arcs(&r.application_context_name,&appcntx.arcs[0],(unsigned int)sizeof(unsigned long),(unsigned int)appcntx.arcs.size());
  r.aarq_user_information = 0;
  begin.choice.begin.dialoguePortion = ( struct EXT *)&dp;

  Component_t comp;
  Component_t *arr[1];
  ComponentPortion_t comps;
  ANY_t argument;

  // intialize invoke with invokeID and operation code
  comp.present = Component_PR_invoke;
  comp.choice.invoke.invokeId = temp_invokeId;
  comp.choice.invoke.linkedId = 0;
  comp.choice.invoke.opcode.present = Code_PR_local;
  comp.choice.invoke.opcode.choice.local = temp_opcode;
  argument.size = (int)temp_arg.size();
  argument.buf = &temp_arg[0];
  comp.choice.invoke.argument = &argument;

  arr[0]= &comp;
  comps.list.count = 1;
  comps.list.size = 1;
  comps.list.array = arr;
  begin.choice.begin.components = &comps;

  vector<unsigned char> data;
  asn_enc_rval_t er;
  er = der_encode(&asn_DEF_TCMessage, &begin, print2vec, &data);
  if(er.encoded == -1) {
    smsc_log_error(logger,
      "SRI4SMTSM::TBeginReq encode fail %s",er.failed_type->name);
  }

  smsc_log_debug(logger,
                 "CALLED[%d]={%s}",raddrlen,dump(raddrlen,raddr).c_str());
  smsc_log_debug(logger,
                 "CALLING[%d]={%s}",laddrlen,dump(laddrlen,laddr).c_str());
  smsc_log_debug(logger,
                 "SRI4SM[%d]={%s}",data.size(),dump((uint16_t)data.size(),&data[0]).c_str());

  tco->SCCPsend(raddrlen,raddr,laddrlen,laddr,(uint16_t)data.size(),&data[0]);
}
}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
