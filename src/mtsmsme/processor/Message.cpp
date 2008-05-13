static char const ident[] = "$Id$";
#include "mtsmsme/processor/Message.hpp"
#include <stdio.h>
#include <sys/types.h>

extern "C" {
#include <constr_TYPE.h>
#include <TCMessage.h>
}
extern asn_TYPE_descriptor_t asn_DEF_TCMessage;

#include "logger/Logger.h"
using smsc::logger::Logger;
namespace smsc{namespace mtsmsme{namespace processor{
extern Logger* MtSmsProcessorLogger;
}}}

namespace smsc{namespace mtsmsme{namespace processor{

static asn_TYPE_descriptor_t *def = &asn_DEF_TCMessage;
using std::vector;
using std::string;
/*
 * Constructor
 */
Message::Message(void *_structure){structure = _structure;}
Message::Message(){structure = 0;}
void Message::setStructure(void *_structure)
{
  if(structure)
  {
    def->free_struct(def, structure, 0);
  }
  structure = _structure;
}

/*
 * Destructor
 * free structure allocated by ber decoder
 */
Message::~Message()
{
  if(structure)
  {
    def->free_struct(def, structure, 0);
  }
}
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
  ac.init(null_buf,sizeof(null_buf)/sizeof(unsigned long));

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
              if (comp->choice.invoke.opcode.choice.local == 46)
              {
                unsigned long sm_mt_relay_v1_buf[] = {0,4,0,0,1,0,25,1};
                ac.init(sm_mt_relay_v1_buf,sizeof(sm_mt_relay_v1_buf)/sizeof(unsigned long));
                smsc_log_debug(MtSmsProcessorLogger,"derive sm_mt_relay_v1 application context");

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
        return comp->choice.invoke.invokeId;
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
  OBJECT_IDENTIFIER_set_arcs(&r.application_context_name,&ac.arcs[0],sizeof(unsigned long),ac.arcs.size());
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
EndMsg::EndMsg()
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
  if(_ac == sm_mt_relay_v1 || _ac == null_ac) return;
  ac = _ac;
  dp.direct_reference = &pduoid;
  dp.indirect_reference = 0;
  dp.data_value_descriptor = 0;
  dp.encoding.present = encoding_PR_single_ASN1_type;
  dp.encoding.choice.single_ASN1_type.present = DialoguePDU_PR_dialogueResponse;
  AARE_apdu_t& r = dp.encoding.choice.single_ASN1_type.choice.dialogueResponse;
  r.protocol_version = &tcapversion;
  OBJECT_IDENTIFIER_set_arcs(&r.application_context_name,&ac.arcs[0],sizeof(unsigned long),ac.arcs.size());
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
  arr[0]= &comp;
  comps.list.count = 1;
  comps.list.size = 1;
  comps.list.array = arr;
  end.choice.end.components = &comps;
}
}//namespace processor
}//namespace mtsmsme
}//namespace smsc