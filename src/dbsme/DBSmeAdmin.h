#ifndef SMSC_DBSME_ADMIN
#define SMSC_DBSME_ADMIN

/**
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see 
 */

namespace smsc { namespace dbsme
{
    struct DBSmeAdmin
    {
        virtual void restart() = 0;
        
        virtual void addJob(std::string jobId) = 0;
        virtual void removeJob(std::string jobId) = 0; 
        virtual void changeJob(std::string jobId) = 0;

	    virtual ~DBSmeAdmin() {};

    protected:

	    DBSmeAdmin() {};
    };
        
}}

#endif // SMSC_DBSME_ADMIN


