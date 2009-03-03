#include "Acceptor.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

Acceptor::Acceptor( const std::string& host, short port ) :
host_(host), port_(port)
{
}

void Acceptor::startup()
{
}

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2
