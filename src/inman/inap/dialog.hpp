#ident "$Id$"
// Диалог (транзакция) IN

#ifndef __SMSC_INMAN_INAP_DIALOG__
#define __SMSC_INMAN_INAP_DIALOG__

#include <list>
#include <vector>

#include "ss7cp.h"

namespace smsc {
namespace inman {
namespace inap {

class Operation;
class Session;

class TcapOperation
{
	UCHAR_T 			 tag;
	std::vector<UCHAR_T> operation;
	std::vector<UCHAR_T> params;
public:
	TcapOperation();
	TcapOperation(USHORT_T opTag, USHORT_T opLen, UCHAR_T* op, USHORT_T parLen, UCHAR_T* par );
	virtual ~TcapOperation();
	virtual const UCHAR_T getTag() const 						{ return tag; }
	virtual const std::vector<UCHAR_T>& getOperation() const 	{ return operation; }
	virtual const std::vector<UCHAR_T>& getParams()  const 		{ return params; }
};

class DialogListener
{
public:

	virtual void invoke(const TcapOperation&) 		  = 0;
	virtual void invokeSuccessed(const TcapOperation&) = 0;
	virtual void invokeFailed(const TcapOperation&)   = 0;
};

typedef std::list<DialogListener*> DialogListenerList;

class Dialog
{
  	friend class Session;

  public:
    Dialog(Session* session, USHORT_T id);
    virtual ~Dialog();

    virtual void addDialogListener(DialogListener* l);
    virtual void removeDialogListener(DialogListener* l);

    virtual USHORT_T beginDialog();
    virtual USHORT_T continueDialog();
    virtual USHORT_T endDialog(USHORT_T termination);

    virtual USHORT_T invoke(const TcapOperation& op);
	virtual USHORT_T invokeSuccessed(const TcapOperation& result);
	virtual USHORT_T invokeFailed(const TcapOperation& result);

	virtual const DialogListenerList& getListeners() const
	{
		return listeners;
	}

	void fireInvoke(const TcapOperation& op);
	void fireInvokeSuccessed(const TcapOperation& op);
	void fireInvokeFailed(const TcapOperation& op);
  protected:

  	DialogListenerList	 listeners;
    Session*     		 session;
    USHORT_T	  		 opid;
    USHORT_T      		 did;
    UCHAR_T       		 qSrvc;
    UCHAR_T       		 priority;
    UCHAR_T       		 acShort;
};

}
}
}

#endif
