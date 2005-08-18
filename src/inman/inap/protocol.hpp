#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_PROTOCOL__
#define __SMSC_INMAN_INAP_PROTOCOL__

#include "inman/common/observable.hpp"

using smsc::inman::common::ObservableT;


namespace smsc  {
namespace inman {
namespace inap  {

class TcapDialog;
class Protocol;
class ProtocolEvent;
class ProtocolListener;

class ProtocolFactory
{
public:
	virtual Protocol*  		createProtocol(TcapDialog*) = 0;
	virtual ProtocolEvent* 	createEvent(TcapDialog*) 	= 0;
};

class ProtocolEvent
{
public:
	virtual void apply(Protocol*) = 0;
};

class ProtocolListener
{
public:
	virtual void dispatchEvent(ProtocolEvent* event) = 0;
};

class Protocol : ObservableT< ProtocolListener >
{
	public:

		Protocol(TcapDialog*);
		virtual ~Protocol();

	protected:

		TcapDialog* dialog;

		virtual void handleEvent(ProtocolEvent*) = 0;
};

}
}
}

#endif
