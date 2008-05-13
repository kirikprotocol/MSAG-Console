static char const ident[] = "$Id$";
#include <stdio.h>
#include <sys/types.h>
#include "mtsmsme/processor/util.hpp"
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

#include "encode.hpp"

namespace smsc{namespace mtsmsme{namespace processor{namespace encode{

using namespace smsc::mtsmsme::processor::util; //for extern "C" static int print2vec(const void *buffer, size_t size, void *app_key);
static void relMtResponse();
static void *prepMtResponse();
static void DialogResp(EXT_t *obj);
static std::vector<unsigned char> sample_encode_mt_resp();

static asn_TYPE_descriptor_t *def = &asn_DEF_TCMessage;

int Encoder::encodeACNotSupported(TrId dtid,AC& ac,vector<unsigned char>& buf)
{
  uint8_t trid[] = {0x00,0x00,0x00,0x00};
  TCMessage_t abort;
  struct Abort::reason reason;

  abort.present = TCMessage_PR_abort;
  memcpy(trid,dtid.buf,dtid.size);
  abort.choice.abort.dtid.buf = trid;
  abort.choice.abort.dtid.size = dtid.size;

  reason.present = reason_PR_u_abortCause;
  reason.choice.u_abortCause.direct_reference = &pduoid;
  reason.choice.u_abortCause.indirect_reference = 0;
  reason.choice.u_abortCause.data_value_descriptor = 0;
  reason.choice.u_abortCause.encoding.present = encoding_PR_single_ASN1_type;

  reason.choice.u_abortCause.encoding.choice.single_ASN1_type.present = DialoguePDU_PR_dialogueResponse;
  AARE_apdu_t& r = reason.choice.u_abortCause.encoding.choice.single_ASN1_type.choice.dialogueResponse;

  r.protocol_version = &tcapversion;
  r.application_context_name.size=0;
  r.application_context_name.buf=0;
  OBJECT_IDENTIFIER_set_arcs(&r.application_context_name,
                             &ac.arcs[0],
                             sizeof(unsigned long),
                             ac.arcs.size());

  r.result = Associate_result_reject_permanent;
  r.result_source_diagnostic.present = Associate_source_diagnostic_PR_dialogue_service_user;
  r.result_source_diagnostic.choice.dialogue_service_user = dialogue_service_user_application_context_name_not_supported;
  r.aare_user_information = 0;

  abort.choice.abort.reason = &reason;

  asn_enc_rval_t er;
  er = der_encode(def, &abort,print2vec, &buf);

  if(er.encoded == -1) {
    smsc_log_error(MtSmsProcessorLogger,
                   "Cannot encode %s",
                   er.failed_type->name);
  }
  if(r.application_context_name.buf)
    free(r.application_context_name.buf);

  return (er.encoded == -1);
}

int Encoder::encodeBadTrPortion(TrId dtid,vector<unsigned char>& buf)
{
  uint8_t trid[] = {0x00,0x00,0x00,0x00};
  TCMessage_t abort;
  struct Abort::reason reason;

  abort.present = TCMessage_PR_abort;
  memcpy(trid,dtid.buf,dtid.size);
  abort.choice.abort.dtid.buf = trid;
  abort.choice.abort.dtid.size = dtid.size;

  reason.present = reason_PR_p_abortCause;
  reason.choice.p_abortCause = P_AbortCause_badlyFormattedTransactionPortion;
  abort.choice.abort.reason = &reason;

  asn_enc_rval_t er;
  er = der_encode(def, &abort,print2vec, &buf);

  if(er.encoded == -1) {
    smsc_log_error(MtSmsProcessorLogger,
                   "Cannot encode %s",
                   er.failed_type->name);
  }
  return (er.encoded == -1);
}

int Encoder::encodeResourceLimitation(TrId dtid,vector<unsigned char>& buf)
{
  uint8_t trid[] = {0x00,0x00,0x00,0x00};
  TCMessage_t abort;
  struct Abort::reason reason;

  abort.present = TCMessage_PR_abort;
  memcpy(trid,dtid.buf,dtid.size);
  abort.choice.abort.dtid.buf = trid;
  abort.choice.abort.dtid.size = dtid.size;

  reason.present = reason_PR_p_abortCause;
  reason.choice.p_abortCause = P_AbortCause_resourceLimitation;
  abort.choice.abort.reason = &reason;

  asn_enc_rval_t er;
  er = der_encode(def, &abort,print2vec, &buf);

  if(er.encoded == -1) {
    smsc_log_error(MtSmsProcessorLogger,
                   "Cannot encode %s",
                   er.failed_type->name);
  }
  return (er.encoded == -1);
}
int Encoder::encode_mt_resp(EndMsg& msg,vector<unsigned char>& buf)
{
  asn_enc_rval_t er;
  er = der_encode(def, &msg.end,print2vec, &buf);

  if(er.encoded == -1) {
    smsc_log_error(MtSmsProcessorLogger,
                   "Cannot encode %s",
                   er.failed_type->name);
  }
  return (er.encoded == -1);
}
int Encoder::encode_resp(ContMsg& msg,vector<unsigned char>& buf)
{
  asn_enc_rval_t er;
  er = der_encode(def, &msg.cont,print2vec, &buf);

  if(er.encoded == -1) {
    smsc_log_error(MtSmsProcessorLogger,
                   "Cannot encode %s",
                   er.failed_type->name);
  }
  return (er.encoded == -1);
}
int Encoder::encode_resp(EndMsg& msg,vector<unsigned char>& buf)
{
  asn_enc_rval_t er;
  er = der_encode(def, &msg.end,print2vec, &buf);

  if(er.encoded == -1) {
    smsc_log_error(MtSmsProcessorLogger,
                   "Cannot encode %s",
                   er.failed_type->name);
  }
  return (er.encoded == -1);
}

int Encoder::encode_mt_resp(ContMsg& msg,vector<unsigned char>& buf)
{
  asn_enc_rval_t er;
  er = der_encode(def, &msg.cont,print2vec, &buf);

  if(er.encoded == -1) {
    smsc_log_error(MtSmsProcessorLogger,
                   "Cannot encode %s",
                   er.failed_type->name);
  }
  return (er.encoded == -1);
}

Encoder::Encoder()
{
}

Encoder::~Encoder()
{
}


//static int print2vec(const void *buffer, size_t size, void *app_key) {
//  std::vector<unsigned char> *stream = (std::vector<unsigned char> *)app_key;
//  unsigned char *buf = (unsigned char *)buffer;
//
//  stream->insert(stream->end(),buf, buf + size);
//
//  return 0;
//}

}//namespace encode
}//namespace processor
}//namespace mtsmsme
}//namespace smsc
