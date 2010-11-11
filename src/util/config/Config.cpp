#include "Config.h"

#include <xercesc/dom/DOM.hpp>
#include <memory>
#include <iostream>
#include <fstream>

#include "util/xml/init.h"
#include "util/xml/DOMTreeReader.h"
#include <util/xml/utilFunctions.h>

#include "core/buffers/TmpBuf.hpp"
#include "core/buffers/File.hpp"

namespace {

class FileStreamBuf : public std::basic_streambuf<char>
{
public:
  void Open(const char* fileName)
  {
    f.RWCreate(fileName);
  }
  int sync()
  {
    try{
      f.Flush();
    }catch(...)
    {
      return -1;
    }
    return 0;
  }
  std::streamsize xsputn(const char_type* s,std::streamsize n)
  {
    try{
      f.Write(s,n);
    }catch(...)
    {
      return -1;
    }
    return n;
  }
protected:
  smsc::core::buffers::File f;
};


struct StrHashAdapter
{
public:
    typedef Hash< char* > HT;
    typedef HT::value_type value_type;
    typedef HT::Iterator Iterator;

    inline StrHashAdapter( HT& h ) : params(h) {}
    inline int Insert( const char* const paramName, const char* value ) {
        char** ptr = params.GetPtr(paramName);
        char* val = smsc::util::cStringCopy(value);
        if ( ptr ) {
            delete[] *ptr;
            *ptr = val;
            return 1;
        } else {
            return params.Insert(paramName,val);
        }
    }
    inline void Delete( const char* const paramName ) {
        char** ptr = params.GetPtr(paramName);
        if (ptr) delete [] *ptr;
        params.Delete(paramName);
    }

    Iterator getIterator() { return params.getIterator(); }

private:
    HT& params;
};


template < class HashType > void removeSectionFromHash
    ( HashType& hash,
      const char * const sectionName,
      bool remove = true,
      HashType* out = 0)
{
    const size_t sectionNameLen = strlen(sectionName);
    char *name;
    typename HashType::value_type value;
    for ( typename HashType::Iterator i = hash.getIterator(); i.Next(name,value); ) {
        if (strlen(name) >= sectionNameLen
            && name[sectionNameLen] == '.'
            && memcmp(name, sectionName, sectionNameLen) == 0) {
            if ( out ) { out->Insert( name+sectionNameLen+1, value ); }
            if ( remove ) hash.Delete( name );
        }
    }
}

} // namespace

