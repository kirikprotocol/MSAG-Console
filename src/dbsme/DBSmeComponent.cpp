
#include "DBSmeComponent.h"

namespace smsc { namespace dbsme 
{

static const char* ARGUMENT_JOB_NAME_ID      = "jobId";
static const char* ARGUMENT_PROVIDER_NAME_ID = "providerId";

DBSmeComponent::DBSmeComponent(DBSmeAdmin& admin)
    : logger(Logger::getCategory("smsc.dbsme.DBSmeComponent")), admin(admin)
{
    Parameters empty_params;
    Method restart((unsigned)restartMethod, "restart", empty_params, StringType);
    
    Parameters job_params;
    job_params[ARGUMENT_PROVIDER_NAME_ID] = Parameter(ARGUMENT_PROVIDER_NAME_ID, StringType);
    job_params[ARGUMENT_JOB_NAME_ID]      = Parameter(ARGUMENT_JOB_NAME_ID,      StringType);
    
    Method add_job   ((unsigned)addJobMethod,    "addJob",    job_params, StringType);
    Method remove_job((unsigned)removeJobMethod, "removeJob", job_params, StringType);
    Method change_job((unsigned)changeJobMethod, "changeJob", job_params, StringType);

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

static const char* DBSME_ADD_FAILED_MESSAGE =
"Failed to add job '%s' in provider '%s'. Cause: %s";
void DBSmeComponent::addJob(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_PROVIDER_NAME_ID)) error("addJob", ARGUMENT_PROVIDER_NAME_ID);
    if (!args.Exists(ARGUMENT_JOB_NAME_ID)) error("addJob", ARGUMENT_JOB_NAME_ID);
    
    Variant arg1 = args[ARGUMENT_PROVIDER_NAME_ID];
    const char* providerId = (arg1.getType() == StringType) ? arg1.getStringValue():0;
    if (!providerId || providerId[0] == '\0') error("addJob", ARGUMENT_PROVIDER_NAME_ID);
    
    Variant arg2 = args[ARGUMENT_JOB_NAME_ID];
    const char* jobId = (arg2.getType() == StringType) ? arg2.getStringValue():0;
    if (!jobId || jobId[0] == '\0') error("addJob", ARGUMENT_JOB_NAME_ID);
    
    try { admin.addJob(providerId, jobId);
    } catch (Exception& exc) {
        throw AdminException(DBSME_ADD_FAILED_MESSAGE, jobId, providerId, exc.what());
    } catch (std::exception& exc) {
        throw AdminException(DBSME_ADD_FAILED_MESSAGE, jobId, providerId, exc.what());
    } catch (...) {
        throw AdminException(DBSME_ADD_FAILED_MESSAGE, jobId, providerId, "unknown");
    }
}

static const char* DBSME_REMOVE_FAILED_MESSAGE =
"Failed to remove job '%s' in provider '%s'. Cause: %s";
void DBSmeComponent::removeJob(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_PROVIDER_NAME_ID)) error("removeJob", ARGUMENT_PROVIDER_NAME_ID);
    if (!args.Exists(ARGUMENT_JOB_NAME_ID)) error("removeJob", ARGUMENT_JOB_NAME_ID);
    
    Variant arg1 = args[ARGUMENT_PROVIDER_NAME_ID];
    const char* providerId = (arg1.getType() == StringType) ? arg1.getStringValue():0;
    if (!providerId || providerId[0] == '\0') error("removeJob", ARGUMENT_PROVIDER_NAME_ID);
    
    Variant arg2 = args[ARGUMENT_JOB_NAME_ID];
    const char* jobId = (arg2.getType() == StringType) ? arg2.getStringValue():0;
    if (!jobId || jobId[0] == '\0') error("removeJob", ARGUMENT_JOB_NAME_ID);
    
    try { admin.removeJob(providerId, jobId);
    } catch (Exception& exc) {
        throw AdminException(DBSME_REMOVE_FAILED_MESSAGE, jobId, providerId, exc.what());
    } catch (std::exception& exc) {
        throw AdminException(DBSME_REMOVE_FAILED_MESSAGE, jobId, providerId, exc.what());
    } catch (...) {
        throw AdminException(DBSME_REMOVE_FAILED_MESSAGE, jobId, providerId, "unknown");
    }
}

static const char* DBSME_CHANGE_FAILED_MESSAGE =
"Failed to change job '%s' in provider '%s'. Cause: %s";
void DBSmeComponent::changeJob(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_PROVIDER_NAME_ID)) error("changeJob", ARGUMENT_PROVIDER_NAME_ID);
    if (!args.Exists(ARGUMENT_JOB_NAME_ID)) error("changeJob", ARGUMENT_JOB_NAME_ID);
    
    Variant arg1 = args[ARGUMENT_PROVIDER_NAME_ID];
    const char* providerId = (arg1.getType() == StringType) ? arg1.getStringValue():0;
    if (!providerId || providerId[0] == '\0') error("changeJob", ARGUMENT_PROVIDER_NAME_ID);
    
    Variant arg2 = args[ARGUMENT_JOB_NAME_ID];
    const char* jobId = (arg2.getType() == StringType) ? arg2.getStringValue():0;
    if (!jobId || jobId[0] == '\0') error("changeJob", ARGUMENT_JOB_NAME_ID);
    
    try { admin.changeJob(providerId, jobId);
    } catch (Exception& exc) {
        throw AdminException(DBSME_CHANGE_FAILED_MESSAGE, jobId, providerId, exc.what());
    } catch (std::exception& exc) {
        throw AdminException(DBSME_CHANGE_FAILED_MESSAGE, jobId, providerId, exc.what());
    } catch (...) {
        throw AdminException(DBSME_CHANGE_FAILED_MESSAGE, jobId, providerId, "unknown");
    }
}


}}

