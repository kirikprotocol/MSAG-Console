#ifndef __SCAG_TRANSPORT_SMPP_WRITER_H__
#define __SCAG_TRANSPORT_SMPP_WRITER_H__

#include "SmppIOBase.h"

namespace scag{
namespace transport{
namespace smpp{

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
