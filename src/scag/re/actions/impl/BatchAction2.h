#ifndef _BATCH_PERS_ACTION_COMMIT_
#define _BATCH_PERS_ACTION_COMMIT_

#include <string>

#include "PersAction2.h"
//#include "scag/pers/PersClient.h"

namespace scag2 { namespace re { namespace actions {  

//using namespace scag::re::actions;
//using namespace scag::re;
//using client::PersCallParams;

class BatchAction : public PersActionBase {
public:
  BatchAction():PersActionBase(PC_MTBATCH), transactMode(false) {}
  ~BatchAction() {}
  virtual bool RunBeforePostpone(ActionContext& context);
  virtual void ContinueRunning(ActionContext& context);
  virtual void init(const SectionParams& params,PropertyObject propertyObject);

protected:
  virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
  virtual bool FinishXMLSubSection(const std::string& name);
  void setStatus(ActionContext& context, PersCallParams *params, int status, const std::string& msg, int action_idx = 0);

protected:
  std::vector<PersActionCommand *> actions;
  PropertyObject pobj;
  bool transactMode;
  std::string status_str, msg_str;
};

}
}
}

#endif
