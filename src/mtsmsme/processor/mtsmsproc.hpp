#ifndef __SMSC_MTSMS_MTSMSPROC_HPP__
#define __SMSC_MTSMS_MTSMSPROC_HPP__

#include <time.h>
#include <string>
#include "core/synchronization/Mutex.hpp"
#include "decode.hpp"
#include "encode.hpp"
#include "MTRequest.hpp"

namespace smsc{
namespace mtsmsme{
namespace processor{

using std::string;
using smsc::core::synchronization::Mutex;

struct MtSmsEvent{
  string from;
  string to;
  time_t time;
};

class MtSmsListener{
  public:
    virtual void missed(MtSmsEvent event) = 0;
};


class MtSmsProcessor{
  public:
    static MtSmsProcessor* instance();
    int  run();
    void stop();
    void addmtsmsmeListener(mtsmsmeListener* listener);
    void removemtsmsmeListener();
    void firemtsmsmeEvent(mtsmsmeEvent& event);

  private:
    mtsmsmeProcessor();
    mtsmsmeListener* listener;
    static mtsmsmeProcessor* volatile processor;
    static Mutex lock;
};

}//namespace pr
}//namespace mtsmsme
}//namespace smsc
#endif
