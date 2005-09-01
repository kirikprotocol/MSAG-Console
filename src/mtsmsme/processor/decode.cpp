#include "decode.hpp"
#include <stdio.h>
#include <sys/types.h>
#include <string>
#include <vector>
#include "logger/Logger.h"

extern "C" {
#include <constr_TYPE.h>
#include <MessageType.h>
}

/*
extern "C"
void ASN_DEBUG_f(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  smsc_log_debug(logger,fmt,ap);
  va_end(ap);
}
*/

extern asn_TYPE_descriptor_t asn_DEF_MessageType;
extern asn_TYPE_descriptor_t asn_DEF_MT_forward;


namespace smsc{namespace mtsmsme{namespace processor{namespace decode{

using std::vector;

static asn_TYPE_descriptor_t *def = &asn_DEF_MessageType;
static asn_TYPE_descriptor_t *mtdef = &asn_DEF_MT_forward;

using smsc::logger::Logger;
static Logger *logger = 0;

Decoder::Decoder()
{
  logger = smsc::logger::Logger::getInstance("mt.sme.asn");
}
Decoder::~Decoder() {}

void Decoder::decodemt(void *buf, int buflen,MtForward& msg)
{
  void *structure = 0;
  asn_codec_ctx_t s_codec_ctx;
  asn_codec_ctx_t *opt_codec_ctx = 0;
  opt_codec_ctx = &s_codec_ctx;
  asn_dec_rval_t rval;

  rval = ber_decode(0/*opt_codec_ctx*/, mtdef,(void **)&structure, buf, buflen);

  smsc_log_debug(logger,
                 "Decoder::decodemt consumes %d/%ld and returns code %d",
                 rval.consumed,
                 buflen,
                 rval.code
                );

  if(structure) {
    msg.setStructure(structure);
    smsc_log_debug(logger,"mt-forward message: %s",msg.toString().c_str());
  }
}
void Decoder::decode(void *buf, int buflen,Message& msg)
{
  void *structure = 0;
  asn_codec_ctx_t s_codec_ctx;
  asn_codec_ctx_t *opt_codec_ctx = 0;
  opt_codec_ctx = &s_codec_ctx;
  asn_dec_rval_t rval;

  rval = ber_decode(0/*opt_codec_ctx*/, def,(void **)&structure, buf, buflen);


  if(structure) {
    msg.setStructure(structure);
  }
  if (rval.code)
  {
    smsc_log_debug(logger,
                   "Decoder::decode fails: retcode=%d consumes %d/%ld",
                 rval.code,
                 rval.consumed,
                 buflen
                );
  }
  else
  {
    smsc_log_debug(logger,"Decoder::decode\n%s",msg.toString().c_str());
  }
}

}//namespace decode
}//namespace processor
}//namespace mtsmsme
}//namespace smsc
