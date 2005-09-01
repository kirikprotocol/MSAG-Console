#include "MtForward.hpp"
#include <stdio.h>
#include <sys/types.h>

extern "C" {
#include <constr_TYPE.h>
#include <MT-forward.h>
}
extern asn_TYPE_descriptor_t asn_DEF_MT_forward;

#include "logger/Logger.h"
using smsc::logger::Logger;
namespace smsc{namespace mtsmsme{namespace processor{
extern Logger* MtSmsProcessorLogger;
}}}

#include <vector>
namespace smsc{namespace mtsmsme{namespace processor{

static asn_TYPE_descriptor_t *def = &asn_DEF_MT_forward;
using std::string;
using std::vector;

void* MtForward::potroha(){ return structure;}

MtForward::MtForward(void *_structure){structure = _structure;}
MtForward::MtForward(){structure = 0;}
MtForward::~MtForward()
{
  if(structure)
  {
    def->free_struct(def, structure, 0);
  }
}
void MtForward::setStructure(void *_structure)
{
  if(structure)
  {
    def->free_struct(def, structure, 0);
  }
  structure = _structure;
}

bool MtForward::isMMS()
{
  bool res = false;
  if(structure) res = (((MT_forward_t*)structure)->moreMessagesToSend != 0);
  return res;
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
