#ifndef CommandRemoveRule_dot_h
#define CommandRemoveRule_dot_h

#include "SCAGCommand.h"
#include "CommandIds.h"

namespace scag {
namespace admin {

class CommandRemoveRule : public scag::admin::SCAGCommand
{
public:
  
  CommandRemoveRule(const xercesc::DOMDocument * doc);
  virtual ~CommandRemoveRule();
  virtual Response * CommandCreate(scag::Scag * SmscApp);

protected:
    int ruleId;
  
};

}
}

#endif
