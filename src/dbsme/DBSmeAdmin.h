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
        virtual void applyChanges() = 0;
        
	    virtual ~DBSmeAdmin() {};

    protected:

	    DBSmeAdmin() {};
    };
        
}}

#endif // SMSC_DBSME_ADMIN


