#include <sua/sua_layer/io_dispatcher/Connection.hpp>
#include <sua/utilx/Exception.hpp>

namespace io_dispatcher {

Connection::~Connection() {}

const ConnectAcceptor*
Connection::getCreator() const
{
  return NULL;
}

communication::LinkId
Connection::getLinkId() const {
  return _linkId;
}

void
Connection::setLinkId(const communication::LinkId& linkId) {
  _linkId = linkId;
}

}
