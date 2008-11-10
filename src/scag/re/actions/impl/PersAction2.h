#ifndef _PERS_ACTION_COMMIT_
#define _PERS_ACTION_COMMIT_

#include <string>

#include "scag/re/base/Action2.h"
#include "scag/re/base/LongCallAction2.h"
#include "scag/re/base/LongCallContext.h"

#include "scag/pers/util/Property.h"
#include "scag/pers/util/ProfileProxy.h"
#include "TimeField.h"

// #include "scag/pers/util/PersClient2.h"
// #include "scag/pers/util/Types.h"


namespace scag2 {
namespace re {
namespace actions {

// extern const char* OPTIONAL_KEY;
// extern uint32_t getKey(const CommandProperty& cp, ProfileType pt);

/*
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
 */

class PersActionCommand : public Action, public pers::util::PropertyProxy
{
public:
    PersActionCommand() :
    cmd(pers::util::PC_GET),
    mod(0),
    policy(pers::util::UNKNOWN),
    ftFinalDate(ftUnknown),
    finalDate(-1)
    {}

    PersActionCommand( pers::util::PersCmd c ) :
    cmd(c),
    mod(0),
    policy(pers::util::UNKNOWN),
    ftFinalDate(ftUnknown),
    finalDate(-1)
    {}

    virtual ~PersActionCommand() {}

    virtual void init( const SectionParams& params, PropertyObject propertyObject );

    /// command type
    virtual pers::util::PersCmd cmdType() const { return cmd; }

    /// writing to sb
    virtual int fillSB( ActionContext& ctx, util::storage::SerialBuffer& sb );

    /// reading from sb
    virtual int readSB( ActionContext& ctx, util::storage::SerialBuffer& sb );

    virtual const std::string& statusName() const { return status; }
    virtual const std::string& msgName() const { return msg; }

    inline const char* propertyName() const { return var.c_str(); }

    // bool RunBeforePostpone(ActionContext& context, PersCallParams* params);
    // void ContinueRunning(ActionContext& context);

    // const char* getVar();
    // bool batchPrepare(ActionContext& context, SerialBuffer& sb);
    // int batchResult(ActionContext& context, SerialBuffer& sb, bool transactMode = false);
    // const string& getStatus() { return status; };
    // const string& getMsg() { return msg; };

private:
    virtual IParserHandler* StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);

    inline const char* name() const { return "PersAction"; }

    // not used
    virtual bool run( ActionContext& context ) { return true; }

    pers::util::TimePolicy getPolicyFromStr( const std::string& str );
    time_t parseFinalDate(const char* s);

private:
    pers::util::PersCmd cmd;
    // --- property name
    FieldType ftVar;
    util::properties::Property::string_type var;
    // --- property value/increment
    FieldType ftValue;
    std::string sValue;
    // --- status/msg variables
    std::string status;
    std::string msg;
    // --- modulus for inc_mod
    FieldType ftModValue;
    std::string sMod;
    uint32_t mod;
    // --- time policy
    pers::util::TimePolicy policy;
    // --- final date for fixed policy
    FieldType ftFinalDate;
    std::string sFinalDate;
    time_t finalDate;
    // --- lifetime
    TimeField lifetime_;
    // --- variable name for inc and inc_mod result
    std::string sResult;
};


class PersActionBase : public LongCallAction 
{
public:
    PersActionBase(pers::util::PersCmd c):
    cmd(c),
    profile(pers::util::PT_UNKNOWN),
    optionalKeyInt(0),
    hasOptionalKey(false),
    ftOptionalKey(ftUnknown) 
    {}
    virtual ~PersActionBase() {}

protected:
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    pers::util::ProfileType getProfileTypeFromStr( const std::string& str );
    std::string getAbntAddress(const char* _address);
    /// set the profile key
    bool setKey( ActionContext& context, pers::util::PersCallParams* params );

protected:
    pers::util::PersCmd     cmd;
    pers::util::ProfileType profile;
    uint32_t                optionalKeyInt;
    std::string             optionalKeyStr;
    bool                    hasOptionalKey;
    FieldType               ftOptionalKey;
};


class PersAction : public PersActionBase 
{
public:
    PersAction() : PersActionBase(pers::util::PC_GET) {}
    PersAction(pers::util::PersCmd c) : PersActionBase(c), persCommand(c) {}
    virtual ~PersAction() {}
    virtual void init(const SectionParams& params, PropertyObject propertyObject);
    virtual bool RunBeforePostpone(ActionContext& context);
    virtual void ContinueRunning(ActionContext& context);

protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);

protected:
    PersActionCommand persCommand;
};

}//actions
}//re
}//scag2

#endif
