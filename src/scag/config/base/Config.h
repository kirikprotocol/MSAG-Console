#ifndef SCAG_CONFIG_CONFIG
#define SCAG_CONFIG_CONFIG

#include <inttypes.h>
#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <xercesc/dom/DOM.hpp>
#include <core/buffers/Hash.hpp>
#include <logger/Logger.h>
#include <util/cstrings.h>
#include "ConfigException.h"

namespace scag {
namespace config {

XERCES_CPP_NAMESPACE_USE
using smsc::core::buffers::Hash;
using smsc::core::buffers::HashInvalidKeyException;
using smsc::logger::Logger;
using smsc::util::cStringCopy;

// class ConfigManagerImpl;

typedef std::set<std::string> CStrSet;

class Config 
{
public:
    // friend class ConfigManagerImpl;

    Config()
    {}

    Config(const DOMElement &element) throw (ConfigException)
    {
        parse(element);
    }

    ~Config()
    {
        strParams.Empty();
        intParams.Empty();
        boolParams.Empty();
        /*
        char * name;
        char * value;
        for (strParamsType::Iterator i = strParams.getIterator(); i.Next(name, value);)
        {
            strParams.Delete(name);
            if (value != 0)
            {
                delete value;
                delete name;
            }
        }

        int32_t ival;
        for (intParamsType::Iterator i = intParams.getIterator(); i.Next(name, ival);)
        {
            intParams.Delete(name);
            delete name;
        }

        bool bval;
        for (boolParamsType::Iterator i = boolParams.getIterator(); i.Next(name, bval);)
        {
            boolParams.Delete(name);
            delete name;
        }
        */
    }
    /**
     *
     * @param paramName ��� ���������
     * @return �������� ��������� ���� int
     * @exception HashInvalidKeyException
     *                   if key not found
     * @see getString()
     * @see getBool()
     */
    int32_t getInt(const char * const paramName) const
        throw (HashInvalidKeyException)
    {
      if (intParams.Exists(paramName))
      {
        return intParams.Get(paramName);
      }
      else
        throw HashInvalidKeyException();
    }

    /**
     *
     * @param paramName ��� ���������
     * @return �������� ��������� ���� String
     * @exception HashInvalidKeyException
     *                   if key not found
     * @see getInt()
     * @see getBool()
     */
    char * getString(const char * const paramName) const
        throw (HashInvalidKeyException)
    {
      if (strParams.Exists(paramName))
      {
        return strParams.Get(paramName);
      }
      else
        throw HashInvalidKeyException();
    }

    /**
     *
     * @param paramName ��� ���������
     * @return �������� ��������� ���� Bool
     * @exception HashInvalidKeyException
     *                   if key not found
     * @see getInt()
     * @see getString()
     */
    bool getBool(const char * const paramName) const
        throw (HashInvalidKeyException)
    {
      if (boolParams.Exists(paramName))
      {
        return boolParams.Get(paramName);
      }
      else
        throw HashInvalidKeyException();
    }

    void setInt(const char * const paramName, int32_t value)
    {
        intParams[paramName] = value;
    }

    void setString(const char * const paramName, const char * const value)
    {
        strParams[paramName] = cStringCopy(value);
    }

    void setBool(const char * const paramName, bool value)
    {
        boolParams[paramName] = value;
    }

    void save(std::ostream &out) const
    {
        std::auto_ptr<ConfigTree> tree(createTree());
        tree->write(out, "  ");
        out.flush();
    }

    char * getTextRepresentation() const
    {
        std::auto_ptr<ConfigTree> tree(createTree());
        return tree->getText("  ");
    }

    void removeSection(const char * const sectionName);

    void clean() throw (ConfigException);

    CStrSet *getChildSectionNames(const char * const sectionName) const;
    CStrSet* getChildShortSectionNames(const char * const sectionName) const;
    CStrSet *getChildIntParamNames(const char * const sectionName) const;
    CStrSet *getChildBoolParamNames(const char * const sectionName) const;
    CStrSet *getChildStrParamNames(const char * const sectionName) const;

//protected:
  template<void(*collect_func)(CStrSet &,
                const char * const ,
                const size_t ,
                const char * const)>
  CStrSet* _getChildSectionNames(const char * const sectionName) const;

    typedef Hash<int32_t> intParamsType;
    typedef Hash<char *> strParamsType;
    typedef Hash<bool> boolParamsType;
    intParamsType intParams;
    strParamsType strParams;
    boolParamsType boolParams;

    class ConfigParam
    {
    public:
        char * name;
        enum types {intType, stringType, boolType} type;
        char * value;

        ConfigParam(const char * const _name,
                                types _type,
                                const char * const _value)
        {
            name = strdup(_name);
            type = _type;
            value = strdup(_value);
        }
        ConfigParam(const ConfigParam &copy)
        {
            name = strdup(copy.name);
            type = copy.type;
            value = strdup(copy.value);
        }
        ~ConfigParam()
        {
            free(name);
            free(value);
        }
    };
    class ConfigTree
    {
    public:
        ConfigTree(const char * const _name)
        {
            name = strdup(_name);
        }
        /*ConfigTree(const ConfigTree &copy)
        {
            name = strdup(copy.name);
            params = copy.params;
            sections = copy.sections;
        }*/
        ~ConfigTree()
        {
            ConfigTree * value;
            char * _name;
            for (sections.First(); sections.Next(_name, value);)
            {
                delete value;
                delete _name;
                sections[_name]=0;
            }
            params.clear();
            free(name);
        }

        void addParam(const char * const name,
                      ConfigParam::types type,
                      const char * const value);
        void write(std::ostream &out, std::string prefix);
        char * getText(std::string prefix);
    private:
        Hash<ConfigTree*>   sections;
        std::vector<ConfigParam> params;
        char * name;
        ConfigTree* createSection(const char * const name);
    };

    ConfigTree * createTree() const;
    void parse(const DOMElement &element) throw (ConfigException);
    void processNode(const DOMElement &element, const char * const prefix) throw (DOMException);
    void processParamNode(const DOMElement &element, const char * const name, const char * const type) throw (DOMException);
};

}
}

namespace scag2 {
namespace config {
using scag::config::CStrSet;
using scag::config::Config;
}
}

#endif // ifndef SMSC_UTIL_CONFIG_CONFIG
