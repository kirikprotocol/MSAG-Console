/* $Id$ */
#include "ConfigView.h"

namespace smsc { namespace util { namespace config
{
using smsc::logger::Logger;

/* ----------------------- Tree Configuration Management ------------------- */

char* ConfigView::prepareSubSection(const char* sub) const
{
    char* section = 0;
    do { // fake loop

      if ( category && category[0] != '\0' ) {
        if ( sub && sub[0] != '\0' ) {
          // %s.%s
          section = new char[strlen(category) + strlen(sub) + 2];
          sprintf(section, "%s.%s", category, sub );
          break;
        }
        sub = category;
      }

      if ( sub && sub[0] != '\0' ) {
        section = new char[strlen(sub)+1];
        strcpy(section, sub);
      }

    } while ( false );
    return section;
}

ConfigView::ConfigView(Manager& manager, const char* cat)
    : log(Logger::getInstance("smsc.util.config.ConfigView")),
        config(manager.getConfig()), category(0)
{
    if (cat)
    {
        category = prepareSubSection(cat);
    }
}

ConfigView::ConfigView(const Config& use_config, const char* cat)
    : log(Logger::getInstance("smsc.util.config.ConfigView")),
        config(use_config), category(0)
{
    if (cat)
    {
        category = prepareSubSection(cat);
    }
}

ConfigView::~ConfigView()
{
    if (category) delete [] category;
}

bool ConfigView::findSubSection(const char * const subsectionName) const
{
    //compose absolute name for subsection
    char* section = prepareSubSection(subsectionName);
    bool  res = config.findSection(section);
    delete section;
    return res;
}

ConfigView* ConfigView::getSubConfig(const char* sub, bool full) const
{
    ConfigView* dsc = 0;
    if (full)
    {
        dsc = new ConfigView(config, sub);
    }
    else
    {
        char* section = prepareSubSection(sub);
        dsc = new ConfigView(config, section);
        if (section) delete section;
    }
    return dsc;
}

CStrSet* ConfigView::getSectionNames() const
    throw (ConfigException)
{
    return config.getChildSectionNames(category);
}
CStrSet* ConfigView::getShortSectionNames() const
    throw (ConfigException)
{
    return config.getChildShortSectionNames(category);
}
CStrSet* ConfigView::getIntParamNames() const
    throw (ConfigException)
{
    return config.getChildIntParamNames(category);
}
CStrSet* ConfigView::getBoolParamNames() const
    throw (ConfigException)
{
    return config.getChildBoolParamNames(category);
}
CStrSet* ConfigView::getStrParamNames() const
    throw (ConfigException)
{
    return config.getChildStrParamNames(category);
}

int32_t ConfigView::getInt(const char* param, const char* error) const
    throw (ConfigException)
{
    char* section = prepareSubSection(param);
    int32_t result;
    try
    {
        result = config.getInt(section);
    }
    catch (const HashInvalidKeyException & exc)
    {
        std::string s("Int key \"");
        s += section;
        s += "\" not found";

        smsc_log_warn(log, "Config parameter missed: <%s>. %s",
                 section, (error) ? error:"");
        if (section) delete section;
        throw ConfigException(s.c_str());
    }
    if (section) delete section;
    return result;
}
char* ConfigView::getString(const char* param, const char* error, bool check) const
    throw (ConfigException)
{
    char* section = prepareSubSection(param);
    char* result;
    try
    {
        char* tmp = config.getString(section);
        result = new char[strlen(tmp)+1];
        strcpy(result, tmp);
    }
    catch (const HashInvalidKeyException & exc)
    {
        std::string s("String key \"");
        s += section;
        s += "\" not found";
        if (check)
            smsc_log_warn(log, "Config parameter missed: <%s>. %s",
                     section, (error) ? error:"");
        if (section) delete [] section;
        if (check) throw ConfigException(s.c_str());
        return 0;
    }
    if (section) delete [] section;
    return result;
}
bool ConfigView::getBool(const char* param, const char* error) const
    throw (ConfigException)
{
    char* section = prepareSubSection(param);
    bool result;
    try
    {
        result = config.getBool(section);
    }
    catch (const HashInvalidKeyException & exc)
    {
        std::string s("Bool key \"");
        s += section;
        s += "\" not found";

        smsc_log_warn(log, "Config parameter missed: <%s>. %s",
                  section, (error) ? error:"");

        if (section) delete section;
        throw ConfigException(s.c_str());
    }
    if (section) delete section;
    return result;
}

}}}
