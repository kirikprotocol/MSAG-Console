
#include <ctype.h>
#include <util/debug.h>
#include <locale.h>
#include "Entities.h"
#include "core/buffers/TmpBuf.hpp"

namespace smsc { namespace util { namespace templates
{

const char* SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION  = "arg";
const char* SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION   = "default";
const char* SMSC_DBSME_IO_FORMAT_PATTERN_OPTION   = "pattern";
const char* SMSC_DBSME_IO_FORMAT_PRECISION_OPTION = "precision";
const char* SMSC_DBSME_IO_FORMAT_EXPONENT_OPTION  = "exponent";
const char* SMSC_DBSME_IO_FORMAT_DIGITS_OPTION    = "digits";
const char* SMSC_DBSME_IO_FORMAT_EXPORT_OPTION    = "export";
const char* SMSC_DBSME_IO_FORMAT_IMPORT_OPTION    = "import";

const char* SMSC_DBSME_MSG_TEXT         = "msg-text";

const char*  ioNowString         = "now";
const char*  ioTodayString       = "today";
const char*  ioTomorrowString    = "tomorrow";
const char*  ioYesterdayString   = "yesterday";
const char*  ioInternalPatternString = "internal";


const char* SMSC_DBSME_IO_DEFAULT_FORMAT_PATTERN  = "dd.MM.yyyy HH:mm:ss";
const char* SMSC_DBSME_IO_DEFAULT_PARSE_PATTERN   = "%d.%d.%d %d:%d:%d %n";

const char*  ioDayTimeParts[2] = {
    "AM", "PM"
};


using smsc::core::buffers::Hash;
using smsc::core::buffers::TmpBuf;

