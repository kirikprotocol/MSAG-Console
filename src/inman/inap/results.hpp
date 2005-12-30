#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_RESULTS__
#define __SMSC_INMAN_INAP_RESULTS__

#include "entity.hpp"

namespace smsc  {
namespace inman {
namespace inap  {

class Dialog;

class InvokeResultLast : public TcapEntity
{
public:
    InvokeResultLast(Dialog* dlg, UCHAR_T tId, UCHAR_T tTag = 0, UCHAR_T tOpCode = 0)
        : TcapEntity(tId, tTag, tOpCode), _dlg(dlg) {}

    void send() throw(CustomException);

protected:
    Dialog *         _dlg;      //parent Dialog
};

class InvokeResultNotLast : public TcapEntity
{
public:
    InvokeResultNotLast(Dialog* dlg, UCHAR_T tId, UCHAR_T tTag = 0, UCHAR_T tOpCode = 0)
        : TcapEntity(tId, tTag, tOpCode), _dlg(dlg) {}

    void send() throw(CustomException);

protected:
    Dialog *         _dlg;      //parent Dialog
};

class InvokeResultError : public TcapEntity
{
public:
    InvokeResultError(Dialog* dlg, UCHAR_T tId, UCHAR_T tTag = 0, UCHAR_T tOpCode = 0)
        : TcapEntity(tId, tTag, tOpCode), _dlg(dlg) {}

    void send() throw(CustomException);

protected:
    Dialog *         _dlg;      //parent Dialog
};

} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_RESULTS__ */

