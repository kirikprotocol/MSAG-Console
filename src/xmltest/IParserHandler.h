#ifndef _IPARSER_HANDLER_H_
#define _IPARSER_HANDLER_H_

#include <core/buffers/Hash.hpp>

class SemanticAnalyser;

typedef smsc::core::buffers::Hash<std::string> SectionParams;

class IParserHandler
{
friend class SemanticAnalyser;

protected:
    virtual void StartXMLSubSection(const std::string& name,const SectionParams& params)  {}
    virtual void FinishXMLSubSection(const std::string& name) {}
    virtual bool SetChildObject(const IParserHandler * child) { return false;}
public:
    virtual ~IParserHandler() {};
};



#endif
