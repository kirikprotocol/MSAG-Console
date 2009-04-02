#ifndef _PERS_ACTION_COMMIT_
#define _PERS_ACTION_COMMIT_

#include <string>

#include "scag/re/base/Action2.h"
#include "scag/re/base/LongCallAction2.h"
#include "scag/re/base/LongCallContext.h"

// #include "scag/pers/util/Property.h"
#include "scag/pvss/base/PersCommand.h"
#include "scag/pvss/base/PersCall.h"
#include "TimeField.h"
#include "scag/pvss/api/packets/ProfileCommandVisitor.h"
#include "scag/pvss/api/packets/AbstractCommand.h"

namespace scag2 {
namespace re {
namespace actions {

class PersActionResultRetriever
{
public:
    virtual ~PersActionResultRetriever() {}

    /*
    bool canProcessRequest( ActionContext& ctx );
    void setStatus( ActionContext& ctx, int status, int actionIdx = 0 );
    // std::auto_ptr< PersCallParams > makeParams( ActionContext& ctx );
    virtual void storeResults( const pvss::PersCommand& cmd, ActionContext& ctx );
    virtual pvss::PersCommand* makeCommand( ActionContext& ctx ) = 0;
    virtual pvss::PersCmd cmdType() const = 0;

    virtual const std::string& statusName() const = 0;
    virtual const std::string& msgName() const = 0;
     */
};


class PersActionCommand : public Action, public PersActionResultRetriever
{
public:
    PersActionCommand( pvss::AbstractCommand* c = 0 ) :
    cmdType_(c),
    mod(0),
    policy(pvss::UNKNOWN),
    ftFinalDate(ftUnknown),
    finalDate(-1)
    {}

    virtual ~PersActionCommand() {}

    virtual void init( const SectionParams& params, PropertyObject propertyObject );

    // command type
    // virtual pvss::PersCmd cmdType() const { return cmdType_; }

    virtual void storeResults( const pvss::PersCommand& command, ActionContext& ctx );
    virtual pvss::AbstractCommand* makeCommand( ActionContext& ctx, int& status );
    int fillCommand( ActionContext& ctx, pvss::PersCommandSingle& command );

    virtual const std::string& statusName() const { return status; }
    virtual const std::string& msgName() const { return msg; }

    inline const char* propertyName() const { return var.c_str(); }

private:
    virtual IParserHandler* StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);

    inline const char* name() const { return "PersAction"; }

    // not used
    virtual bool run( ActionContext& context ) { return true; }

    pvss::TimePolicy getPolicyFromStr( const std::string& str );
    time_t parseFinalDate(const char* s);

private:
    class XMLParser : public pvss::ProfileCommandVisitor
    {
    public:
        XMLParser( PersActionCommand& cmd,
                   const SectionParams& params,
                   PropertyObject propertyObject ) :
        cmd_(cmd), params_(params), propertyObject_(propertyObject) {}

        virtual bool visitDelCommand( pvss::DelCommand& cmd ) { initCommon(); return true; }
        virtual bool visitIncCommand( pvss::IncCommand& cmd ) { initCommon(); parseInc(); initValue(); initTime(); return true; }
        virtual bool visitIncModCommand( pvss::IncModCommand& cmd ) { initCommon(); parseInc(); initValue(); initMod(); initTime(); return true; }
        virtual bool visitGetCommand( pvss::GetCommand& cmd ) { initCommon(); parseValue(); initValue(); return true; }
        virtual bool visitSetCommand( pvss::SetCommand& cmd ) { initCommon(); parseValue(); initValue(); initTime(); return true; }
        virtual bool visitBatchCommand( pvss::BatchCommand& cmd ) { return false; }

    private:
        void initCommon();
        void parseInc();
        void parseValue();
        void initValue();
        void initMod();
        void initTime();

    private:
        PersActionCommand&   cmd_;
        const SectionParams& params_;
        PropertyObject       propertyObject_;
    };

    std::auto_ptr<pvss::AbstractCommand>  cmdType_;  // used as a prototype
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
    pvss::TimePolicy policy;
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
    PersActionBase():
    // cmdType_(c),
    profile(pvss::PT_UNKNOWN),
    optionalKeyInt(0),
    hasOptionalKey(false),
    ftOptionalKey(ftUnknown) 
    {}
    virtual ~PersActionBase() {}

protected:
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    pvss::ProfileType getProfileTypeFromStr( const std::string& str );
    std::string getAbntAddress(const char* _address);
    std::auto_ptr< lcm::LongCallParams > makeParams( ActionContext& context,
                                                     PersActionResultRetriever& creator );
    virtual void ContinueRunning(ActionContext& context);
    virtual PersActionResultRetriever& results() = 0;

protected:
    // pvss::PersCmd     cmdType_;
    pvss::ProfileType profile;
    uint32_t                optionalKeyInt;
    std::string             optionalKeyStr;
    bool                    hasOptionalKey;
    FieldType               ftOptionalKey;
};


class PersAction : public PersActionBase 
{
public:
    PersAction() : PersActionBase() {}
    PersAction(pvss::AbstractCommand* c) : PersActionBase(), persCommand(c) {}
    virtual ~PersAction() {}
    virtual void init(const SectionParams& params, PropertyObject propertyObject);
    virtual bool RunBeforePostpone(ActionContext& context);

protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
    virtual void storeResults( const pvss::PersCommand& cmd, ActionContext& ctx ) {
        persCommand.storeResults( cmd, ctx );
    }
    virtual PersActionResultRetriever& results() { return persCommand; }

protected:
    PersActionCommand persCommand;
};

}//actions
}//re
}//scag2

#endif
