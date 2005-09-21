#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_RESULTS__
#define __SMSC_INMAN_INAP_RESULTS__

#include <map>

#include "ss7cp.h"
#include "inman/common/types.hpp"
#include "entity.hpp"

namespace smsc  {
namespace inman {
namespace inap  {

class InvokeResultLast : public TcapEntity
{
    public:
    	virtual void send(Dialog* dialog);
};

class InvokeResultNotLast : public TcapEntity
{
    public:
    	virtual void send(Dialog* dialog);
};

class InvokeResultError : public TcapEntity
{
    public:
    	virtual void send(Dialog* dialog);
};

}
}
}

#endif
