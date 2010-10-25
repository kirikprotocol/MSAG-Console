#ident "$Id$"
#ifndef __SMSC_MTSMSME_PROCESSOR_MTFORWARD_HPP_
#define __SMSC_MTSMSME_PROCESSOR_MTFORWARD_HPP_

#include "logger/Logger.h"
#include <string>
namespace smsc{namespace mtsmsme{namespace processor{

using std::string;
using smsc::logger::Logger;

class MtForward
{
  public:
    MtForward(Logger* logger);
    string toString();
    void* potroha();
    bool isMMS();
    uint8_t get_TP_MTI();
    void MtForward::decode(void *buf, int buflen);
    ~MtForward();
  private:
    void *structure;
    Logger* logger;
};

}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
#endif //__SMSC_MTSMSME_PROCESSOR_MTFORWARD_HPP_
