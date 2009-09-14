#ifndef __EYELINE_CLUSTERCONTROLLER_PROTOCOL_MAGICS_HPP__
#define __EYELINE_CLUSTERCONTROLLER_PROTOCOL_MAGICS_HPP__

namespace eyeline{
namespace clustercontroller{
namespace protocol{

enum ProtocolMagics{
  pmWebApp=0x57424150,//'WBAP'
  pmSmsc=0x534d5343,//'SMSC'
  pmLoadBalancer=0x4c444c42//'LDBL'
};

}
}
}

#endif
