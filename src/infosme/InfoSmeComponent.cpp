
#include "InfoSmeComponent.h"

namespace smsc { namespace infosme 
{

using namespace smsc::core::buffers;

InfoSmeComponent::InfoSmeComponent(InfoSmeAdmin& admin)
    : logger(Logger::getCategory("smsc.infosme.InfoSmeComponent")), admin(admin)
{
}

InfoSmeComponent::~InfoSmeComponent()
{
}

Variant InfoSmeComponent::call(const Method& method, const Arguments& args)
    throw (AdminException)
{
    try 
    {
        logger.debug("call \"%s\"", method.getName());
        
        switch (method.getId())
        {
        case addTaskMethod:
            logger.debug("addTaskMethod processing...");
            logger.debug("addTaskMethod processed.");
            break;
        case removeTaskMethod:
            logger.debug("removeTaskMethod processing...");
            logger.debug("removeTaskMethod processed.");
            break;

        default:
            logger.debug("unknown method \"%s\" [%u]", method.getName(), method.getId());
            throw AdminException("Unknown method \"%s\"", method.getName());
        }

        return Variant("");
    }
    catch (AdminException &e) {
        logger.debug("AdminException: %s", e.what());
        throw e;
    }
    catch (...) {
        logger.debug("... Exception");
        throw AdminException("Unknown exception catched during call");
    }
}

void InfoSmeComponent::error(const char* method, const char* param)
{
    Exception exc("Parameter '%s' missed or invalid "
                  "for InfoSmeComponent::%s()", param, method);
    logger.error(exc.what());
    throw exc;
}

}}

