#ifndef _PERS_ACTION_COMMIT_
#define _PERS_ACTION_COMMIT_

#include "scag/pers/Types.h"
#include <string>

#include "scag/re/actions/Action.h"
#include "scag/re/actions/LongCallAction.h"
#include "scag/pers/Property.h"

#include "scag/pers/PersClient.h"

namespace scag { namespace pers {

using namespace scag::re::actions;
using namespace scag::re;
using namespace scag::pers::client;

extern const char* OPTIONAL_KEY;
extern uint32_t getKey(const CommandProperty& cp, ProfileType pt);


class PersActionCommand : public Action {
public:
  PersActionCommand() : cmd(PC_GET), finalDate(-1), lifeTime(-1), policy(UNKNOWN),
                        ftLifeTime(ftUnknown), ftFinalDate(ftUnknown), mod(0) {};
  PersActionCommand(PersCmd c) : cmd(c), finalDate(-1), lifeTime(-1), policy(UNKNOWN),
                                 ftLifeTime(ftUnknown), ftFinalDate(ftUnknown), mod(0) {};
  virtual ~PersActionCommand() {};
  void init(const SectionParams& params, PropertyObject propertyObject);
  bool run(ActionContext& context) { return true; };
  IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
  bool FinishXMLSubSection(const std::string& name);
  bool RunBeforePostpone(ProfileType pt, ActionContext& context, PersCallParams* params);
  void ContinueRunning(ActionContext& context);
  const char* getVar();
  bool batchPrepare(ProfileType pt, ActionContext& context, SerialBuffer& sb);
  int batchResult(ActionContext& context, SerialBuffer& sb, bool transactMode = false);

private:
  TimePolicy getPolicyFromStr(const std::string& str);
  time_t parseFinalDate(const std::string& s);
  uint32_t parseLifeTime(const std::string& s);

private:
  PersCmd cmd;
  TimePolicy policy;
  time_t finalDate;
  uint32_t lifeTime;
  uint32_t mod;
  FieldType ftLifeTime;
  FieldType ftFinalDate;
  FieldType ftValue;
  FieldType ftModValue;
  FieldType ftVar;
  std::string sLifeTime;
  std::string sFinalDate;
  std::string sValue;
  std::string sMod;
  std::string sResult;
  std::string var;
  std::string status;
  std::string msg;
};

class PersActionBase : public LongCallAction {
public:
  PersActionBase(PersCmd c): cmd(c), profile(PT_UNKNOWN), hasOptionalKey(false),
                             optionalIkey(0), ftOptionalKey(ftUnknown) {};
  virtual ~PersActionBase() {};

protected:
  virtual void init(const SectionParams& params,PropertyObject propertyObject);
  ProfileType getProfileTypeFromStr(const std::string& str);
  void getOptionalKey(const std::string& key_str);
  std::string getAbntAddress(const std::string& _address);

protected:
  PersCmd cmd;
  ProfileType profile;
  std::string optionalSkey;
  uint32_t optionalIkey;
  std::string optionalKeyStr;
  bool hasOptionalKey;
  FieldType ftOptionalKey;
};

class PersAction : public PersActionBase {
public:
  PersAction() : PersActionBase(PC_GET) {}
  PersAction(PersCmd c) : PersActionBase(c), persCommand(c) {}
  ~PersAction() {}
  virtual bool RunBeforePostpone(ActionContext& context);
  virtual void ContinueRunning(ActionContext& context);
  virtual void init(const SectionParams& params, PropertyObject propertyObject);

protected:
  virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
  virtual bool FinishXMLSubSection(const std::string& name);

protected:
  PersActionCommand persCommand;
};

}}

#endif
