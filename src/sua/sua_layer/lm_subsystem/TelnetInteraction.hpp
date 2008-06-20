#ifndef __SUA_SUALAYER_LMSUBSYSTEM_TELNETINTERACTION_HPP__
# define __SUA_SUALAYER_LMSUBSYSTEM_TELNETINTERACTION_HPP__

# include <string>
# include <netinet/in.h>
# include <logger/Logger.h>
# include <core/synchronization/Mutex.hpp>
# include <sua/corex/io/IOStreams.hpp>
# include <sua/corex/io/network/TCPServerSocket.hpp>
# include <sua/sua_layer/lm_subsystem/UserInteractionProcessor.hpp>

namespace lm_subsystem {

class TelnetInteraction : public UserInteractionProcessor {
public:
  TelnetInteraction(const std::string& host, in_port_t port);
  virtual ~TelnetInteraction();

  virtual int Execute();
  virtual void stop();
private:
  size_t readInputString(corex::io::InputStream* iStream,
                         char* inputBuf,
                         size_t inputBufMaxSz);

  void writeOutputString(corex::io::OutputStream* oStream,
                         const char* outputStr,
                         size_t outputStrLen);

  std::string processUserInput(const std::string& inputString);

  smsc::logger::Logger* _logger;
  corex::io::network::TCPServerSocket* _serverSocket;
  corex::io::network::TCPSocket* _socket;
  smsc::core::synchronization::Mutex _socketLock;
};

}

#endif
