#ifndef __SCAG_RULE_ENGINE_ACTION_DEL__
#define __SCAG_RULE_ENGINE_ACTION_DEL__

#include "scag/re/base/Action2.h"
#include "scag/re/base/StringField.h"

namespace scag2 {
namespace re {
namespace actions {

class ActionDel : public Action
{
public:

public:
    virtual const char* opname() const { return "del"; }
    virtual bool run( ActionContext& context );

    ActionDel() : varname_(*this,"var",true,false) {}
    virtual void init( const SectionParams& params, PropertyObject propertyObject );

protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);

private:
    ActionDel(const ActionDel &);
    ActionDel& operator = (const ActionDel&);

private:
    StringField varname_;
};

}}}

#endif
