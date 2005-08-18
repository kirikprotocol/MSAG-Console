static char const ident[] = "$Id$";
#include "operations.hpp"
#include "dialog.hpp"

namespace smsc {
namespace inman {
namespace inap {

TcapEntity::~TcapEntity()
{
  	delete param;
}

void TcapOperation::invoke()
{
	dialog->invoke( this );
}

void TcapOperation::notifySuccessed(TcapResult* res) const
{
	dialog->resultLast( res );
}

void TcapOperation::notifyFailed(TcapError* err ) const
{
	dialog->userError( err );
}

}
}
}
