
#include "ConfigView.h"

namespace smsc { namespace util { namespace config
{
using smsc::util::Logger;

/* ----------------------- Tree Configuration Management ------------------- */
log4cpp::Category& ConfigView::log = 
    Logger::getCategory("smsc.util.config.ConfigView");

char* ConfigView::prepareSubSection(const char* sub)
{
    char* section = 0;
    if (!sub || sub[0] == '\0') return category;
    if (category)
    {
        section = new char[strlen(category)+strlen(sub)+2];
        sprintf(section, "%s.%s", category, sub);
    }
    else
    {
        section = new char[strlen(sub)+1];
        strcpy(section, sub);
    }
    return section;
}
ConfigView::ConfigView(Manager& manager, const char* cat)
    : config(manager), category(0)
{
    if (cat)
    {
        category = prepareSubSection(cat);
    }
}
ConfigView::~ConfigView()
{
    if (category) delete category;
}

ConfigView* ConfigView::getSubConfig(const char* sub, bool full)
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

CStrSet* ConfigView::getSectionNames()
    throw (ConfigException)
{
    return config.getChildSectionNames(category);
}
CStrSet* ConfigView::getIntParamNames()
    throw (ConfigException)
{
    return config.getChildIntParamNames(category);
}
CStrSet* ConfigView::getBoolParamNames()
    throw (ConfigException)
{
    return config.getChildBoolParamNames(category);
}
CStrSet* ConfigView::getStrParamNames()
    throw (ConfigException)
{
    return config.getChildStrParamNames(category);
}

int32_t ConfigView::getInt(const char* param, const char* error)
    throw (ConfigException)
{
    char* section = prepareSubSection(param);
    int32_t result;
    try 
    {
        result = config.getInt(section);
    }
    catch (ConfigException& exc)
    {
        log.warn("Config parameter missed: <%s>. %s",
                 section, (error) ? error:"");
        if (section) delete section;
        throw;

    }
    if (section) delete section;
    return result;
}
char* ConfigView::getString(const char* param, const char* error, bool check)
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
    catch (ConfigException& exc)
    {
        if (check)
            log.warn("Config parameter missed: <%s>. %s",
                     section, (error) ? error:"");
        if (section) delete section;
        if (check) throw;
        return 0;
    }
    if (section) delete section;
    return result;
}
bool ConfigView::getBool(const char* param, const char* error)
    throw (ConfigException)
{
    char* section = prepareSubSection(param);
    bool result;
    try 
    {
        result = config.getBool(section);
    }
    catch (ConfigException& exc)
    {
        log.warn("Config parameter missed: <%s>. %s",
                  section, (error) ? error:"");
        if (section) delete section;
        throw;
    }
    if (section) delete section;
    return result;
}

}}}
