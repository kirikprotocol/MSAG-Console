
/*
	$Id$
*/

#if !defined __Cpp_Header__smeman_smereg_h__
#define __Cpp_Header__smeman_smereg_h__

#include "smetypes.h"
#include "smeproxy.h"

namespace smsc {
namespace smeman {

// abstract
class SmeRegistrar
{
public:
	virtual void registerSmeProxy(const SmeSystemId& systemId, SmeProxy* smeProxy) = 0;
};


}; // namespace smeman
}; // namespace smsc

#endif


