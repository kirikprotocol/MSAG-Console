
#include "DBSmeComponent.h"

namespace smsc { namespace dbsme 
{

DBSmeComponent::DBSmeComponent(DBSmeAdmin& admin)
    : logger(Logger::getCategory("smsc.dbsme.DBSmeComponent")), admin(admin)
{
    Parameters empty_params;
    Method apply((unsigned)applyChangesMethod, "apply", empty_params, StringType);

    methods[apply.getName()] = apply;
}

DBSmeComponent::~DBSmeComponent()
{
}

Variant DBSmeComponent::call(const Method& method, const Arguments& args)
    throw (AdminException)
{
    try 
    {
        logger.debug("call \"%s\"", method.getName());
        
        switch (method.getId())
        {
        case applyChangesMethod:
            admin.applyChanges(); 
            break;
        
        default:
            logger.debug("unknown method \"%s\" [%u]", method.getName(), method.getId());
            throw AdminException("Unknown method \"%s\"", method.getName());
        }

        return Variant("");
    }
    catch (AdminException &e) {
        logger.error("AdminException: %s", e.what());
        throw e;
    }
    catch (std::exception& exc) {
        logger.error("Exception: %s", exc.what());
        throw AdminException("%s", exc.what());
    }
    catch (...) {
        logger.error("... Exception");
        throw AdminException("Unknown exception was caught during call");
    }
}

void DBSmeComponent::error(const char* method, const char* param)
{
    throw Exception("Parameter '%s' missed or invalid for DBSmeComponent::%s()",
                    param, method);
}

}}

