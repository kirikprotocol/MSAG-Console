/*
	$Id$
*/

#if !defined __Cpp_Header__smeman_smedispatch_h__
#define __Cpp_Header__smeman_smedispatch_h__

#include "smetypes.h"
#include "smeproxy.h"

namespace smsc {
namespace smeman {


// abstract
class SmeDispatch
{
public:
	virtual SmeProxy* selectSmeProxy() = 0;
};


}; // namespace smeman
}; // namespace smsc

#endif


