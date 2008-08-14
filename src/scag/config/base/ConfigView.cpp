#include "ConfigView.h"
#include "Config.h"

namespace scag {
namespace config {
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
ConfigView::ConfigView(const Config& config_, const char* cat)
    : log(Logger::getInstance("scag.onfig.ConfigView")),
        config(config_), category(0)
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
    catch (HashInvalidKeyException& exc)
    {
        smsc_log_warn(log, "Config parameter missed: <%s>. %s",
                 section, (error) ? error:"");

        char msg[512];
        sprintf(msg, "Config parameter missed: <%s>.", section);

        if (section) delete section;

        throw ConfigException(msg);

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
    catch (HashInvalidKeyException& exc)
    {
        if (check)
            smsc_log_warn(log, "Config parameter missed: <%s>. %s",
                     section, (error) ? error:"");

        char msg[512];
        sprintf(msg, "Config parameter missed: <%s>.", section);

        if (section) delete section;

        if (check) throw ConfigException(msg);
        return 0;
    }
    if (section) delete section;
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
    catch (HashInvalidKeyException& exc)
    {
        smsc_log_warn(log, "Config parameter missed: <%s>. %s",
                  section, (error) ? error:"");
        char msg[512];
        sprintf(msg, "Config parameter missed: <%s>.", section);

        if (section) delete section;

        throw ConfigException(msg);
    }
    if (section) delete section;
    return result;
}

}}
