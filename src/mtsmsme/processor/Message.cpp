static char const ident[] = "$Id$";
#include "mtsmsme/processor/Message.hpp"
#include "mtsmsme/processor/util.hpp"
#include <stdio.h>
#include <sys/types.h>
#include "logger/Logger.h"

namespace smsc{namespace mtsmsme{namespace processor{

static asn_TYPE_descriptor_t *def = &asn_DEF_TCMessage;
using std::vector;
using std::string;
using smsc::logger::Logger;
/*
 * Constructor
 */
Message::Message(Logger* _logger){structure = 0; logger = _logger; }
void Message::decode(void *buf, int buflen)
{
  if (structure)
    def->free_struct(def, structure, 0);
  structure = 0;
  asn_codec_ctx_t s_codec_ctx;
  asn_codec_ctx_t *opt_codec_ctx = 0;
  opt_codec_ctx = &s_codec_ctx;
  asn_dec_rval_t rval;

  rval = ber_decode(0/*opt_codec_ctx*/, &asn_DEF_TCMessage,
      (void **) &structure, buf, buflen);
  if (rval.code != RC_OK)
    smsc_log_error(logger,
        "Message::decode consumes %d/%d and returns code %d", rval.consumed,
        buflen, rval.code);
  }
  /*
 * Destructor
 * free structure allocated by ber decoder
 */
Message::~Message() { if(structure) def->free_struct(def, structure, 0); }
/*
 * return transaction id
 */
TrId Message::getOTID()
{
  TrId otid;
  if(structure)
  {
    TCMessage_t* pmsg = (TCMessage_t*)structure;
    if(pmsg->present == TCMessage_PR_begin)
    {
      OCTET_STRING_t *tid = &(pmsg->choice.begin.otid);
      otid.size = tid->size;
      memcpy(otid.buf,tid->buf,tid->size);
      //otid.insert(otid.end(),tid->buf, tid->buf + tid->size);
    }
    if(pmsg->present == TCMessage_PR_contiinue)
    {
      OCTET_STRING_t *tid = &(pmsg->choice.contiinue.otid);
      otid.size = tid->size;
      memcpy(otid.buf,tid->buf,tid->size);
      //otid.insert(otid.end(),tid->buf, tid->buf + tid->size);
    }
  }
  return otid;
}
TrId Message::getDTID()
{
  TrId dtid;
  if(structure)
  {
    TCMessage_t* pmsg = (TCMessage_t*)structure;
    if(pmsg->present == TCMessage_PR_contiinue)
    {
      OCTET_STRING_t *tid = &(pmsg->choice.contiinue.dtid);
      dtid.size = tid->size;
      memcpy(dtid.buf,tid->buf,tid->size);
      //otid.insert(otid.end(),tid->buf, tid->buf + tid->size);
    }
    if(pmsg->present == TCMessage_PR_end)
    {
      OCTET_STRING_t *tid = &(pmsg->choice.end.dtid);
      dtid.size = tid->size;
      memcpy(dtid.buf,tid->buf,tid->size);
      //otid.insert(otid.end(),tid->buf, tid->buf + tid->size);
    }
  }
  return dtid;
}

bool Message::isBegin()
{
  bool res = false;
  if(structure) res = (((TCMessage_t*)structure)->present == TCMessage_PR_begin);
  return res;
}
bool Message::isContinue()
{
  bool res = false;
  if(structure) res = (((TCMessage_t*)structure)->present == TCMessage_PR_contiinue);
  return res;
}
bool Message::isEnd()
{
  bool res = false;
  if(structure) res = (((TCMessage_t*)structure)->present == TCMessage_PR_end);
  return res;
}
bool Message::isAbort()
{
  bool res = false;
  if(structure) res = (((TCMessage_t*)structure)->present == TCMessage_PR_abort);
  return res;

}

/*
 * check component presence
 */
bool Message::isComponentPresent()
{
  bool res = false;
  if(structure)
  {
    TCMessage_t* pmsg = (TCMessage_t*)structure;
    if(pmsg->present == TCMessage_PR_begin)
    {
      void *comps = pmsg->choice.begin.components;
      res = (comps != 0);
    }
    if(pmsg->present == TCMessage_PR_contiinue)
    {
      void *comps = pmsg->choice.contiinue.components;
      res = (comps != 0);
    }
    if(pmsg->present == TCMessage_PR_end)
    {
      void *comps = pmsg->choice.end.components;
      res = (comps != 0);
    }
  }
  return res;
}
bool Message::isDialoguePortionExist()
{
  bool res = false;
  if(structure)
  {
    TCMessage_t* pmsg = (TCMessage_t*)structure;
    if(pmsg->present == TCMessage_PR_begin)
    {
      void *comps = pmsg->choice.begin.dialoguePortion;
      res = (comps != 0);
    }
  }
  return res;
}

void Message::getAppContext(AC& ac)
{
  unsigned long null_buf[] = {0,0,0,0,0,0,0,0};
  ac.init(null_buf,(int)(sizeof(null_buf)/sizeof(unsigned long)));

  TCMessage_t* pmsg = (TCMessage_t*)structure;
  if(pmsg->present == TCMessage_PR_begin)
  {
    EXT_t *dp_ptr = (EXT_t*)pmsg->choice.begin.dialoguePortion;

    if (dp_ptr)
    {
      EXT_t& dp = *dp_ptr;
      if (dp.encoding.present == encoding_PR_single_ASN1_type)
      {
        DialoguePDU_t& dpdu = dp.encoding.choice.single_ASN1_type;
        if (dpdu.present == DialoguePDU_PR_dialogueRequest)
        {
          AARQ_apdu_t&  aare = dpdu.choice.dialogueRequest;

          {
            OBJECT_IDENTIFIER_t *oid = &aare.application_context_name;
            unsigned long fixed_arcs[10];
            unsigned long *arcs = fixed_arcs;
            int arc_type_size = sizeof(fixed_arcs[0]);
            int arc_slots = sizeof(fixed_arcs)/sizeof(fixed_arcs[0]);
            int count;
            int i;

            count = OBJECT_IDENTIFIER_get_arcs(oid, arcs,
                arc_type_size, arc_slots);
            // If necessary, reallocate arcs array and try again.
            if(count > arc_slots) {
              arc_slots = count;
              arcs = (unsigned long *)malloc(arc_type_size * arc_slots);
              if(!arcs) return;
              count = OBJECT_IDENTIFIER_get_arcs(oid, arcs,
                  arc_type_size, arc_slots);
            }
            ac.init(arcs,count);
            // Avoid memory leak.
            if(arcs != fixed_arcs) free(arcs);
          }
        }
      }
    }
    else
    {
      /* no dialoguePortion, check operation code */
      ComponentPortion_t *comps =  pmsg->choice.begin.components;
      if (comps)
      {
        /* obtain first component */
        typedef A_SEQUENCE_OF(void) T;
        T *list = (T*)&comps->list;//just a hack
        if (list->count == 1)
        {
          Component_t *comp = (Component_t *)(list->array[0]);
          if (comp->present == Component_PR_invoke)
          {
            if(comp->choice.invoke.opcode.present == Code_PR_local)
            {
              if (comp->choice.invoke.opcode.choice.local == 46)
              {
                unsigned long sm_mt_relay_v1_buf[] = {0,4,0,0,1,0,25,1};
                ac.init(sm_mt_relay_v1_buf,(int)(sizeof(sm_mt_relay_v1_buf)/sizeof(unsigned long)));
                smsc_log_debug(logger,"derive sm_mt_relay_v1 application context");
              }
              // SRI4SM operation
              if (comp->choice.invoke.opcode.choice.local == 45)
              {
                unsigned long shortMsgGatewayContext_v1_buf[] = {0,4,0,0,1,0,20,1};
                ac.init(shortMsgGatewayContext_v1_buf,(int)(sizeof(shortMsgGatewayContext_v1_buf)/sizeof(unsigned long)));
                smsc_log_debug(logger,"derive shortMsgGatewayContext_v1 application context");
              }
              if (comp->choice.invoke.opcode.choice.local == 4)
              {
                unsigned long roamingNumberEnquiryContext_v1_buf[] = {0,4,0,0,1,0,3,1};
                ac.init(roamingNumberEnquiryContext_v1_buf,(int)(sizeof(roamingNumberEnquiryContext_v1_buf)/sizeof(unsigned long)));
                smsc_log_debug(logger,"derive roamingNumberEnquiryContext_v1_buf application context");
              }
            }
          }
        }
      }
    }
  }
}

int Message::getInvokeId()
{
  TCMessage_t* pmsg = (TCMessage_t*)structure;
  ComponentPortion_t *comps = 0;
  if(pmsg->present == TCMessage_PR_begin) comps = pmsg->choice.begin.components;
  if(pmsg->present == TCMessage_PR_contiinue) comps = pmsg->choice.contiinue.components;
  if(pmsg->present == TCMessage_PR_end) comps = pmsg->choice.end.components;
  if (comps)
  {
    /* obtain first component */
    typedef A_SEQUENCE_OF(void) T;
    T *list = (T*)&comps->list;//just a hack
    if (list->count == 1)
    {
      Component_t *comp = (Component_t *)(list->array[0]);
      if (comp->present == Component_PR_invoke)
      {
        return (int)(comp->choice.invoke.invokeId);
      }
    }
  }
  return 0;
}
int Message::getOperationCode()
{
  TCMessage_t* pmsg = (TCMessage_t*)structure;
  ComponentPortion_t *comps = 0;
  if(pmsg->present == TCMessage_PR_begin) comps = pmsg->choice.begin.components;
  if(pmsg->present == TCMessage_PR_contiinue) comps = pmsg->choice.contiinue.components;
  if(pmsg->present == TCMessage_PR_end) comps = pmsg->choice.end.components;
  if(pmsg->present == TCMessage_PR_end) comps = pmsg->choice.end.components;
  if (comps)
  {
    /* obtain first component */
    typedef A_SEQUENCE_OF(void) T;
    T *list = (T*)&comps->list;//just a hack
    if (list->count == 1)
    {
      Component_t *comp = (Component_t *)(list->array[0]);
      if (comp->present == Component_PR_invoke)
      {
        return (int)(comp->choice.invoke.opcode.choice.local);
      }
    }
  }
  return 0;
}
vector<unsigned char> Message::getComponent()
{
  vector<unsigned char> buf;
  TCMessage_t* pmsg = (TCMessage_t*)structure;
  ComponentPortion_t *comps = 0;
  if(pmsg->present == TCMessage_PR_begin) comps = pmsg->choice.begin.components;
  if(pmsg->present == TCMessage_PR_contiinue) comps = pmsg->choice.contiinue.components;
  if
  if (comps)
  {
    /* obtain first component */
    typedef A_SEQUENCE_OF(void) T;
    T *list = (T*)&comps->list;//just a hack
    if (list->count == 1)
    {
      Component_t *comp = (Component_t *)(list->array[0]);
      if (comp->present == Component_PR_invoke)
      {
        ANY_t *pdu = comp->choice.invoke.argument;
        buf.insert(buf.end(),pdu->buf, pdu->buf + pdu->size);
      }
    }
  }
  return buf;
}
/* Dump the data into the specified stdio stream */
extern "C" static int toStream(const void *buffer, size_t size, void *app_key) {
  std::vector<unsigned char> *stream = (std::vector<unsigned char> *)app_key;
  unsigned char *buf = (unsigned char *)buffer;
  stream->insert(stream->end(),buf, buf + size);
  return 0;
}
string Message::toString() {
  /* Check structure presense */
  if(!structure) {
    return "null structure";
  }

  /* Invoke type-specific printer */
  std::vector<unsigned char> stream;
  if(def->print_struct(def, structure, 1, toStream, &stream))
    return "print error";

  /* Terminate the output */
  toStream("\n", 1, &stream);

  /* Create and return resulting string */
  string result((char*)&stream[0],(char*)&stream[0]+stream.size());
  return result;
}
ContMsg::ContMsg()
{
  cont.present = TCMessage_PR_contiinue;
  cont.choice.contiinue.dialoguePortion = 0;
  cont.choice.contiinue.components = 0;
  dp.encoding.choice.single_ASN1_type.choice.dialogueResponse.application_context_name.size=0;
  dp.encoding.choice.single_ASN1_type.choice.dialogueResponse.application_context_name.buf=0;
}
ContMsg::~ContMsg()
{
  if(dp.encoding.choice.single_ASN1_type.choice.dialogueResponse.application_context_name.buf)
    free(dp.encoding.choice.single_ASN1_type.choice.dialogueResponse.application_context_name.buf);
}
void ContMsg::setOTID(TrId _otid)
{
  memcpy(otid,_otid.buf,_otid.size);
  cont.choice.contiinue.otid.buf = otid;
  cont.choice.contiinue.otid.size = _otid.size;
}
void ContMsg::setDTID(TrId _dtid)
{
  memcpy(dtid,_dtid.buf,_dtid.size);
  cont.choice.contiinue.dtid.buf = dtid;
  cont.choice.contiinue.dtid.size = _dtid.size;
}
void ContMsg::setDialog(AC& _ac)
{
  if(_ac == sm_mt_relay_v1 || _ac == null_ac) return;
  ac = _ac;
  dp.direct_reference = &pduoid;
  dp.indirect_reference = 0;
  dp.data_value_descriptor = 0;
  dp.encoding.present = encoding_PR_single_ASN1_type;
  dp.encoding.choice.single_ASN1_type.present = DialoguePDU_PR_dialogueResponse;
  AARE_apdu_t& r = dp.encoding.choice.single_ASN1_type.choice.dialogueResponse;
  r.protocol_version = &tcapversion;
  OBJECT_IDENTIFIER_set_arcs(&r.application_context_name,&ac.arcs[0],
                             (unsigned int)sizeof(unsigned long),
                             (unsigned int)ac.arcs.size());
  r.result = Associate_result_accepted;
  r.result_source_diagnostic.present = Associate_source_diagnostic_PR_dialogue_service_user;
  r.result_source_diagnostic.choice.dialogue_service_user = dialogue_service_user_null;
  r.aare_user_information = 0;
  //cont.choice.contiinue.dialoguePortion = ( DialoguePortion *)&dp;
  cont.choice.contiinue.dialoguePortion = ( struct EXT *)&dp;
}
void ContMsg::setComponent(int result, int iid)
{
  if(result)
  {
    comp.present = Component_PR_returnError;
    comp.choice.returnError.invokeId = iid;
    comp.choice.returnError.errcode.present = Error_PR_local;
    comp.choice.returnError.errcode.choice.local = 31;
    comp.choice.returnError.parameter = 0;
  }
  else
  {
    comp.present = Component_PR_returnResultLast;
    comp.choice.returnResultLast.invokeId = iid;
    comp.choice.returnResultLast.result = 0;
  }
  arr[0]= &comp;
  comps.list.count = 1;
  comps.list.size = 1;
  comps.list.array = arr;
  cont.choice.contiinue.components = &comps;
}
EndMsg::EndMsg(): logger(0)
{
//  logger = 0;
  end.present = TCMessage_PR_end;
  end.choice.end.dialoguePortion = 0;
  end.choice.end.components = 0;
  dp.encoding.choice.single_ASN1_type.choice.dialogueResponse.application_context_name.size=0;
  dp.encoding.choice.single_ASN1_type.choice.dialogueResponse.application_context_name.buf=0;
}
EndMsg::EndMsg(Logger* _logger): logger(_logger)
{
  end.present = TCMessage_PR_end;
  end.choice.end.dialoguePortion = 0;
  end.choice.end.components = 0;
  dp.encoding.choice.single_ASN1_type.choice.dialogueResponse.application_context_name.size=0;
  dp.encoding.choice.single_ASN1_type.choice.dialogueResponse.application_context_name.buf=0;
}
EndMsg::~EndMsg()
{
  if(dp.encoding.choice.single_ASN1_type.choice.dialogueResponse.application_context_name.buf)
    free(dp.encoding.choice.single_ASN1_type.choice.dialogueResponse.application_context_name.buf);
}
void EndMsg::setTrId(TrId dtid)
{
  memcpy(trid,dtid.buf,dtid.size);
  end.choice.end.dtid.buf = trid;
  end.choice.end.dtid.size = dtid.size;
}
void EndMsg::setDialog(AC& _ac)
{
  if(_ac == sm_mt_relay_v1 ||
     _ac == null_ac ||
     _ac == roamingNumberEnquiryContext_v1 ||
     _ac == shortMsgGatewayContext_v1)
    return;
  ac = _ac;
  dp.direct_reference = &pduoid;
  dp.indirect_reference = 0;
  dp.data_value_descriptor = 0;
  dp.encoding.present = encoding_PR_single_ASN1_type;
  dp.encoding.choice.single_ASN1_type.present = DialoguePDU_PR_dialogueResponse;
  AARE_apdu_t& r = dp.encoding.choice.single_ASN1_type.choice.dialogueResponse;
  r.protocol_version = &tcapversion;
  OBJECT_IDENTIFIER_set_arcs(&r.application_context_name,&ac.arcs[0],
                             (unsigned int)(sizeof(unsigned long)),
                             (unsigned int)ac.arcs.size());
  r.result = Associate_result_accepted;
  r.result_source_diagnostic.present = Associate_source_diagnostic_PR_dialogue_service_user;
  r.result_source_diagnostic.choice.dialogue_service_user = dialogue_service_user_null;
  r.aare_user_information = 0;
  //end.choice.end.dialoguePortion = ( DialoguePortion *)&dp;
  end.choice.end.dialoguePortion = ( struct EXT *)&dp;
}
void EndMsg::setComponent(int result, int iid)
{
  if(result)
  {
    comp.present = Component_PR_returnError;
    comp.choice.returnError.invokeId = iid;
    comp.choice.returnError.errcode.present = Error_PR_local;
    comp.choice.returnError.errcode.choice.local = 31;
    comp.choice.returnError.parameter = 0;
  }
  else
  {
    comp.present = Component_PR_returnResultLast;
    comp.choice.returnResultLast.invokeId = iid;
    comp.choice.returnResultLast.result = 0;
  }
  fillComponentList();
  //arr[0]= &comp;
  //comps.list.count = 1;
  //comps.list.size = 1;
  //comps.list.array = arr;
  //end.choice.end.components = &comps;
}
void EndMsg::encode(vector<unsigned char>& buf)
{
  asn_enc_rval_t er = der_encode(def,&end,print2vec, &buf);
  if(er.encoded == -1 && logger)
  {
    smsc_log_error(logger,
                   "EndMsg::encode() fails, encode %d bytes, can't encode %s",
                   er.encoded, er.failed_type->name);
  }
}
void EndMsg::setReturnResultL(int iid, uint8_t opcode, vector<unsigned char>& argument)
{
  //initialize result component
  res.opcode.present = Code_PR_local;
  res.opcode.choice.local = opcode;
  res.result.size = (int)argument.size();
  res.result.buf = &argument[0];

  // intialize invoke with invokeID and result component
  comp.present = Component_PR_returnResultLast;
  comp.choice.returnResultLast.invokeId = iid;
  if ( argument.size() != 0 )
    comp.choice.returnResultLast.result = &res;
  else
    comp.choice.returnResultLast.result = 0;
  fillComponentList();
  //initialize component list
  //arr[0]= &comp;
  //comps.list.count = 1;
  //comps.list.size = 1;
  //comps.list.array = arr;
  //initialize components portion
  //end.choice.end.components = &comps;
}
void EndMsg::setError(int iid, uint8_t errcode, vector<unsigned char>& argument)
{
  comp.present = Component_PR_returnError;
  comp.choice.returnError.invokeId = iid;
  comp.choice.returnError.errcode.present = Error_PR_local;
  comp.choice.returnError.errcode.choice.local = errcode;
  comp.choice.returnError.parameter = 0;
  fillComponentList();
}
void EndMsg::fillComponentList()
{
  //initialize component list
  arr[0]= &comp;
  comps.list.count = 1;
  comps.list.size = 1;
  comps.list.array = arr;
  //initialize components portion
  end.choice.end.components = &comps;
}
BeginMsg::BeginMsg()
{
  begin.present = TCMessage_PR_begin;
  begin.choice.begin.dialoguePortion = 0;
  begin.choice.begin.components = 0;
  dp.encoding.choice.single_ASN1_type.choice.dialogueResponse.application_context_name.size=0;
  dp.encoding.choice.single_ASN1_type.choice.dialogueResponse.application_context_name.buf=0;
}
BeginMsg::~BeginMsg()
{
  if(dp.encoding.choice.single_ASN1_type.choice.dialogueResponse.application_context_name.buf)
    free(dp.encoding.choice.single_ASN1_type.choice.dialogueResponse.application_context_name.buf);
}
//
void BeginMsg::setOTID(TrId _otid)
{
  memcpy(otid,_otid.buf,_otid.size);
  begin.choice.begin.otid.buf = otid;
  begin.choice.begin.otid.size = _otid.size;
}
void BeginMsg::setDialog(AC& _ac)
{
  if(_ac == sm_mt_relay_v1 || _ac == null_ac) return;
  ac = _ac;
  dp.direct_reference = &pduoid;
  dp.indirect_reference = 0;
  dp.data_value_descriptor = 0;
  dp.encoding.present = encoding_PR_single_ASN1_type;
  dp.encoding.choice.single_ASN1_type.present = DialoguePDU_PR_dialogueRequest;
  AARQ_apdu_t& r = dp.encoding.choice.single_ASN1_type.choice.dialogueRequest;
  r.protocol_version = &tcapversion;
  OBJECT_IDENTIFIER_set_arcs(&r.application_context_name,&ac.arcs[0],
                             (unsigned int)sizeof(unsigned long),
                             (unsigned int)ac.arcs.size());
  r.aarq_user_information = 0;
  begin.choice.begin.dialoguePortion = ( struct EXT *)&dp;
}
void BeginMsg::setInvokeReq(int iid, uint8_t opcode, vector<unsigned char>& _argument)
{
  //initialize component argument
  argument.size = (int)_argument.size();
  argument.buf = &_argument[0];

  // intialize invoke with invokeID and operation code
  comp.present = Component_PR_invoke;
  comp.choice.invoke.invokeId = iid;
  comp.choice.invoke.linkedId = 0;
  comp.choice.invoke.opcode.present = Code_PR_local;
  comp.choice.invoke.opcode.choice.local = opcode;
  comp.choice.invoke.argument = &argument;

  arr[0]= &comp;
  comps.list.count = 1;
  comps.list.size = 1;
  comps.list.array = arr;
  begin.choice.begin.components = &comps;
}
void BeginMsg::encode(vector<unsigned char>& buf)
{
  asn_enc_rval_t er = der_encode(def,&begin,print2vec, &buf);
}
/* namespace processor */ } /* namespace mtsmsme */ } /* namespace smsc */}
