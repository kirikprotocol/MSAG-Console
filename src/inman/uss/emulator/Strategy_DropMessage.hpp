#ifndef __INMAN_USS_EMULATOR_STRATEGYDROPMESSAGE_HPP__
# define __INMAN_USS_EMULATOR_STRATEGYDROPMESSAGE_HPP__

# include <core/threads/ThreadedTask.hpp>
# include <inman/uss/ussmessages.hpp>
# include <inman/uss/emulator/MessageHandler.hpp>

namespace smsc  {
namespace inman {
namespace uss  {

class Strategy_DropMessage : public MessageHandler {
public:
  //  virtual int Execute();
  //  virtual const char* taskName() { return "Strategy_DropMessage"; }
  virtual void handle();
};

}}}

#endif
