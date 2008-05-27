static char const ident[] = "$Id$";
#include "TCO.hpp"
#include "SRI4SMTSM.hpp"
#include "util.hpp"
#include "mtsmsme/comp/UpdateLocation.hpp"
#include <string>
#include <asn_application.h>

namespace smsc{namespace mtsmsme{namespace processor{


using smsc::mtsmsme::processor::util::packSCCPAddress;
using smsc::mtsmsme::processor::util::dump;
using smsc::mtsmsme::comp::UpdateLocationMessage;
using std::string;

SRI4SMTSM::SRI4SMTSM(TrId _ltrid,AC& ac,TCO* _tco):TSM(_ltrid,ac,_tco)
{
  logger = Logger::getInstance("mt.sme.sri4sm");
  smsc_log_debug(logger,"tsm otid=%s create SendRoutingInfoForSM",ltrid.toString().c_str());
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
  smsc_log_debug(logger,"tsm otid=%s delete UpdateLocation",ltrid.toString().c_str());
}
void SRI4SMTSM::BEGIN_received(uint8_t _laddrlen, uint8_t *_laddr, uint8_t _raddrlen, uint8_t *_raddr, TrId _rtrid,Message& msg)
{
  smsc_log_debug(logger,"tsm otid=%s receive BEGIN with component, handling is NOT IMPLEMENTED YET",ltrid.toString().c_str());
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
    tco->SCCPsend(raddrlen,&raddr[0],laddrlen,laddr,rsp.size(),&rsp[0]);

    smsc_log_debug(logger,
                   "tsm.SRI4SMTSM otid=%s receive CONTINUE with component, INVOKE_RES sent",
                   ltrid.toString().c_str());
  }
}


void SRI4SMTSM::END_received(Message& msg)
{
  smsc_log_debug(logger,"tsm otid=%s receive END, close dialogue",ltrid.toString().c_str());
  if(listener) listener->complete(1);
  tco->TSMStopped(ltrid);
}
void SRI4SMTSM::TInvokeReq(uint8_t invokeId, uint8_t opcode, CompIF& arg)
{
  arg.encode(temp_arg);
  temp_opcode = opcode;
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
  OBJECT_IDENTIFIER_set_arcs(&r.application_context_name,&appcntx.arcs[0],sizeof(unsigned long),appcntx.arcs.size());
  r.aarq_user_information = 0;
  begin.choice.begin.dialoguePortion = ( struct EXT *)&dp;

  Component_t comp;
  Component_t *arr[1];
  ComponentPortion_t comps;
  ANY_t argument;

  // intialize invoke with invokeID and operation code
  comp.present = Component_PR_invoke;
  comp.choice.invoke.invokeId = 1;
  comp.choice.invoke.linkedId = 0;
  comp.choice.invoke.opcode.present = Code_PR_local;
  comp.choice.invoke.opcode.choice.local = temp_opcode;
  argument.size = temp_arg.size();
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

  smsc_log_error(logger,
                 "CALLED[%d]={%s}",raddrlen,dump(raddrlen,raddr).c_str());
  smsc_log_error(logger,
                 "CALLING[%d]={%s}",laddrlen,dump(laddrlen,laddr).c_str());
  smsc_log_error(logger,
                 "SRI4SM[%d]={%s}",data.size(),dump(data.size(),&data[0]).c_str());

  tco->SCCPsend(raddrlen,raddr,laddrlen,laddr,data.size(),&data[0]);
}
}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/