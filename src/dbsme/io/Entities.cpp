
#include <ctype.h>

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

        printf("Result %s=%s\n", name.c_str(), value.c_str());
        options.Insert(name.c_str(), value);
        
        while (isspace(line[curPos])) // skip spaces after 'value'
            curPos++;
    }
}
    
FormatEntity::FormatEntity(std::string line, bool type)
    throw(FormatRenderingException)
{
    if (type)
    {
        const char* raw = line.c_str();
        int i;
        for (i=0; i<entityTypesNumber-1; i++)
        {
            //printf("Checking type: %s\n", entityTypeStrings[i]);
            if (raw == strstr(raw, entityTypeStrings[i]))
            {
                this->type = (EntityType)i; str = "";
                break;
            }
        }
        if (i >= entityTypesNumber-1 ||
            !isspace(raw[strlen(entityTypeStrings[i])]))
        {
            throw FormatRenderingException("FormatRenderingException: "
                                           "Argument type is not supported. "
                                           "Parsing format string: <%s>", raw);
        }
        renderOptions(raw+strlen(entityTypeStrings[i]));
        
        const char* arg = getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_NUMBER);
        if (!arg) 
            throw FormatRenderingException("Argument number not defined !"
                                           "Parsing format string: <%s>", raw);
        position = atoi(arg);
    }
    else 
    {
        str = line; this->type = TEXT; position = 0;
    }
}
FormatEntity::~FormatEntity()
{
}

FormatEntityRenderer::FormatEntityRenderer(const char* format, bool text)
    throw(FormatRenderingException)
{
    printf("Format for parsing is: '%s'\n", format);
    
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
                printf("%s line: <%s>\n", (opened) ? "Inner":"Outer", line);

                if (opened || text)
                {
                    try
                    {
                        (void)entities.Push(new FormatEntity(str, opened));
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
    printf("%s line: <%s>\n", (opened) ? "Inner":"Outer", line);

    if (opened || text)
    {
        try
        {
            (void)entities.Push(new FormatEntity(str, opened));
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


