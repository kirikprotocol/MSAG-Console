
#include "DBSmeComponent.h"

namespace smsc { namespace dbsme 
{

static const char* ARGUMENT_NAME_ID         = "id";

DBSmeComponent::DBSmeComponent(DBSmeAdmin& admin)
    : logger(Logger::getCategory("smsc.dbsme.DBSmeComponent")), admin(admin)
{
    Parameters empty_params;
    Method restart((unsigned)restartMethod, "restart", empty_params, StringType);
    
    Parameters id_params;
    id_params[ARGUMENT_NAME_ID] = Parameter(ARGUMENT_NAME_ID, StringType);
    Method add_job   ((unsigned)addJobMethod,    "addJob",    id_params, StringType);
    Method remove_job((unsigned)removeJobMethod, "removeJob", id_params, StringType);
    Method change_job((unsigned)changeJobMethod, "changeJob", id_params, StringType);

    methods[restart.getName()]    = restart;
    methods[add_job.getName()]    = add_job;
    methods[remove_job.getName()] = remove_job;
    methods[change_job.getName()] = change_job;
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
        case restartMethod:
            admin.restart(); 
            break;
        case addJobMethod:
            addJob(args);
            break;
        case removeJobMethod:
            removeJob(args);
            break;
        case changeJobMethod:
            changeJob(args);
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

void DBSmeComponent::addJob(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_NAME_ID)) 
        error("addJob", ARGUMENT_NAME_ID);
    Variant arg = args[ARGUMENT_NAME_ID];
    const char* id = (arg.getType() == StringType) ? arg.getStringValue():0;
    if (!id || id[0] == '\0')
        error("addJob", ARGUMENT_NAME_ID);
    
    try { admin.addJob(id);
    } catch (Exception& exc) {
        throw AdminException("Failed to add job '%s'. Cause: %s", id, exc.what());
    } catch (std::exception& exc) {
        throw AdminException("Failed to add job '%s'. Cause: %s", id, exc.what());
    } catch (...) {
        throw AdminException("Failed to add job '%s'. Cause is unknown", id);
    }
}
void DBSmeComponent::removeJob(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_NAME_ID)) 
        error("removeJob", ARGUMENT_NAME_ID);
    Variant arg = args[ARGUMENT_NAME_ID];
    const char* id = (arg.getType() == StringType) ? arg.getStringValue():0;
    if (!id || id[0] == '\0')
        error("removeJob", ARGUMENT_NAME_ID);
    
    try { admin.removeJob(id);
    } catch (Exception& exc) {
        throw AdminException("Failed to remove job '%s'. Cause: %s", id, exc.what());
    } catch (std::exception& exc) {
        throw AdminException("Failed to remove job '%s'. Cause: %s", id, exc.what());
    } catch (...) {
        throw AdminException("Failed to remove job '%s'. Cause is unknown", id);
    }
}
void DBSmeComponent::changeJob(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_NAME_ID)) 
        error("changeJob", ARGUMENT_NAME_ID);
    Variant arg = args[ARGUMENT_NAME_ID];
    const char* id = (arg.getType() == StringType) ? arg.getStringValue():0;
    if (!id || id[0] == '\0')
        error("changeJob", ARGUMENT_NAME_ID);
    
    try { admin.changeJob(id);
    } catch (Exception& exc) {
        throw AdminException("Failed to change job '%s'. Cause: %s", id, exc.what());
    } catch (std::exception& exc) {
        throw AdminException("Failed to change job '%s'. Cause: %s", id, exc.what());
    } catch (...) {
        throw AdminException("Failed to change job '%s'. Cause is unknown", id);
    }
}


}}

