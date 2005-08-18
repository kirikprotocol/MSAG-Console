static char const ident[] = "$Id$";
#include "protocol.hpp"

#include <assert.h>

namespace smsc  {
namespace inman {
namespace inap  {

Protocol::Protocol(TcapDialog*dlg) : dialog(dlg)
{
}

Protocol::~Protocol()
{
}

}
}
}