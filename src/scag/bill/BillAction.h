#ifndef _BILL_ACTION_
#define _BILL_ACTION_

#include "scag/re/actions/Action.h"
#include "scag/re/actions/ActionFactory.h"

#include <logger/Logger.h>
#include <string>

namespace scag { namespace bill {

using namespace scag::re::actions;
using namespace scag::re;
using smsc::logger::Logger;


    class BillAction : public Action
    {
    protected:
        std::string m_sName;
        std::string m_sServiceName;
        std::string m_sStatus;
        std::string m_sMessage;
        static Logger * logger;

        virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
        virtual bool FinishXMLSubSection(const std::string& name);
    public:
    //    virtual bool run(ActionContext& context);
        virtual void init(const SectionParams& params,PropertyObject propertyObject);
    };

}}


#endif


