
#include <ctype.h>
#include <util/debug.h>

#include "Entities.h"

namespace smsc { namespace dbsme { namespace io
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
        
        while (isspace(line[curPos])) // skip spaces before 'name'
            curPos++;
        while (isalnum(line[curPos])) 
            name += line[curPos++];
        if (!name.length())
        {
            throw FormatRenderingException("FormatRenderingException: "
                "Argument name expected. Parsing line: <%s>", line);
        }
        while (isspace(line[curPos])) // skip spaces after 'name'
            curPos++;
        if (line[curPos++] != SMSC_DBSME_IO_FORMAT_OPTION_ASSIGN)
            throw FormatRenderingException("FormatRenderingException: "
                "'%c' sign expected. Parsing line: <%s>", 
                SMSC_DBSME_IO_FORMAT_OPTION_ASSIGN, line);
        while (isspace(line[curPos])) // skip spaces before 'value'
            curPos++;
        
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
        
        while (isspace(line[curPos])) // skip spaces after 'value'
            curPos++;
    }
}

// io == false --> parser, io == true --> formatter
FormatEntity::FormatEntity(std::string line, bool io, bool type)
    throw(FormatRenderingException)
{
    if (type)
    {
        const char* raw = line.c_str();
        int i;
        for (i=0; i<ioEntityTypesNumber-1; i++)
        {
            //__trace2__("Checking type: %s", ioEntityTypeStrings[i]);
            if (raw == strstr(raw, ioEntityTypeStrings[i]))
            {
                this->type = (EntityType)i; str = "";
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
        
        if (io) // formatter
        {
            if (!arg && !def)
                throw FormatRenderingException(
                    "Neither '%s' nor '%s' option defined ! "
                    "Parsing format string: <%s>", 
                    SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION,
                    SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION, raw);
        }
        else    // parser
        {
            if (!arg) 
                throw FormatRenderingException(
                    "'%s' option wasn't defined ! "
                    "Parsing format string: <%s>", 
                    SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION, raw);
        }
    }
    else 
    {
        str = line; this->type = TEXT;
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
    bool    escape = false;
    bool    opened = false;
    bool    string = false;
    
    while (format && format[curPos] != '\0')
    {
        switch(format[curPos])
        {
        case SMSC_DBSME_IO_FORMAT_ENTITY_ESCAPER:
            if (!escape && format[curPos+1] != '\0')
            {
                if (!string &&
                    format[curPos+1] != SMSC_DBSME_IO_FORMAT_STRING_DELIMETER)
                {
                    escape = true; curPos++;
                    continue;
                }
                str += format[curPos++];
            }
            break;
        case SMSC_DBSME_IO_FORMAT_STRING_DELIMETER:
            if (!escape)
            {
                string = !string;
            }
            break;
        case SMSC_DBSME_IO_FORMAT_ENTITY_DELIMETER:
            if (!escape && !string &&
                format[curPos+1] == SMSC_DBSME_IO_FORMAT_ENTITY_DELIMETER) 
            {
                curPos += 2;
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
                
                opened = !opened; str = "";
                continue;
            } 
            break;
        default:
            break;
        }
        
        escape = false;
        str += format[curPos++];
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

}}}


