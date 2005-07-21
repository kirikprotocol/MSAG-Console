#ifndef _IPARSER_HANDLER_H_
#define _IPARSER_HANDLER_H_

#include <core/buffers/Hash.hpp>
#include <util/Exception.hpp>


namespace scag { namespace re { 

using smsc::util::Exception;

typedef smsc::core::buffers::Hash<std::string> SectionParams;

class SemanticAnalyser;

class IParserHandler
{
friend class SemanticAnalyser;

protected:
    virtual void StartXMLSubSection(const std::string& name,const SectionParams& params) = 0;
    virtual void FinishXMLSubSection(const std::string& name) = 0;
    virtual void SetChildObject(IParserHandler * child) {throw Exception("IParserHandler object must have no children");};
public:
    virtual void init(const SectionParams& params) = 0;
    virtual ~IParserHandler() {};
};


}}

#endif
