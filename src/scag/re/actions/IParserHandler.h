#ifndef _IPARSER_HANDLER_H_
#define _IPARSER_HANDLER_H_

#include <core/buffers/Hash.hpp>
#include <util/Exception.hpp>


namespace scag { namespace re { 

using smsc::util::Exception;

typedef smsc::core::buffers::Hash<std::string> SectionParams;

class SemanticAnalyser;
class ActionFactory;

class IParserHandler
{
friend class SemanticAnalyser;

protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory) = 0;
    virtual bool FinishXMLSubSection(const std::string& name) = 0;
    virtual void SetChildObject(IParserHandler * child) {throw Exception("IParserHandler object must have no children");};
public:
    virtual void init(const SectionParams& params) = 0;
    virtual ~IParserHandler() {};
};


}}

#endif
