#ifndef __EYELINE_SS7NA_COMMON_LMSUBSYSTEM_TELNETINTERACTION_HPP__
# define __EYELINE_SS7NA_COMMON_LMSUBSYSTEM_TELNETINTERACTION_HPP__

# include <string>
# include <netinet/in.h>

# include "logger/Logger.h"
# include "core/synchronization/Mutex.hpp"
# include "eyeline/corex/io/IOStreams.hpp"
# include "eyeline/corex/io/network/TCPServerSocket.hpp"
# include "eyeline/ss7na/common/lm_subsystem/UserInteractionProcessor.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace lm_subsystem {

class TelnetInteraction : public UserInteractionProcessor {
public:
  TelnetInteraction(const std::string& host, in_port_t port,
                    InputCommandProcessor& input_cmd_processor)
  : _logger(smsc::logger::Logger::getInstance("lm")),
    _serverSocket(new corex::io::network::TCPServerSocket(host, port)),
    _inputCmdProcessor(input_cmd_processor), _socket(NULL)
  {}

  virtual ~TelnetInteraction() {
    delete _socket;
    delete _serverSocket;
  }

  virtual int Execute();
  virtual void stop();
private:
  size_t readInputString(corex::io::InputStream* i_stream,
                         char* input_buf,
                         size_t input_buf_max_sz);

  void writeOutputString(corex::io::OutputStream* o_stream,
                         const char* output_str,
                         size_t output_str_len);

  std::string processUserInput(const std::string& input_string);

  int threadSafeCloseSocket();

  smsc::logger::Logger* _logger;
  corex::io::network::TCPServerSocket* _serverSocket;
  corex::io::network::TCPSocket* _socket;
  InputCommandProcessor& _inputCmdProcessor;
  smsc::core::synchronization::Mutex _socketLock;
};

}}}}

#endif
