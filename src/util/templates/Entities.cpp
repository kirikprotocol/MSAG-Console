
#include <ctype.h>
#include <util/debug.h>

#include "Entities.h"

namespace smsc { namespace util { namespace templates
{

using smsc::core::buffers::Hash;

const char* FormatEntity::getOption(const char* name)
{
    if (options.Exists(name))
    {
        std::string str = options.Get(name);
        return str.c_str();
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

        __trace2__("Result %s=%s", name.c_str(), value.c_str());
        options.Insert(name.c_str(), value);
        
        while (isspace(line[curPos])) curPos++; // skip spaces after 'value'
    }
}

// io == false --> parser, io == true --> formatter
FormatEntity::FormatEntity(std::string line, bool io, bool type)
    throw(FormatRenderingException)
{
    str = "";
    char* raw = (char *)line.c_str();

    if (type)
    {
        while (isspace(*raw)) raw++;

        int i;
        for (i=0; i<ioEntityTypesNumber-1; i++)
        {
            //__trace2__("Checking type: %s", ioEntityTypeStrings[i]);
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
        this->type = TEXT; int curPos = 0; bool escape = false;
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
    __trace2__("Format for parsing is: '%s'", format);
    
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

        const char* line = str.c_str();
        __trace2__("%s line: <%s>", (opened) ? "Inner":"Outer", line);
        
        if (opened || io)
        {
            try
            {
                (void)entities.Push(new FormatEntity(str, io, opened));
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


bool ContextEnvironment::exportStr(const char* key, const char* val)
{
    if (!key || !val) return false;
    if (strs.Exists(key)) strs.Delete(key);
    strs.Insert(key, std::string(val));
    return true;
}
bool ContextEnvironment::importStr(const char* key, char* &val)
{
    if (!key || !strs.Exists(key)) return false;
    val = (char *)(strs.Get(key).c_str());
    return true;
}
bool ContextEnvironment::exportInt(const char* key, int64_t val)
{
    if (!key) return false;
    if (ints.Exists(key)) ints.Delete(key);
    ints.Insert(key, val);
    return true;
}
bool ContextEnvironment::importInt(const char* key, int64_t &val)
{
    if (!key || !ints.Exists(key)) return false;
    val = ints.Get(key);
    return true;
}
bool ContextEnvironment::exportDat(const char* key, time_t val)
{
    if (!key) return false;
    if (dats.Exists(key)) dats.Delete(key);
    dats.Insert(key, val);
    return true;
}
bool ContextEnvironment::importDat(const char* key, time_t &val)
{
    if (!key || !dats.Exists(key)) return false;
    val = dats.Get(key);
    return true;
}
bool ContextEnvironment::exportFlt(const char* key, float val)
{
    if (!key) return false;
    if (flts.Exists(key)) flts.Delete(key);
    flts.Insert(key, val);
    return true;
}
bool ContextEnvironment::importFlt(const char* key, float &val)
{
    if (!key || !flts.Exists(key)) return false;
    val = flts.Get(key);
    return true;
}
bool ContextEnvironment::exportDbl(const char* key, double val)
{
    if (!key) return false;
    if (dbls.Exists(key)) dbls.Delete(key);
    dbls.Insert(key, val);
    return true;
}
bool ContextEnvironment::importDbl(const char* key, double &val)
{
    if (!key || !dbls.Exists(key)) return false;
    val = dbls.Get(key);
    return true;
}
bool ContextEnvironment::exportLdl(const char* key, long double val)
{
    if (!key) return false;
    if (ldls.Exists(key)) ldls.Delete(key);
    ldls.Insert(key, val);
    return true;
}
bool ContextEnvironment::importLdl(const char* key, long double &val)
{
    if (!key || !ldls.Exists(key)) return false;
    val = ldls.Get(key);
    return true;
}

void ContextEnvironment::reset()
{
    strs.Empty(); ints.Empty(); dats.Empty();
    flts.Empty(); dbls.Empty(); ldls.Empty();
}

}}}


