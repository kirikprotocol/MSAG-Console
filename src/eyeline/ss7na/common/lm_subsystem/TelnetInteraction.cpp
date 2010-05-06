#include "eyeline/corex/io/network/TCPSocket.hpp"
#include "eyeline/utilx/hexdmp.hpp"
#include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"

#include "InputCommandProcessor.hpp"
#include "TelnetInteraction.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace lm_subsystem {

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
      smsc_log_debug(_logger, "TelnetInteraction::Execute::: started");
      _socket = _serverSocket->accept();

      corex::io::InputStream* iStream = _socket->getInputStream();
      corex::io::OutputStream* oStream = _socket->getOutputStream();

      const std::string& userPrompt = _inputCmdProcessor.getUserPrompt();
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
    smsc_log_error(_logger, "TelnetInteraction::Execute::: caught exception=[%s]", ex.what());
  } catch (...) {
    smsc_log_error(_logger, "TelnetInteraction::Execute::: caught unexpected exception=[...]");
  }
  return 0;
}

size_t
TelnetInteraction::readInputString(corex::io::InputStream* i_stream,
                                   char* input_buf,
                                   size_t input_buf_max_sz)
{
  size_t totalBytes=0;

  unsigned char* ptr = reinterpret_cast<unsigned char*>(input_buf);
  do {
    ptr += totalBytes;
    size_t byteWasRead = i_stream->read(ptr, input_buf_max_sz - totalBytes);
    totalBytes += byteWasRead;
    smsc_log_debug(_logger, "TelnetInteraction::readInputString::: got string=[%s]", utilx::hexdmp(ptr, byteWasRead).c_str());
  } while (totalBytes < input_buf_max_sz && input_buf[totalBytes-2] != '\r' && input_buf[totalBytes-1] != '\n');

  input_buf[totalBytes-2]=0;
  return totalBytes - 2;
}

void
TelnetInteraction::writeOutputString(corex::io::OutputStream* o_stream,
                                     const char* output_str,
                                     size_t output_str_len)
{
  size_t offset=0;
  while (offset<output_str_len)
    offset += o_stream->write(reinterpret_cast<const unsigned char*>(output_str+offset), output_str_len - offset);
}

std::string
TelnetInteraction::processUserInput(const std::string& input_string)
{
  return _inputCmdProcessor.process(input_string);
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
