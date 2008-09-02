#ifndef __SCAG_TRANSPORT_SMPP_WRITER2_H__
#define __SCAG_TRANSPORT_SMPP_WRITER2_H__

#include "SmppIOBase2.h"

namespace scag2 {
namespace transport {
namespace smpp {

class SmppWriter:public SmppIOBase
{
public:
  const char* taskName(){return "SmppWriter";}
  int Execute();
protected:
};

}//smpp
}//transport
}//scag


#endif
