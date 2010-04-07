#include "util/Exception.hpp"
#include "eyeline/utilx/PreallocatedMemoryManager.hpp"

#include "LibSccpConnectAcceptor.hpp"
#include "LibSccpConnect.hpp"
#include "LibSccpConnectIndicationEvent.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace sccp_sap {

LibSccpConnectAcceptor::LibSccpConnectAcceptor(const std::string& acceptorName,
                                               const std::string& host,
                                               in_port_t listenPort)
  : _acceptorName(acceptorName), _serverSocket(new corex::io::network::TCPServerSocket(host.c_str(), listenPort))
{
  if ( !_serverSocket ) throw smsc::util::Exception("LibSccpConnectAcceptor::LibSccpConnectAcceptor::: can't create TCpServerSocket");
}

LibSccpConnectAcceptor::~LibSccpConnectAcceptor()
{
  delete _serverSocket;
}

io_dispatcher::Link*
LibSccpConnectAcceptor::accept() const
{
  return new LibSccpConnect(_serverSocket->accept(), this);
}

const std::string&
LibSccpConnectAcceptor::getName() const
{
  return _acceptorName;
}

io_dispatcher::IOEvent*
LibSccpConnectAcceptor::createIOEvent(const LinkId& linkId) const
{
  utilx::alloc_mem_desc_t *ptr_desc = utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_EVENT>();
  return new (ptr_desc->allocated_memory) LibSccpConnectIndicationEvent(linkId);
}

corex::io::network::TCPServerSocket*
LibSccpConnectAcceptor::_getListenSocket() const
{
  return _serverSocket;
}

}}}}
