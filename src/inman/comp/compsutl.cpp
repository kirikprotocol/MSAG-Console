#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/asn1rt/asn_internal.h"
#include "inman/comp/compsutl.hpp"
#include "inman/common/adrutil.hpp"

namespace smsc {
namespace inman {
namespace comp {

extern "C" int print2vec(const void *buffer, size_t size, void *app_key)
{
    std::vector<unsigned char> *stream = (std::vector<unsigned char> *)app_key;
    unsigned char *buf = (unsigned char *)buffer;

    stream->insert(stream->end(),buf, buf + size);
    return 0;
}

std::string printType2String(asn_TYPE_descriptor_t * def, void * tStruct) {

  /* Invoke type-specific printer */
  std::vector<unsigned char> stream;

  if (def->print_struct(def, tStruct, 1, print2vec, &stream))
    return "asn1/c type print error";

  /* Terminate the output */
  print2vec("\n", 1, &stream);

  /* Create and return resulting string */
  std::string result((char*)&stream[0],(char*)&stream[0]+stream.size());
  return result;
}

/*
 * Returns number of signals in address, zero in case of error.
 */
unsigned OCTET_STRING_2_Address(OCTET_STRING_t * octs, TonNpiAddress & addr)
{
    addr.clear();
    if (octs && octs->size)
      smsc::cvtutil::unpackOCTS2MAPAddress(addr, octs->buf, octs->size);
    return addr.length;
}

}//namespace comp
}//namespace inman
}//namespace smsc

