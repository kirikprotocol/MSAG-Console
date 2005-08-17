#ident "$Id$"
// Диалог (транзакция) IN

#ifndef __SMSC_INMAN_INAP_DIALOG__
#define __SMSC_INMAN_INAP_DIALOG__

#include <list>
#include <vector>

#include "ss7cp.h"
#include "inman/comp/comps.hpp"

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

  TcapOperation(TcapDialog*);

public:
  virtual ~TcapOperation();

  virtual void decode(USHORT_T opTag, USHORT_T opLen, UCHAR_T* op, USHORT_T parLen, UCHAR_T* par);
  virtual void encode(USHORT_T& opTag, vector<UCHAR_T>& op, vector<UCHAR_T>& par);

  virtual UCHAR_T getTag() const 
  { 
  	return tag; 
  }

  virtual Component* getOperation() const
  {
  	return operation;
  }

  virtual void setOperation(Component* op)
  {
		operation = op;
  }

  virtual Component* getParameters() const
  {
  	return parameters;
  }

  virtual void invoke();

protected:
  TcapDialog* 		   dialog;
  UCHAR_T  			   tag;
  Component*		   operation;
  Component*		   parameters;
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

    virtual TcapOperation* createOperation();

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
