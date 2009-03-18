#include <util/Exception.hpp>
#include <eyeline/utilx/PreallocatedMemoryManager.hpp>

#include "LibSuaConnectAcceptor.hpp"
#include "LibSuaConnect.hpp"
#include "LibSuaConnectIndicationEvent.hpp"

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace sua_user_communication {

LibSuaConnectAcceptor::LibSuaConnectAcceptor(const std::string& acceptorName, const std::string& host, in_port_t listenPort)
  : _acceptorName(acceptorName), _serverSocket(new corex::io::network::TCPServerSocket(host.c_str(), listenPort))
{
  if ( !_serverSocket ) throw smsc::util::Exception("LibSuaConnectAcceptor::LibSuaConnectAcceptor::: can't create TCpServerSocket");
}

LibSuaConnectAcceptor::~LibSuaConnectAcceptor()
{
  delete _serverSocket;
}

io_dispatcher::Link*
LibSuaConnectAcceptor::accept() const
{
  return new LibSuaConnect(_serverSocket->accept(), this);
}

const std::string&
LibSuaConnectAcceptor::getName() const
{
  return _acceptorName;
}

io_dispatcher::IOEvent*
LibSuaConnectAcceptor::createIOEvent(const communication::LinkId& linkId) const
{
  utilx::alloc_mem_desc_t *ptr_desc = utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_EVENT>();
  return new (ptr_desc->allocated_memory) LibSuaConnectIndicationEvent(linkId);
}

corex::io::network::ServerSocket*
LibSuaConnectAcceptor::_getListenSocket() const
{
  return _serverSocket;
}

}}}}