    const char*  ioFullMonthesNames[12] = {
        "January", "February", "March", "April",
        "May", "June", "July", "August", "September",
        "October", "November", "December"
    };
    const char*  ioShortMonthesNames[12] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    const char*  ioFullWeekDays[7] = {
        "Sunday", "Monday", "Tuesday", "Wednesday",
        "Thursday", "Friday", "Saturday"
    };
    const char*  ioShortWeekDays[7] = {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };

struct InitLocaleDataDummy{
  InitLocaleDataDummy()
  {
    const char *oldLocale=setlocale(LC_TIME,0);
    const char *newLocale=setlocale(LC_TIME,"");
    if(newLocale==0)
    {
      fprintf(stderr,"Failed to set LC_TIME from environment\n");
      return;
    };
    fprintf(stderr,"setlocale for LC_TIME to %s\n",newLocale);
    tm t={0,};
    static char mon1[12][32];
    static char mon2[12][32];
    static char week1[7][32];
    static char week2[7][32];
    for(int i=0;i<12;i++)
    {
      t.tm_mon=i;
      strftime(mon1[i],32,"%b",&t);
      strftime(mon2[i],32,"%B",&t);
      ioShortMonthesNames[i]=mon1[i];
      ioFullMonthesNames[i]=mon2[i];
    }
    for(int i=0;i<7;i++)
    {
      t.tm_wday=i;
      strftime(week1[i],32,"%a",&t);
      strftime(week2[i],32,"%A",&t);
      ioShortWeekDays[i]=week1[i];
      ioFullWeekDays[i]=week2[i];
    }
    setlocale(LC_TIME,oldLocale);
  }
}initLocaleDummy;


static bool ENTITY_TRACE = false;

const char* FormatEntity::getOption(const char* name)
{
    if (options.Exists(name))
    {
        //std::string str = options.Get(name);
        return options.Get(name).c_str();
    }
    return 0;
}

void FormatEntity::renderOptions(const char* line)
    throw(FormatRenderingException)
{
    int curPos = 0;

    while (line && line[curPos] != '\0')
    {
        std::string name = ""; std::string value = "";

        while (isspace(line[curPos])) curPos++; // skip spaces before 'name'
        while (isalnum(line[curPos])) name += line[curPos++];
        if (!name.length())
        {
            throw FormatRenderingException("FormatRenderingException: "
                "Argument name expected. Parsing line: <%s>", line);
        }
        while (isspace(line[curPos])) curPos++; // skip spaces after 'name'
        if (line[curPos++] != SMSC_DBSME_IO_FORMAT_OPTION_ASSIGN)
            throw FormatRenderingException("FormatRenderingException: "
                "'%c' sign expected. Parsing line: <%s>",
                SMSC_DBSME_IO_FORMAT_OPTION_ASSIGN, line);
        while (isspace(line[curPos])) curPos++; // skip spaces before 'value'

        if (line[curPos] == SMSC_DBSME_IO_FORMAT_STRING_DELIMETER)
        {
            curPos++;
            bool stop = false;
            bool escape = false;
            while (line[curPos] && !stop)
            {
                switch(line[curPos])
                {
                case SMSC_DBSME_IO_FORMAT_ENTITY_ESCAPER:
                    if (!escape && line[curPos+1] != '\0')
                    {
                        escape = true; curPos++;
                        continue;
                    }
                    break;
                case SMSC_DBSME_IO_FORMAT_STRING_DELIMETER:
                    if (!escape)
                    {
                        stop = true; curPos++;
                        continue;
                    }
                    break;
                default:
                    if (escape)
                    {
                        value += SMSC_DBSME_IO_FORMAT_ENTITY_ESCAPER;
                    }
                    break;
                }

                escape = false;
                value += line[curPos++];
            }
            if (!stop)
            {
                throw FormatRenderingException("FormatRenderingException: "
                    "'%c' sign expected. Parsing line: <%s>",
                    SMSC_DBSME_IO_FORMAT_STRING_DELIMETER, line);
            }
        }
        else
        {
            while (isalnum(line[curPos]) ||
                   line[curPos] == '.' || line[curPos] == ',')
                value += line[curPos++];
        }

        if (ENTITY_TRACE) __trace2__("Result %s=%s", name.c_str(), value.c_str());
        options.Insert(name.c_str(), value);

        while (isspace(line[curPos])) curPos++; // skip spaces after 'value'
    }
}

// io == false --> parser, io == true --> formatter
FormatEntity::FormatEntity(std::string line, bool io, bool _type)
    throw(FormatRenderingException)
{
    str = "";
    char* raw = (char *)line.c_str();

    if (_type)
    {
        while (isspace(*raw)) raw++;

        int i;
        for (i=0; i<ioEntityTypesNumber-1; i++)
        {
            if (ENTITY_TRACE) __trace2__("Checking type: %s", ioEntityTypeStrings[i]);
            if (raw == strstr(raw, ioEntityTypeStrings[i]))
            {
                this->type = (EntityType)i;
                break;
            }
        }
        if (i >= ioEntityTypesNumber-1 ||
            !isspace(raw[strlen(ioEntityTypeStrings[i])]))
        {
            throw FormatRenderingException("FormatRenderingException: "
                                           "Argument type is not supported. "
                                           "Parsing format string: <%s>", raw);
        }
        renderOptions(raw+strlen(ioEntityTypeStrings[i]));

        const char* arg = getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
        const char* def = getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        const char* imp = getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
        const char* exp = getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);

        if (io) // formatter
        {
            if (!arg && !def && !imp)
                throw FormatRenderingException(
                    "Neither '%s' nor '%s' nor '%s' option defined ! "
                    "Parsing output format string: <%s>",
                    SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION,
                    SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION,
                    SMSC_DBSME_IO_FORMAT_IMPORT_OPTION, raw);
        }
        else    // parser
        {
            if (!arg && !exp)
                throw FormatRenderingException(
                    "Neither '%s' nor '%s' option wasn't defined ! "
                    "Parsing input format string: <%s>",
                    SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION,
                    SMSC_DBSME_IO_FORMAT_EXPORT_OPTION, raw);
        }
    }
    else
    {
        this->type = ET_TEXT; int curPos = 0; bool escape = false;
        // Replace all \\->\, \" -> "
        while (raw && raw[curPos] != '\0')
        {
            if (raw[curPos] == SMSC_DBSME_IO_FORMAT_ENTITY_ESCAPER)
            {
                if (!escape) escape = true;
                else
                {
                    str += SMSC_DBSME_IO_FORMAT_ENTITY_ESCAPER;
                    escape = false;
                }
            }
            else
            {
                if (escape) escape = false;
                str += raw[curPos];
            }
            curPos++;
        }
        if (escape) str += SMSC_DBSME_IO_FORMAT_ENTITY_ESCAPER;
    }
}
FormatEntity::~FormatEntity()
{
}

FormatEntityRenderer::FormatEntityRenderer(const char* format, bool io)
    throw(FormatRenderingException)
{
    if (ENTITY_TRACE) __trace2__("Format for parsing is: '%s'", format);

    std::string str = "";

    int     curPos = 0;
    bool    opened = false;
    bool    escape = false;
    bool    ending = false;

    while (format)
    {
        switch (format[curPos])
        {
        case SMSC_DBSME_IO_FORMAT_ENTITY_ESCAPER:
            if (!escape) escape = true;
            else
            {
                str += SMSC_DBSME_IO_FORMAT_ENTITY_ESCAPER;
                str += SMSC_DBSME_IO_FORMAT_ENTITY_ESCAPER;
                escape = false;
            }
            curPos++;
            continue;

        case SMSC_DBSME_IO_FORMAT_ENTITY_DELIMETER:
            if (escape ||
                format[curPos+1] != SMSC_DBSME_IO_FORMAT_ENTITY_DELIMETER)
            {
                str += format[curPos++];
                escape = false;
                continue;
            }
            curPos += 2;
            ending = false;
            break;

        case '\0':
            if (escape)
            {
                str += SMSC_DBSME_IO_FORMAT_ENTITY_ESCAPER;
                escape = false;
            }
            ending = true;
            break;

        default:
            if (escape)
            {
                str += SMSC_DBSME_IO_FORMAT_ENTITY_ESCAPER;
                escape = false;
            }
            str += format[curPos++];
            continue;
        }

        if (ENTITY_TRACE) {
            const char* line = str.c_str();
            __trace2__("%s line: <%s>", (opened) ? "Inner":"Outer", line);
        }

        if (opened || io)
        {
            try
            {
                if (opened || str.length() > 0)
                {
                    (void)entities.Push(new FormatEntity(str, io, opened));
                }
            }
            catch (Exception& exc)
            {
                clearEntities();
                throw;
            }
        }

        if (ending) break;
        opened = !opened;
        str = "";
    }

    if (opened)
        throw FormatRenderingException("Closeup symbols expected '%c%c' !"
                                       "Parsing format string: <%s>",
                                       SMSC_DBSME_IO_FORMAT_ENTITY_DELIMETER,
                                       SMSC_DBSME_IO_FORMAT_ENTITY_DELIMETER,
                                       format ? format:"");
}
void FormatEntityRenderer::clearEntities()
{
    while (entities.Count())
    {
        FormatEntity* entity=0L;
        (void) entities.Pop(entity);
        if (entity) delete entity;
    }
}
FormatEntityRenderer::~FormatEntityRenderer()
{
    clearEntities();
}
int FormatEntityRenderer::getEntityType(const char* key, const char* option)
{
    if (!key || !option) return -1;

    for (int i=0; i<entities.Count(); i++)
    {
        FormatEntity* entity = entities[i];
        if (!entity) continue;
        const char* value = entity->getOption(option);
        if (value && strcmp(key, value) == 0) return entity->type;
    }
    return -1;
}


void ContextEnvironment::toUpperCase(const char* str, char* low)
{
    __require__(str && low);

    do *low++=toupper(*str);
    while (*str++);
}
bool ContextEnvironment::exportStr(const char* key, const char* val)
{
    if (!key || !val) return false;
    //char ukey[strlen(key)+1];
    TmpBuf<char,64> ukey(strlen(key)+1);
    toUpperCase(key, ukey);
    if (strs.Exists(ukey)) strs.Delete(ukey);
    strs.Insert(ukey, std::string(val));
    return true;
}
bool ContextEnvironment::importStr(const char* key, char* &val)
{
    if (!key) return false;
    //char ukey[strlen(key)+1];
    TmpBuf<char,64> ukey(strlen(key)+1);
    toUpperCase(key, ukey);
    if (!strs.Exists(ukey)) return false;
    val = (char *)(strs.Get(ukey).c_str());
    return true;
}
bool ContextEnvironment::exportInt(const char* key, int64_t val)
{
    if (!key) return false;
    //char ukey[strlen(key)+1];
    TmpBuf<char,64> ukey(strlen(key)+1);
    toUpperCase(key, ukey);
    if (ints.Exists(ukey)) ints.Delete(ukey);
    ints.Insert(ukey, val);
    return true;
}
bool ContextEnvironment::importInt(const char* key, int64_t &val)
{
    if (!key) return false;
    //char ukey[strlen(key)+1];
    TmpBuf<char,64> ukey(strlen(key)+1);
    toUpperCase(key, ukey);
    if (!ints.Exists(ukey)) return false;
    val = ints.Get(ukey);
    return true;
}
bool ContextEnvironment::exportDat(const char* key, time_t val)
{
    if (!key) return false;
    //char ukey[strlen(key)+1];
    TmpBuf<char,64> ukey(strlen(key)+1);
    toUpperCase(key, ukey);
    if (dats.Exists(ukey)) dats.Delete(ukey);
    dats.Insert(ukey, val);
    return true;
}
bool ContextEnvironment::importDat(const char* key, time_t &val)
{
    if (!key) return false;
    //char ukey[strlen(key)+1];
    TmpBuf<char,64> ukey(strlen(key)+1);
    toUpperCase(key, ukey);
    if (!dats.Exists(ukey)) return false;
    val = dats.Get(ukey);
    return true;
}
bool ContextEnvironment::exportFlt(const char* key, float val)
{
    if (!key) return false;
    //char ukey[strlen(key)+1];
    TmpBuf<char,64> ukey(strlen(key)+1);
    toUpperCase(key, ukey);
    if (flts.Exists(ukey)) flts.Delete(ukey);
    flts.Insert(ukey, val);
    return true;
}
bool ContextEnvironment::importFlt(const char* key, float &val)
{
    if (!key) return false;
    //char ukey[strlen(key)+1];
    TmpBuf<char,64> ukey(strlen(key)+1);
    toUpperCase(key, ukey);
    if (!flts.Exists(ukey)) return false;
    val = flts.Get(ukey);
    return true;
}
bool ContextEnvironment::exportDbl(const char* key, double val)
{
    if (!key) return false;
    //char ukey[strlen(key)+1];
    TmpBuf<char,64> ukey(strlen(key)+1);
    toUpperCase(key, ukey);
    if (dbls.Exists(ukey)) dbls.Delete(ukey);
    dbls.Insert(ukey, val);
    return true;
}
bool ContextEnvironment::importDbl(const char* key, double &val)
{
    if (!key) return false;
    //char ukey[strlen(key)+1];
    TmpBuf<char,64> ukey(strlen(key)+1);
    toUpperCase(key, ukey);
    if (!dbls.Exists(ukey)) return false;
    val = dbls.Get(ukey);
    return true;
}
bool ContextEnvironment::exportLdl(const char* key, long double val)
{
    if (!key) return false;
    //char ukey[strlen(key)+1];
    TmpBuf<char,64> ukey(strlen(key)+1);
    toUpperCase(key, ukey);
    if (ldls.Exists(ukey)) ldls.Delete(ukey);
    ldls.Insert(ukey, val);
    return true;
}
bool ContextEnvironment::importLdl(const char* key, long double &val)
{
    if (!key) return false;
    //char ukey[strlen(key)+1];
    TmpBuf<char,64> ukey(strlen(key)+1);
    toUpperCase(key, ukey);
    if (!ldls.Exists(ukey)) return false;
    val = ldls.Get(ukey);
    return true;
}

void ContextEnvironment::reset()
{
    strs.Empty(); ints.Empty(); dats.Empty();
    flts.Empty(); dbls.Empty(); ldls.Empty();
}

}}}
