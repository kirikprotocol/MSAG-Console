static char const ident[] = "$Id$";
#include <assert.h>

#include "billing.hpp"
#include "inman/inap/inap.hpp"

using smsc::inman::inap::InapProtocol;

namespace smsc  {
namespace inman {
namespace interaction  {

BillingInteraction::BillingInteraction(InapProtocol* prot) : protocol( prot )
{
	context = new BillingInteractionContext( *this );
	assert( protocol );
}

BillingInteraction::~BillingInteraction()
{
	delete context;
}

void BillingInteraction::start()
{
	//todo: delegete to context
}


}
}
}
