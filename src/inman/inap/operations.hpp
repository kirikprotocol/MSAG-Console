#ident "$Id$"
#ifndef __SMSC_INMAN_INAP_OPERATIONS__
#define __SMSC_INMAN_INAP_OPERATIONS__

#include "ss7cp.h"
#include "inman/common/types.hpp"
#include "inman/common/observable.hpp"
#include "inman/comp/comps.hpp"

using smsc::inman::common::ObservableT;
using smsc::inman::comp::Component;

namespace smsc {
namespace inman {
namespace inap {

class TcapDialog;
class TcapEntity
{

public:

  TcapEntity(TcapDialog* dlg, UCHAR_T tg, UCHAR_T op)
	: dialog( dlg )
	, tag( tg )
	, opcode( op )
	, param( NULL )
  {
  }
 
  virtual ~TcapEntity();

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


protected:
  TcapDialog* 	dialog;
  UCHAR_T		opcode;
  UCHAR_T  		tag;
  Component*	param;
};

class TcapError : public TcapEntity
{
public:
	TcapError(TcapDialog* dialog, UCHAR_T tag, UCHAR_T opcode) 
		: TcapEntity( dialog, tag, opcode ) {}
};

class TcapResult : public TcapEntity
{
public:
	TcapResult(TcapDialog* dialog, UCHAR_T tag, UCHAR_T opcode) : TcapEntity( dialog, tag, opcode ) {}
};

class TcapOperationListener
{
public:
	virtual void sucessed(TcapResult*) = 0;
	virtual void failed(TcapError*)    = 0;
};

class TcapOperation : public TcapEntity, ObservableT< TcapOperationListener >
{
public:
	TcapOperation(TcapDialog* dialog, UCHAR_T tag, UCHAR_T opcode) : TcapEntity( dialog, tag, opcode ) {}
  	virtual void invoke();
  	virtual void notifySuccessed(TcapResult*) const;
  	virtual void notifyFailed(TcapError*) const;
};


}
}
}
#endif
