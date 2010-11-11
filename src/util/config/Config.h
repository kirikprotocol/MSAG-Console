#ifndef SMSC_UTIL_CONFIG_CONFIG
#define SMSC_UTIL_CONFIG_CONFIG

#ifndef __GNUC__
#pragma ident "$Id$"
#endif

#include <inttypes.h>
#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <xercesc/dom/DOM.hpp>
#include <core/buffers/Hash.hpp>
#include <logger/Logger.h>
#include <util/cstrings.h>
#include <util/config/ConfigException.h>

namespace smsc {
namespace util {
namespace config {

XERCES_CPP_NAMESPACE_USE
using smsc::core::buffers::Hash;
using smsc::core::buffers::HashInvalidKeyException;
using smsc::logger::Logger;
using smsc::util::cStringCopy;

class Manager;

typedef std::set<std::string> CStrSet;

class Config {
public:
    // friend class Manager;

    static Config* createFromFile( const char* xmlfile ); // throw configexception

    Config()
    {}

    Config(const DOMElement &element) throw (ConfigException)
    {
        parse(element);
    }

    ~Config()
    {
        char * value = NULL;
        char * _name = NULL;
        for (strParams.First(); strParams.Next(_name, value);)
            delete [] value;
        strParams.Empty();
        intParams.Empty();
        boolParams.Empty();
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
        throw HashInvalidKeyException(paramName);
    }

    /**
     *
     * @param paramName имя параметра
     * @return значение параметра типа String
     * @exception HashInvalidKeyException
     *                   if key not found
     * @see getInt()
     * @see getBool()
     * 
     * NOTE: returned value should not be deleted!
     */
    char * getString(const char * const paramName) const
        throw (HashInvalidKeyException)
    {
      if (strParams.Exists(paramName))
      {
        return strParams.Get(paramName);
      }
      else
        throw HashInvalidKeyException(paramName);
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
        throw HashInvalidKeyException(paramName);
    }

    void setInt(const char * const paramName, int32_t value)
    {
        intParams[paramName] = value;
    }

    void setString(const char * const paramName, const char * const value)
    {
        char* oldval;
        if ( strParams.Pop(paramName,oldval) ) {
            delete [] oldval;
        }
        strParams[paramName] = cStringCopy(value);
    }

    void setBool(const char * const paramName, bool value)
    {
        boolParams[paramName] = value;
    }

    /// saving the whole config to file
    /// NOTE: it is your responsibility to make sure that string values
    /// corresponds to the encoding specified.
    /// E.g. look at getLocalEncoding() in xml/utilFunctions.cpp.
    void saveToFile( const char* filename, const char* encoding = "windows-1251" ) const;

    char * getTextRepresentation() const
    {
        std::auto_ptr<ConfigTree> tree(createTree());
        return tree->getText("  ");
    }

    void removeSection(const char * const sectionName);

    /// create a copy of the section (w/ initial part removed, i.e. sectionName)
    Config* getSubConfig(const char* const sectionName, bool removeFromOriginal );

    //checks does the section with given absolute name exist having parameters defined
    bool    findSection(const char * const sectionName) const;
    //returns all nonempty root sections
    CStrSet *getRootSectionNames(void) const;
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
            ConfigTree * value = NULL;
            char * _name = NULL;
            for (sections.First(); sections.Next(_name, value);)
                delete value;
            sections.Empty();
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

private:
    void writeHeader( std::ostream& out, const char* encoding ) const;
    void save(std::ostream &out) const
    {
        std::auto_ptr<ConfigTree> tree(createTree());
        tree->write(out, "  ");
        out.flush();
    }
    void writeFooter( std::ostream& out ) const;
};

}
}
}
#endif // ifndef SMSC_UTIL_CONFIG_CONFIG
