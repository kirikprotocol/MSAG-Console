#ifndef SMSC_UTIL_CONFIG_CONFIG
#define SMSC_UTIL_CONFIG_CONFIG

#include <inttypes.h>
#include <iostream.h>
#include <xercesc/dom/DOM_Element.hpp>
#include <xercesc/dom/DOM_DOMException.hpp>
#include <log4cpp/Category.hh>
#include <core/buffers/Hash.hpp>
#include <util/Logger.h>
#include <util/cstrings.h>
#include <util/config/ConfigException.h>

namespace smsc {
namespace util {
namespace config {

using smsc::core::buffers::Hash;
using smsc::core::buffers::HashInvalidKeyException;
using smsc::util::Logger;
using smsc::util::cStringCopy;

class smsc::util::config::Manager;

typedef std::set<std::string> CStrSet;

class Config {
public:
    friend class smsc::util::config::Manager;

    Config()
    {}

    Config(const DOM_Element &element) throw (ConfigException)
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
     * @param paramName имя параметра
     * @return значение параметра типа int
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
     * @param paramName имя параметра
     * @return значение параметра типа String
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
     * @param paramName имя параметра
     * @return значение параметра типа Bool
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

    CStrSet *getChildSectionNames(const char * const sectionName);
    CStrSet *getChildIntParamNames(const char * const sectionName);
    CStrSet *getChildBoolParamNames(const char * const sectionName);
    CStrSet *getChildStrParamNames(const char * const sectionName);

protected:
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
    void parse(const DOM_Element &element) throw (ConfigException);
    void processNode(const DOM_Element &element, const char * const prefix) throw (DOM_DOMException);
    void processParamNode(const DOM_Element &element, const char * const name, const char * const type) throw (DOM_DOMException);
};

}
}
}
#endif // ifndef SMSC_UTIL_CONFIG_CONFIG
