#ident "$Id$"
// Диалог (транзакция) IN

#ifndef __SMSC_INMAN_INAP_DIALOG__
#define __SMSC_INMAN_INAP_DIALOG__

#include <map>

#include "ss7cp.h"

#include "dialog_sm.h"

namespace smsc {
namespace inman {
namespace inap {

class Operation;
class Session;

class Dialog
{
  friend class Session;

  public:
    Dialog(Session* session, USHORT_T id);
    virtual ~Dialog();

    USHORT_T invoke(const Operation* op);

	USHORT_T initialDPSMS();
	USHORT_T eventReportSMS();
	USHORT_T applicationEnd();

    USHORT_T start();

    USHORT_T beginReq();
    USHORT_T endReq();
    USHORT_T dataReq();

  protected:
    Session*     session;

    USHORT_T      did;
    UCHAR_T       qSrvc;
    UCHAR_T       priority;
    UCHAR_T       acShort;
  	DialogContext context;
};

}
}
}

#endif