namespace smsc {
namespace util {
namespace config {

using namespace xercesc;
using namespace std;
using namespace smsc::util::xml;
using smsc::core::buffers::TmpBuf;



Config* Config::createFromFile( const char* xmlfile )
{
    initXerces();
    if ( !xmlfile || *xmlfile == '\0' ) {
        throw ConfigException("config file is not specified");
    }
    struct stat s;
    int rc = stat(xmlfile,&s);
    std::auto_ptr<char> tmp;
    if ( *xmlfile != '/' ) {
        if ( rc == 0 && S_ISREG(s.st_mode) ) {
            // found
        } else {
            tmp.reset(new char[strlen(xmlfile) + 10]);
            strcpy(tmp.get(),"conf/");
            strcat(tmp.get(),xmlfile);
            if ( stat(tmp.get(),&s) == 0 && S_ISREG(s.st_mode) ) {
                // found
                xmlfile = tmp.get();
            } else {
                strcpy(tmp.get(),"../conf/");
                strcat(tmp.get(),xmlfile);
                if ( stat(tmp.get(),&s) == 0 && S_ISREG(s.st_mode) ) {
                    // found
                    xmlfile = tmp.get();
                } else {
                    throw ConfigException("config file '%s' is not found", xmlfile );
                }
            }
        }
    } else if ( rc == 0 && S_ISREG(s.st_mode) ) {
        // ok
    } else {
        throw ConfigException("config file '%s' is not found", xmlfile);
    }

    // trying to open the file
    std::auto_ptr< Config > config;
    try {
        DOMTreeReader reader;
        DOMDocument* document = reader.read(xmlfile);
        DOMElement* elem;
        if ( document && (elem = document->getDocumentElement()) ) {
            config.reset( new Config );
            config->parse(*elem);
        } else {
            throw ConfigException("config file '%s' parsed to null");
        }
    } catch ( ParseException& e ) {
        throw ConfigException(e.what());
    }
    return config.release();
}


void Config::parse(const DOMElement &element)
  throw (ConfigException)
{
  try {
    processNode(element,"");
  }
  catch (DOMException &e)
  {
    std::string s("Exception on processing config tree, nested: ");
    std::auto_ptr<char> msg(XMLString::transcode(e.msg));
    s += msg.get();
    throw ConfigException(s.c_str());
  }
  catch (...)
  {
    throw ConfigException("Exception on processing config tree");
  }
}

void Config::processNode(const DOMElement &element,
                         const char * const prefix)
  throw (DOMException)
{
  DOMNodeList *list = element.getChildNodes();
  size_t listLength = list->getLength();
  for (size_t i=0; i<listLength; i++)
  {
    DOMNode *n = list->item(i);
    if (n->getNodeType() == DOMNode::ELEMENT_NODE)
    {
      DOMElement *e = (DOMElement*)(n);
      XmlStr name(e->getAttribute(XmlStr("name")));
      auto_ptr<char> fullName(new char[strlen(prefix) +1 +strlen(name) +1]);
      if (prefix[0] != 0)
      {
        strcpy(fullName.get(), prefix);
        strcat(fullName.get(), ".");
        strcat(fullName.get(), name);
      } else {
        strcpy(fullName.get(), name);
      }
      XmlStr nodeName(e->getNodeName());
      if (strcmp(nodeName, "section") == 0)
      {
        processNode(*e, fullName.get());
      }
      else if (strcmp(nodeName, "param") == 0)
      {
        processParamNode(*e, fullName.get(), XmlStr(e->getAttribute(XmlStr("type"))));
      }
      else
      {
        smsc_log_warn(Logger::getInstance("smsc.util.config.Config"), "Unknown node \"%s\" in section \"%s\"", nodeName.c_str(), prefix);
      }
    }
  }
}

void Config::processParamNode(const DOMElement &element,
                              const char * const name,
                              const char * const type)
  throw (DOMException)
{
  //getting value
  std::auto_ptr<char> value(getNodeText(element));

  if (strcmp(type, "string") == 0)
  {
    setString(name, value.get());
  }
  else if (strcmp(type, "int") == 0)
  {
    setInt(name, atoi(value.get()));
  }
  else if (strcmp(type, "bool") == 0)
  {
    if ((strcmp(value.get(), "true") == 0)
        || (strcmp(value.get(), "on") == 0)
        || (strcmp(value.get(), "yes") == 0))
    {
      setBool(name, true);
    }
    else if ((strcmp(value.get(), "false") == 0)
             || (strcmp(value.get(), "off") == 0)
             || (strcmp(value.get(), "no") == 0))
    {
      setBool(name, false);
    }
    else
    {
      smsc_log_warn(Logger::getInstance("smsc.util.config.Config"), "Unrecognized boolean value \"%s\" for parameter \"%s\". Setted to FALSE.", value.get(), name);
      setBool(name, false);
    }
  }
  else
  {
    smsc_log_warn(Logger::getInstance("smsc.util.config.Config"), "Unrecognized parameter type \"%s\" for parameter \"%s\"", type, name);
  }
}

Config::ConfigTree * Config::createTree() const
{
  ConfigTree * result = new ConfigTree("root");

  char * _name;

  char * svalue;
  for (Hash<char *>::Iterator i= strParams.getIterator(); i.Next(_name, svalue);)
    result->addParam(_name, ConfigParam::stringType, svalue);

  int32_t lvalue;
  char tmp[33];
  for (Hash<int32_t>::Iterator i = intParams.getIterator(); i.Next(_name, lvalue);)
  {
    snprintf(tmp, sizeof(tmp), "%i", lvalue);
    result->addParam(_name, ConfigParam::intType, tmp);
  }

  bool bvalue;
  for (Hash<bool>::Iterator i = boolParams.getIterator(); i.Next(_name, bvalue);)
    result->addParam(_name, ConfigParam::boolType, bvalue ? "true" : "false");

  return result;
}

void Config::ConfigTree::addParam(const char * const _name,
                                   ConfigParam::types type,
                                   const char * const value)
{
  char *sname = strdup(_name);
  char *p = strrchr(sname, '.');
  if (p != 0)
  {
    *p = 0;
    ConfigTree* node = createSection(sname);
    node->params.push_back(ConfigParam(p+1, type, value));
  }
  else {
    params.push_back(ConfigParam(_name, type, value));
  }
  free(sname);
}

void Config::ConfigTree::write(std::ostream &out, std::string prefix)
{
  char * _name;
  ConfigTree *val;
  std::string newPrefix(prefix + "  ");
  for(sections.First(); sections.Next(_name, val);)
  {
    std::auto_ptr<char> tmp_name(encode(_name));
    out << prefix << "<section name=\"" << tmp_name.get() << "\">" << std::endl;
    val->write(out, newPrefix);
    out << prefix << "</section>" << std::endl;
  }

  for (size_t i=0; i<params.size(); i++)
  {
    std::auto_ptr<char> paramName(encode(params[i].name));
    std::auto_ptr<char> paramValue(encode(params[i].value));
    out << prefix << "<param name=\"" << paramName.get() << "\" type=\"";
    switch (params[i].type)
    {
    case ConfigParam::boolType:
      out << "bool";
      break;
    case ConfigParam::intType:
      out << "int";
      break;
    case ConfigParam::stringType:
      out << "string";
      break;
    }
    out << "\">" << paramValue.get() << "</param>" << std::endl;
  }
}

char * Config::ConfigTree::getText(std::string prefix)
{
  std::string result;
  char * _name;
  ConfigTree *val;
  std::string newPrefix(prefix + "  ");
  for(sections.First(); sections.Next(_name, val);)
  {
    std::auto_ptr<char> tmp_name(encode(_name));
    result += prefix;
    result += "<section name=\"";
    result += tmp_name.get();
    result += "\">\n";
    char * tmp = val->getText(newPrefix);
    result += tmp;
    delete[] tmp;
    result += prefix;
    result += "</section>\n";
  }

  for (size_t i=0; i<params.size(); i++)
  {
    std::auto_ptr<char> paramName(encode(params[i].name));
    std::auto_ptr<char> paramValue(encode(params[i].value));
    result += prefix;
    result += "<param name=\"";
    result += paramName.get();
    result += "\" type=\"";
    switch (params[i].type)
    {
    case ConfigParam::boolType:
      result += "bool";
      break;
    case ConfigParam::intType:
      result += "int";
      break;
    case ConfigParam::stringType:
      result += "string";
      break;
    }
    result += "\">";
    result += paramValue.get();
    result += "</param>\n";
  }
  char * str_result = new char[result.size() +1];
  strcpy(str_result, result.c_str());
  return str_result;
}

Config::ConfigTree* Config::ConfigTree::createSection(const char * const _name)
{
  ConfigTree *t = this;
  char * newName = strdup(_name);
  char * n = newName;
  for (char * p = strchr(n, '.'); p != 0; p = strchr(n, '.'))
  {
    *p=0;
    if (!t->sections.Exists(n))
    {
      t->sections[n] = new ConfigTree(n);
    }
    t = t->sections[n];
    n = p+1;
  }
  if (!t->sections.Exists(n))
  {
    t->sections[n] = new ConfigTree(n);
  }
  return t->sections[n];
}

void Config::saveToFile( const char* filename, const char* encoding ) const
{
    /*
    FileStreamBuf buf;
    buf.Open(filename);
    std::ostream out(&buf);
    // ((std::basic_ios<char>&)out).rdbuf(&buf);
    writeHeader(out);
    save(out);
    writeFooter(out);
    out.flush();
     */
    std::ofstream out;
    out.open(filename, std::ios_base::out | std::ios_base::trunc );
    if ( !out ) {
        throw smsc::core::buffers::FileException(FileException::errOpenFailed,filename);
    }
    writeHeader(out, encoding);
    save(out);
    writeFooter(out);
    out.close();
}


void Config::removeSection(const char * const sectionName)
{
    removeSectionFromHash( intParams, sectionName );
    removeSectionFromHash( boolParams, sectionName );
    StrHashAdapter strHash(strParams);
    removeSectionFromHash( strHash, sectionName );
}

Config* Config::getSubConfig( const char* const sectionName, bool remove )
{
    std::auto_ptr< Config > ret( new Config() );
    removeSectionFromHash( intParams, sectionName, remove, &ret->intParams );
    removeSectionFromHash( boolParams, sectionName, remove, &ret->boolParams );
    StrHashAdapter from(strParams);
    StrHashAdapter into(ret->strParams);
    removeSectionFromHash( from, sectionName, remove, &into );
    return ret.release();
}


void collect_section_names_into_set(CStrSet &result,
                  const char * const sectionName,
                  const size_t sectionNameLen,
                  const char * const name)
{
  if (strlen(name) >= sectionNameLen
    && name[sectionNameLen] == '.'
    && memcmp(name, sectionName, sectionNameLen) == 0)
  {
    const char* dotpos = strchr(name+sectionNameLen+1, '.');
    if (dotpos != 0)
    {
      //char sectName[dotpos - name+1];
      TmpBuf<char,1024> sectName(dotpos - name+1);
      memcpy(sectName, name, dotpos - name);
      sectName[dotpos - name] = 0;
      result.insert(std::string(sectName));
    }
  }
}

void collect_short_section_names_into_set(CStrSet &result,
                      const char * const sectionName,
                      const size_t sectionNameLen,
                      const char * const name)
{
  if (strlen(name) >= sectionNameLen
    && name[sectionNameLen] == '.'
    && memcmp(name, sectionName, sectionNameLen) == 0)
  {
    const char* dotpos = strchr(name+sectionNameLen+1, '.');
    if (dotpos != 0)
    {
      //char sectName[dotpos - name+1];
      TmpBuf<char,1024> sectName(dotpos - name+1);
      memcpy(sectName, name, dotpos - name);
      sectName[dotpos - name] = 0;
      char * shortSectName = strrchr(sectName, '.');
      if (shortSectName == NULL)
        shortSectName = sectName;
      else
        shortSectName++;

      result.insert(std::string(shortSectName));
    }
  }
}

template <class _HashType, class _HashIteratorType, class _ValueType>
bool findSectionInHash(const _HashType & hash, const char * const tgtName,
                                            const size_t tgtNameLen)
{
    char *        name;
    _ValueType    value;

    for (_HashIteratorType i = hash.getIterator(); i.Next(name, value); ) {
        if ((strlen(name) >= tgtNameLen)
            && (name[tgtNameLen] == '.')
            && !memcmp(name, tgtName, tgtNameLen))
            return true;
    }
    return false;
}

template <class _HashType, class _HashIteratorType, class _ValueType>
void getRootSectionsFromHash(const _HashType & hash, CStrSet &result)
{
    char *        name;
    _ValueType    value;

    for (_HashIteratorType i = hash.getIterator(); i.Next(name, value); ) {
        const char * dotpos = strchr(name, '.');
        if (dotpos) { //parameter from some section
            size_t sectionNameLen = dotpos - name;
            TmpBuf<char,64> sectName(sectionNameLen + 1);
            memcpy(sectName, name, sectionNameLen);
            sectName[sectionNameLen] = 0;
            result.insert(std::string(sectName));
        }
    }
}


template<class _HashType, class _HashIteratorType, class _ValueType, void(*collect_func)(CStrSet &,
                          const char * const ,
                          const size_t ,
                          const char * const)>
void getChildSectionsFromHash(const _HashType & hash,
                const char * const sectionName,
                size_t sectionNameLen,
                CStrSet &result)
{
  char *name;
  _ValueType value;
  for (_HashIteratorType i = hash.getIterator(); i.Next(name, value); )
  {
    collect_func(result, sectionName, sectionNameLen, name);
  }
}

template<class _HashType, class _HashIteratorType, class _ValueType>
void getChildParamsFromHash(const _HashType & hash,
              const char * const sectionName,
              size_t sectionNameLen,
              CStrSet &result)
{
  char *name;
  _ValueType value;
  for (_HashIteratorType i = hash.getIterator(); i.Next(name, value); )
  {
    if (strlen(name) >= sectionNameLen && memcmp(name, sectionName, sectionNameLen) == 0)
    {
      //char paramName[strlen(name) -(sectionNameLen +1) +1];
      TmpBuf<char,64> paramName(strlen(name) -(sectionNameLen +1) +1);
      strcpy(paramName, name +(sectionNameLen +1));
      result.insert(std::string(paramName));
    }
  }
}

template<void(*collect_func)(CStrSet &,
               const char * const ,
               const size_t ,
               const char * const)>
CStrSet* Config::_getChildSectionNames(const char * const sectionName) const
{
  CStrSet *result = new CStrSet;
  const size_t sectionNameLen = sectionName[strlen(sectionName)-1] == '.'
    ? strlen(sectionName)-1
    : strlen(sectionName);

  getChildSectionsFromHash<intParamsType, intParamsType::Iterator, int32_t, collect_func>
    (intParams, sectionName, sectionNameLen, *result);

  getChildSectionsFromHash<boolParamsType, boolParamsType::Iterator, bool, collect_func>
    (boolParams, sectionName, sectionNameLen, *result);

  getChildSectionsFromHash<strParamsType, strParamsType::Iterator, char *, collect_func>
    (strParams, sectionName, sectionNameLen, *result);

  return result;
}

bool Config::findSection(const char * const sectionName) const
{
    size_t sectNameLen = strlen(sectionName);
    if (!findSectionInHash<intParamsType, intParamsType::Iterator, int32_t>
                                        (intParams, sectionName, sectNameLen)
        && !findSectionInHash<boolParamsType, boolParamsType::Iterator, bool>
                                        (boolParams, sectionName, sectNameLen)
        && !findSectionInHash<strParamsType, strParamsType::Iterator, char*>
                                        (strParams, sectionName, sectNameLen))
        return false;
    return true;
}

CStrSet* Config::getRootSectionNames(void) const
{
    CStrSet *result = new CStrSet;
    getRootSectionsFromHash<intParamsType, intParamsType::Iterator, int32_t>(intParams, *result);
    getRootSectionsFromHash<boolParamsType, boolParamsType::Iterator, bool>(boolParams, *result);
    getRootSectionsFromHash<strParamsType, strParamsType::Iterator, char *>(strParams, *result);
    return result;
}

CStrSet* Config::getChildSectionNames(const char * const sectionName) const
{
  return _getChildSectionNames<collect_section_names_into_set>(sectionName);
}

CStrSet* Config::getChildShortSectionNames(const char * const sectionName) const
{
  return _getChildSectionNames<collect_short_section_names_into_set>(sectionName);
}

CStrSet * Config::getChildIntParamNames(const char * const sectionName) const
{
  std::auto_ptr<CStrSet> result(new CStrSet);
  const size_t sectionNameLen = sectionName[strlen(sectionName)-1] == '.'
    ? strlen(sectionName)-1
    : strlen(sectionName);

  getChildParamsFromHash<intParamsType, intParamsType::Iterator, int32_t>
    (intParams, sectionName, sectionNameLen, *result);

  return result.release();
}

CStrSet * Config::getChildBoolParamNames(const char * const sectionName) const
{
  std::auto_ptr<CStrSet> result(new CStrSet);
  const size_t sectionNameLen = sectionName[strlen(sectionName)-1] == '.'
    ? strlen(sectionName)-1
    : strlen(sectionName);

  getChildParamsFromHash<boolParamsType, boolParamsType::Iterator, bool>
    (boolParams, sectionName, sectionNameLen, *result);

  return result.release();
}

CStrSet * Config::getChildStrParamNames(const char * const sectionName) const
{
  std::auto_ptr<CStrSet> result(new CStrSet);
  const size_t sectionNameLen = sectionName[strlen(sectionName)-1] == '.'
    ? strlen(sectionName)-1
    : strlen(sectionName);

  getChildParamsFromHash<strParamsType, strParamsType::Iterator, char *>
    (strParams, sectionName, sectionNameLen, *result);

  return result.release();
}


void Config::writeHeader(std::ostream &out, const char* encoding) const
{
  out << "<?xml version=\"1.0\" encoding=\"" << encoding << "\"?>" << std::endl;
  out << "<!DOCTYPE config SYSTEM \"configuration.dtd\">" << std::endl;
  out << "<config>" << std::endl;
}

void Config::writeFooter(std::ostream &out) const
{
  out << "</config>" << std::endl;
}

}
}
}
