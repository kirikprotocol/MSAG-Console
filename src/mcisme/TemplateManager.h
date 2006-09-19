
#ifndef SMSC_MCI_SME_TEMPLATE_MANAGER
#define SMSC_MCI_SME_TEMPLATE_MANAGER

#include <logger/Logger.h>

#include <util/config/ConfigView.h>
#include <util/config/ConfigException.h>

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/synchronization/EventMonitor.hpp>

#include <core/buffers/Array.hpp>
#include <core/buffers/Hash.hpp>
#include <core/buffers/IntHash.hpp>

#include "Templates.h"

namespace smsc { namespace mcisme
{

    using namespace smsc::core::synchronization;
    using namespace smsc::core::buffers;

    using smsc::logger::Logger;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;
    
    class TemplateManager
    {
    private:

        int32_t defaultInformTemplateId, defaultNotifyTemplateId;
        IntHash<InformTemplateFormatter *> informTemplates;
        IntHash<NotifyTemplateFormatter *> notifyTemplates;

    public:
        
        TemplateManager(ConfigView* config);
        ~TemplateManager();

        // if id<0 || formatter not exists => returns default formatter
        InformTemplateFormatter* getInformFormatter(int32_t id);
        NotifyTemplateFormatter* getNotifyFormatter(int32_t id);
    };

}}

#endif // SMSC_MCI_SME_TEMPLATE_MANAGER
