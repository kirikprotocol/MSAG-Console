#ifndef SMSC_WSME_ADMIN
#define SMSC_WSME_ADMIN

/**
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <util/debug.h>

#include "WSmeExceptions.h"

namespace smsc { namespace wsme
{
    struct WSmeAdmin
    {
        WSmeAdmin() {};

        // TODO: Add all administrative methods here !!!
        
        virtual void addVisitor(const std::string msisdn)
            throw (ProcessException) = 0;
        virtual void removeVisitor(const std::string msisdn)
            throw (ProcessException) = 0;
        
        virtual void addLang(const std::string mask, std::string lang)
            throw (ProcessException) = 0;
        virtual void removeLang(const std::string mask)
            throw (ProcessException) = 0;
       
        virtual void addAd(int id, const std::string lang, std::string ad)
            throw (ProcessException) = 0;
        virtual void removeAd(int id, const std::string lang)
            throw (ProcessException) = 0;
        
    protected:

        virtual ~WSmeAdmin() {};
    };
        
}}

#endif // SMSC_WSME_ADMIN


