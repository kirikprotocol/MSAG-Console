#ifndef CommandUpdateRule_dot_h
#define CommandUpdateRule_dot_h

#include "SCAGCommand.h"
#include "CommandIds.h"

namespace scag {
namespace admin {

class CommandUpdateRule : public scag::admin::SCAGCommand
{
public:
  
  CommandUpdateRule(const xercesc::DOMDocument * doc);
  virtual ~CommandUpdateRule();
  virtual Response * CommandCreate(scag::Scag * SmscApp);

protected:
    int ruleId;
  
};

}
}

#endif
