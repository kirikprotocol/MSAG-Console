#ifndef SMSC_MCI_SME_TEMPLATES
#define SMSC_MCI_SME_TEMPLATES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <string>

#include <util/templates/Formatters.h>

namespace smsc { namespace mcisme
{
    using namespace smsc::util::templates;

    static const uint8_t INFORM_TEMPLATE_FORMATTER = 1;
    static const uint8_t NOTIFY_TEMPLATE_FORMATTER = 2;

    class TemplateFormatter
    {
    protected:

        uint8_t             type;
        OutputFormatter*    messageFormatter;
        
        TemplateFormatter(const std::string& messageTemplate, uint8_t _type) 
            : type(_type), messageFormatter(0)
        {
            messageFormatter = new OutputFormatter(messageTemplate.c_str());
        };

    public:

        virtual ~TemplateFormatter()
        {
            if (messageFormatter) delete messageFormatter;
        };

        inline uint8_t getType() {
            return type;
        }
        inline OutputFormatter* getMessageFormatter() {
            return messageFormatter;
        };
    };
    
    struct NotifyTemplateFormatter : public TemplateFormatter
    {
        NotifyTemplateFormatter(const std::string& messageTemplate) 
            : TemplateFormatter(messageTemplate, NOTIFY_TEMPLATE_FORMATTER) {};
    };
    
    struct InformTemplateFormatter : public TemplateFormatter
    {
    protected:

        bool                group;
        std::string         unknownCaller;
        OutputFormatter*    multiFormatter;
        OutputFormatter*    singleFormatter;
        
    public:

        InformTemplateFormatter(const std::string& messageTemplate,
                                const std::string& _unknownCaller,
                                const std::string& singleTemplate) 
            : TemplateFormatter(messageTemplate, INFORM_TEMPLATE_FORMATTER), 
                group(false), unknownCaller(_unknownCaller), multiFormatter(0), singleFormatter(0)
        {
            singleFormatter = new OutputFormatter(singleTemplate.c_str());
        };
        InformTemplateFormatter(const std::string& messageTemplate,
                                const std::string& _unknownCaller,
                                const std::string& singleTemplate,
                                const std::string& multiTemplate) 
            : TemplateFormatter(messageTemplate, INFORM_TEMPLATE_FORMATTER), 
                group(true), unknownCaller(_unknownCaller), multiFormatter(0), singleFormatter(0)
        {
            multiFormatter = new OutputFormatter(multiTemplate.c_str());
            singleFormatter = new OutputFormatter(singleTemplate.c_str());
        };
        virtual ~InformTemplateFormatter()
        {
            if (singleFormatter) delete singleFormatter;
            if (multiFormatter) delete multiFormatter;
        };

        inline bool isGroupping() { 
            return group;
        };
        inline const std::string& getUnknownCaller() {
            return unknownCaller;
        }
        inline OutputFormatter* getMultiFormatter() {
            return multiFormatter;
        };
        inline OutputFormatter* getSingleFormatter() {
            return singleFormatter;
        };
    };

}}

#endif // SMSC_MCI_SME_TEMPLATES

