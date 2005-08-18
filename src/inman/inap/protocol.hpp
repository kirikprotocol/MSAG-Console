#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_PROTOCOL__
#define __SMSC_INMAN_INAP_PROTOCOL__

#include "inman/common/observable.hpp"

using smsc::inman::common::ObservableT;


namespace smsc  {
namespace inman {
namespace inap  {

class TcapOperation;
class TcapDialog;
class Protocol;

class ProtocolFactory
{
public:
	virtual Protocol* createProtocol(TcapDialog*) = 0;
};

class ProtocolListener
{
public:
		virtual void invoke(TcapOperation*) = 0;
};

class Protocol : ObservableT< ProtocolListener >
{
public:
	Protocol(TcapDialog* dlg);
	virtual ~Protocol();
	virtual void invoke(TcapOperation*) = 0;
protected:
	TcapDialog* dialog;
};

}
}
}

#endif
