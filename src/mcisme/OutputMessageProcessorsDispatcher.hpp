#ifndef __SMSC_MCISME_OUTPUTMESSAGEPROCESSORSDISPATCHER_HPP__
# define __SMSC_MCISME_OUTPUTMESSAGEPROCESSORSDISPATCHER_HPP__

# include <mcisme/AbntAddr.hpp>

namespace smsc {
namespace mcisme {

class OutputMessageProcessor;

class OutputMessageProcessorsDispatcher {
public:
  virtual void dispatch(const AbntAddr& abnt) = 0;
  virtual void markMessageProcessorAsFree(OutputMessageProcessor* freeMessageProcessor) = 0;
  virtual void deleteMessageProcessor(OutputMessageProcessor* terminatedMessageProcessor) = 0;
  virtual void shutdown() {}
};

}}

#endif
