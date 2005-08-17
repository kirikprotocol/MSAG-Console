#ident "$Id$"

#ifndef __SMSC_INMAN_INTERACTION_INTERACTION__
#define __SMSC_INMAN_INTERACTION_INTERACTION__

#include "inman/common/observable.hpp"

using smsc::inman::common::ObservableT;

namespace smsc  {
namespace inman {
namespace interaction {

class Interaction;

class InteractionListener
{
public:
	virtual void finished(const Interaction*, unsigned short status) = 0;
};

class Interaction : public ObservableT< InteractionListener >
{
public:
	virtual	void start() = 0;
	
};


}
}
}


#endif
