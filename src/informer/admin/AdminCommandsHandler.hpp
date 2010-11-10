#ifndef __EYELINE_INFORMER_ADMIN_ADMINCOMMANDSHANDLER_HPP__
#define __EYELINE_INFORMER_ADMIN_ADMINCOMMANDSHANDLER_HPP__

#include "messages/ConfigOp.hpp"
#include "messages/SetDefaultSmsc.hpp"
#include "messages/LoggerGetCategories.hpp"
#include "messages/LoggerSetCategories.hpp"
#include "messages/SendTestSms.hpp"

namespace eyeline{
namespace informer{
namespace admin{

class AdminCommandsHandler{
public:
  virtual void handle(const messages::ConfigOp& cmd)=0;
  virtual void handle(const messages::SetDefaultSmsc& cmd)=0;
  virtual void handle(const messages::LoggerGetCategories& cmd)=0;
  virtual void handle(const messages::LoggerSetCategories& cmd)=0;
  virtual void handle(const messages::SendTestSms& cmd)=0;

};


}
}
}

#endif
