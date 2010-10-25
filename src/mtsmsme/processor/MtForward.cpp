static char const ident[] = "$Id$";
#include "MtForward.hpp"
#include <stdio.h>
#include <sys/types.h>
#include "MT-ForwardSM-Arg.h"
#include <vector>
namespace smsc{namespace mtsmsme{namespace processor{

static asn_TYPE_descriptor_t *def = &asn_DEF_MT_ForwardSM_Arg;
using std::string;
using std::vector;

void* MtForward::potroha(){ return structure;}

MtForward::MtForward(Logger* _logger){structure = 0; logger = _logger; }
MtForward::~MtForward()
{
  if(structure)
  {
    def->free_struct(def, structure, 0);
  }
}

void MtForward::decode(void *buf, int buflen)
{
  if (structure)
    def->free_struct(def, structure, 0);
  structure = 0;
  asn_codec_ctx_t s_codec_ctx;
  asn_codec_ctx_t *opt_codec_ctx = 0;
  opt_codec_ctx = &s_codec_ctx;
  asn_dec_rval_t rval;

  rval = ber_decode(0/*opt_codec_ctx*/, &asn_DEF_MT_ForwardSM_Arg,
      (void **) &structure, buf, buflen);
  if (rval.code != RC_OK)
    smsc_log_error(logger,
        "MtForward::decode consumes %d/%d and returns code %d",
        rval.consumed,buflen,rval.code
    );
  if(structure) smsc_log_debug(logger,"mt-forward message: %s",this->toString().c_str());
}
bool MtForward::isMMS()
{
  bool res = false;
  if(structure) res = (((MT_ForwardSM_Arg_t*)structure)->moreMessagesToSend != 0);
  return res;
}
uint8_t MtForward::get_TP_MTI()
{
  MT_ForwardSM_Arg_t& msg = *structure;
  return msg.sm_RP_UI.buf[0];
}
/* Dump the data into the specified stdio stream */
extern "C" static int toStream(const void *buffer, size_t size, void *app_key) {
  std::vector<unsigned char> *stream = (std::vector<unsigned char> *)app_key;
  unsigned char *buf = (unsigned char *)buffer;
  stream->insert(stream->end(),buf, buf + size);
  return 0;
}

string MtForward::toString() {
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


}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
