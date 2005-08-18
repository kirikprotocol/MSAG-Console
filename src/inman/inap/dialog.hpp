#ident "$Id$"
// Диалог (транзакция) IN

#ifndef __SMSC_INMAN_INAP_DIALOG__
#define __SMSC_INMAN_INAP_DIALOG__

#include <list>
#include <vector>

#include "ss7cp.h"
#include "inman/comp/comps.hpp"
#include "inman/comp/comfactory.hpp"

#include "protocol.hpp"

namespace smsc {
namespace inman {
namespace inap {

class Session;
using smsc::inman::comp::Component;
using smsc::inman::comp::ComponentFactory;
using std::vector;

class TcapOperation
{
  friend class TcapDialog;

  TcapOperation(TcapDialog*, UCHAR_T tag, UCHAR_T opcode);

public:
  virtual ~TcapOperation();

  virtual UCHAR_T getTag() const 
  { 
  		return tag; 
  }

  virtual UCHAR_T getOpcode() const
  {
  		return opcode;
  }

  virtual Component* getParam() const
  {
		return param;
  }

  virtual void setParam(Component* p)
  {
		param = p;
  }

  virtual void invoke();

protected:
  TcapDialog* 	dialog;
  UCHAR_T		opcode;
  UCHAR_T  		tag;
  Component*	param;
};

class TcapDialog
{
    friend class Session;

  public:

    TcapDialog(Session* session, USHORT_T id);
    virtual ~TcapDialog();

    void setProtocolFactory( ProtocolFactory* pf );

    ComponentFactory* getComponentFactory()  { return &compFactory; }
    ProtocolFactory*  getProtocolFactory()  { return protFactory;   }

    virtual TcapOperation* createOperation(UCHAR_T opcode);
    virtual TcapOperation* createOperation(UCHAR_T tag, UCHAR_T opcode, const UCHAR_T* param, USHORT_T paramBuf );

    // Transaction layer
    virtual USHORT_T beginDialog();//called by client of this dialog instance
    virtual USHORT_T continueDialog();
    virtual USHORT_T endDialog(USHORT_T termination);

    // Interaction level
    virtual USHORT_T invoke(TcapOperation* op);//called by client of this dialog instance
    virtual USHORT_T resultLast(TcapOperation* result);
    virtual USHORT_T resultNotLast(TcapOperation* result);
    virtual USHORT_T userError(TcapOperation* result);
    virtual USHORT_T timerReset();

    // Transaction level callbacks
	virtual USHORT_T handleBeginDialog();
	virtual USHORT_T handleContinueDialog();
	virtual USHORT_T handleEndDialog();

    // Interaction level callbacks
    virtual USHORT_T handleInvoke(TcapOperation* op);
    virtual USHORT_T handleResultLast(TcapOperation* op);
    virtual USHORT_T handleResultNotLast(TcapOperation* op);
    virtual USHORT_T handleUserError(TcapOperation* op);

   protected:
	ComponentFactory  compFactory;
    ProtocolFactory*  protFactory;
    Protocol*		  protocol;
    Session*          session;
    USHORT_T          opid;
    USHORT_T          did;
    UCHAR_T           qSrvc;
    UCHAR_T           priority;
    UCHAR_T           acShort;
};

}
}
}

#endif
