#ident "$Id$"

#ifndef __SMSC_INMAN_PAYMENT__
#define __SMSC_INMAN_PAYMENT__

#include "billing_sm.h"
#include "interaction.hpp"
#include "inman/inap/inap.hpp"

using smsc::inman::inap::InapProtocol;

namespace smsc  	  {
namespace inman 	  {
namespace interaction {


class BillingInteraction : public Interaction
{
  public:
	BillingInteraction(InapProtocol*);
	virtual ~BillingInteraction();

	virtual void start();

  protected:
  	BillingInteractionContext* context;
  	InapProtocol* protocol;
};

}
}
}


#endif
