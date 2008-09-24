#ifndef _PERS_ACTION_COMMIT_
#define _PERS_ACTION_COMMIT_

#include <string>

#include "scag/re/base/Action2.h"
#include "scag/re/base/LongCallAction2.h"
#include "scag/re/base/LongCallContext.h"

#include "scag/pers/util/Property.h"
#include "scag/pers/util/PersClient.h"
#include "scag/pers/util/Types.h"

namespace scag2 { namespace re { namespace actions {

using namespace scag::pers::util;
using std::string;

extern const char* OPTIONAL_KEY;
extern uint32_t getKey(const CommandProperty& cp, ProfileType pt);

class PersCallParams : public LongCallParams{
public:
    PersCallParams() : error(0), result(0) {};
    ProfileType pt;
	SerialBuffer sb;
	uint32_t ikey;
    string skey;
    string propName;
	scag::pers::util::Property prop;
    uint32_t mod;
    int32_t error;
    uint32_t result;
};

class PersActionCommand : public Action {
public:
  PersActionCommand() : cmd(PC_GET), finalDate(-1), lifeTime(-1), policy(UNKNOWN),
                        ftLifeTime(ftUnknown), ftFinalDate(ftUnknown), mod(0) {};
  PersActionCommand(PersCmd c) : cmd(c), finalDate(-1), lifeTime(-1), policy(UNKNOWN),
                                 ftLifeTime(ftUnknown), ftFinalDate(ftUnknown), mod(0) {};
  virtual ~PersActionCommand() {};
  void init(const SectionParams& params, PropertyObject propertyObject);
  bool run(ActionContext& context) { return true; };
  IParserHandler * StartXMLSubSection(const string& name,const SectionParams& params,const ActionFactory& factory);
  bool FinishXMLSubSection(const string& name);
  bool RunBeforePostpone(ActionContext& context, PersCallParams* params);
  void ContinueRunning(ActionContext& context);
  const char* getVar();
  bool batchPrepare(ActionContext& context, SerialBuffer& sb);
  int batchResult(ActionContext& context, SerialBuffer& sb, bool transactMode = false);
  const string& getStatus() { return status; };
  const string& getMsg() { return msg; };

private:
  TimePolicy getPolicyFromStr(const string& str);
  time_t parseFinalDate(const string& s);
  uint32_t parseLifeTime(const string& s);

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
  string sLifeTime;
  string sFinalDate;
  string sValue;
  string sMod;
  string sResult;
  string var;
  string status;
  string msg;
};

class PersActionBase : public LongCallAction {
public:
  PersActionBase(PersCmd c): cmd(c), profile(PT_UNKNOWN), hasOptionalKey(false),
                             optionalIkey(0), ftOptionalKey(ftUnknown) {};
  virtual ~PersActionBase() {};

protected:
  virtual void init(const SectionParams& params,PropertyObject propertyObject);
  ProfileType getProfileTypeFromStr(const string& str);
  void getOptionalKey(const string& key_str);
  string getAbntAddress(const string& _address);
  bool setKey(ActionContext& context, PersCallParams* params);
  void setStatus(ActionContext& context, int status, const string& statusName, const string& msgName, int actionIdx = 0);
  bool checkConnection(ActionContext& context, const string& statusName, const string& msgName);

protected:
  PersCmd cmd;
  ProfileType profile;
  string optionalSkey;
  uint32_t optionalIkey;
  string optionalKeyStr;
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
  virtual IParserHandler * StartXMLSubSection(const string& name,const SectionParams& params,const ActionFactory& factory);
  virtual bool FinishXMLSubSection(const string& name);

protected:
  PersActionCommand persCommand;
};

}//actions
}//re
}//scag2

#endif
