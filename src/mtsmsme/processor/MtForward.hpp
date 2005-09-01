#ifndef __SMSC_MTSMSME_PROCESSOR_MTFORWARD_HPP_
#define __SMSC_MTSMSME_PROCESSOR_MTFORWARD_HPP_

#include <string>

namespace smsc{namespace mtsmsme{namespace processor{

using std::string;

class MtForward
{
  public:
    MtForward(void *structure);
    MtForward();
    string toString();
    void* potroha();
    bool isMMS();
    void setStructure(void *structure);
    ~MtForward();
  private:
    void *structure;
};

}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
#endif //__SMSC_MTSMSME_PROCESSOR_MTFORWARD_HPP_
