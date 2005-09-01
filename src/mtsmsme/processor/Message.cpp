#include "Message.hpp"
#include <stdio.h>
#include <sys/types.h>

extern "C" {
#include <constr_TYPE.h>
#include <MessageType.h>
}
extern asn_TYPE_descriptor_t asn_DEF_MessageType;

#include "logger/Logger.h"
using smsc::logger::Logger;
namespace smsc{namespace mtsmsme{namespace processor{
extern Logger* MtSmsProcessorLogger;
}}}

namespace smsc{namespace mtsmsme{namespace processor{

static asn_TYPE_descriptor_t *def = &asn_DEF_MessageType;
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
    MessageType_t* pmsg = (MessageType_t*)structure;
    if(pmsg->present == MessageType_PR_begin)
    {
      OCTET_STRING_t *tid = &(pmsg->choice.begin.otid);
      otid.size = tid->size;
      memcpy(otid.buf,tid->buf,tid->size);
      //otid.insert(otid.end(),tid->buf, tid->buf + tid->size);
    }
    if(pmsg->present == MessageType_PR_contiinue)
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
    MessageType_t* pmsg = (MessageType_t*)structure;
    if(pmsg->present == MessageType_PR_contiinue)
    {
      OCTET_STRING_t *tid = &(pmsg->choice.contiinue.dtid);
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
  if(structure) res = (((MessageType_t*)structure)->present == MessageType_PR_begin);
  return res;
}
bool Message::isContinue()
{
  bool res = false;
  if(structure) res = (((MessageType_t*)structure)->present == MessageType_PR_contiinue);
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
    MessageType_t* pmsg = (MessageType_t*)structure;
    if(pmsg->present == MessageType_PR_begin)
    {
      void *comps = pmsg->choice.begin.components;
      res = (comps != 0);
    }
    if(pmsg->present == MessageType_PR_contiinue)
    {
      void *comps = pmsg->choice.contiinue.componenets;
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
    MessageType_t* pmsg = (MessageType_t*)structure;
    if(pmsg->present == MessageType_PR_begin)
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

  MessageType_t* pmsg = (MessageType_t*)structure;
  if(pmsg->present == MessageType_PR_begin)
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
  MessageType_t* pmsg = (MessageType_t*)structure;
  ComponentPortion_t *comps = 0;
  if(pmsg->present == MessageType_PR_begin) comps = pmsg->choice.begin.components;
  if(pmsg->present == MessageType_PR_contiinue) comps = pmsg->choice.contiinue.componenets;
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
        return comp->choice.invoke.invokeID;
      }
    }
  }
  return 0;
}

vector<unsigned char> Message::getComponent()
{
  vector<unsigned char> buf;
  MessageType_t* pmsg = (MessageType_t*)structure;
  ComponentPortion_t *comps = 0;
  if(pmsg->present == MessageType_PR_begin) comps = pmsg->choice.begin.components;
  if(pmsg->present == MessageType_PR_contiinue) comps = pmsg->choice.contiinue.componenets;
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
}//namespace processor
}//namespace mtsmsme
}//namespace smsc