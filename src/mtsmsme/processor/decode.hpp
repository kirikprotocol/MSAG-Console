#ifndef __SMSC_MTSMSME_PROCESSOR_DECODE_HPP__
#define __SMSC_MTSMSME_PROCESSOR_DECODE_HPP__

#include "Message.hpp"
#include "MtForward.hpp"
namespace smsc{namespace mtsmsme{namespace processor{namespace decode{

class Decoder {
  public:
    void decode(void *buf, int buflen, Message& msg);
    void decodemt(void *buf, int buflen,MtForward& msg);
    Decoder();
    ~Decoder();
};

}/*namespace decode*/}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
#endif
