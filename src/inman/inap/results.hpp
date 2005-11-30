#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_RESULTS__
#define __SMSC_INMAN_INAP_RESULTS__

#include "entity.hpp"

namespace smsc  {
namespace inman {
namespace inap  {

class InvokeResultLast : public TcapEntity
{
public:
    InvokeResultLast()
        : TcapEntity(0, 0, 0) { }

    InvokeResultLast(UCHAR_T tId, UCHAR_T tTag, UCHAR_T tOpCode)
        : TcapEntity(tId, tTag, tOpCode) {}

    virtual void send(Dialog* dialog);
};

class InvokeResultNotLast : public TcapEntity
{
public:
    InvokeResultNotLast()
        : TcapEntity(0, 0, 0) { }

    InvokeResultNotLast(UCHAR_T tId, UCHAR_T tTag, UCHAR_T tOpCode)
        : TcapEntity(tId, tTag, tOpCode) {}

    virtual void send(Dialog* dialog);
};

class InvokeResultError : public TcapEntity
{
public:
    InvokeResultError()
        : TcapEntity(0, 0, 0) { }

    InvokeResultError(UCHAR_T tId, UCHAR_T tTag, UCHAR_T tOpCode)
        : TcapEntity(tId, tTag, tOpCode) {}

    virtual void send(Dialog* dialog);
};

} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_RESULTS__ */

