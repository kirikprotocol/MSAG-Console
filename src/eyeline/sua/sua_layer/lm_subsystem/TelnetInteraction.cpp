#include <eyeline/corex/io/network/TCPSocket.hpp>
#include <eyeline/utilx/hexdmp.hpp>
#include <eyeline/utilx/runtime_cfg/RuntimeConfig.hpp>

#include "InputCommandProcessor.hpp"
#include "TelnetInteraction.hpp"

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace lm_subsystem {

TelnetInteraction::TelnetInteraction(const std::string& host, in_port_t port)
  : _logger(smsc::logger::Logger::getInstance("lm_subsys")), 
    _serverSocket(new corex::io::network::TCPServerSocket(host, port)),
    _socket(NULL)
{}

TelnetInteraction::~TelnetInteraction()
{
  delete _socket;
  delete _serverSocket;
}

int
TelnetInteraction::threadSafeCloseSocket()
{
  smsc::core::synchronization::MutexGuard synchronize(_socketLock);
  delete _socket; _socket = NULL;
  return 0;
}

int
TelnetInteraction::Execute()
{
  try {
    while(true) {
      _socket = _serverSocket->accept();

      corex::io::InputStream* iStream = _socket->getInputStream();
      corex::io::OutputStream* oStream = _socket->getOutputStream();

      const std::string& userPrompt = lm_subsystem::InputCommandProcessor::getInstance().getUserPrompt();
      writeOutputString(oStream, userPrompt.data(), userPrompt.size());

      char inputString[512];
      try {
        while (true) {
          size_t inputStringLen = readInputString(iStream, inputString, sizeof(inputString));

          const std::string& responseString =  processUserInput(inputString);

          writeOutputString(oStream, responseString.data(), responseString.size());
        }
      } catch(UserTerminateSessionException& ex) {
        smsc_log_info(_logger, "User has terminated the session");
        threadSafeCloseSocket();
      } catch (corex::io::EOFException& ex) {
        smsc_log_info(_logger, "Connection closed by remote side");
      } catch (corex::io::BrokenPipe& ex) {
        smsc_log_info(_logger, "Broken pipe");
      } catch (...) {
        threadSafeCloseSocket();
      }
      threadSafeCloseSocket();
    }
  } catch (std::exception& ex) {
    smsc_log_error(_logger, "TelnetInteraction::Execute::: catched exception=[%s]", ex.what());
  } catch (...) {
    smsc_log_error(_logger, "TelnetInteraction::Execute::: catched unexpected exception=[...]");
  }
  return 0;
}

size_t
TelnetInteraction::readInputString(corex::io::InputStream* iStream,
                                   char* inputBuf,
                                   size_t inputBufMaxSz)
{
  size_t totalBytes=0;

  unsigned char* ptr = reinterpret_cast<unsigned char*>(inputBuf);
  do {
    ptr += totalBytes;
    size_t byteWasRead = iStream->read(ptr, inputBufMaxSz - totalBytes);
    totalBytes += byteWasRead;
    smsc_log_debug(_logger, "TelnetInteraction::readInputString::: got string=[%s]", utilx::hexdmp(ptr, byteWasRead).c_str());
  } while (totalBytes < inputBufMaxSz && inputBuf[totalBytes-2] != '\r' && inputBuf[totalBytes-1] != '\n');

  inputBuf[totalBytes-2]=0;
  return totalBytes - 2;
}

void
TelnetInteraction::writeOutputString(corex::io::OutputStream* oStream,
                                     const char* outputStr,
                                     size_t outputStrLen)
{
  size_t offset=0;
  while (offset<outputStrLen)
    offset += oStream->write(reinterpret_cast<const unsigned char*>(outputStr+offset), outputStrLen - offset);
}

std::string
TelnetInteraction::processUserInput(const std::string& inputString)
{
  return lm_subsystem::InputCommandProcessor::getInstance().process(inputString);
}


void
TelnetInteraction::stop()
{
  {
    smsc::core::synchronization::MutexGuard synchronize(_socketLock);
    if ( _socket ) _socket->close();
  }
  if ( _serverSocket )_serverSocket->close();
}

}}}}
