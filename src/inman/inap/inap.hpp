#ident "$Id$"
// Диалог (транзакция) IN

#ifndef __SMSC_INMAN_INAP_INAP__
#define __SMSC_INMAN_INAP_INAP__

#include <map>

#include "ss7cp.h"
#include "dialog.hpp"
#include "inap_sm.h"

namespace smsc {
namespace inman {
namespace inap {

class INAP : public DialogListener
{

  public:
    INAP(Dialog* dialog);
    virtual ~INAP();

  public: // DialogListener impl
	virtual void invoke(const TcapOperation&);
	virtual void invokeSuccessed(const TcapOperation&);
	virtual void invokeFailed(const TcapOperation&);

  public: // Called from FSM
	virtual void sendInitialDP();
	virtual void sendEventReport();

  protected:
	Dialog* 	  dialog;
  	INAPContext * context;
};

}
}
}

#endif
