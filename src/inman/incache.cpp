static const char ident[] = "$Id$";

#include "inman/incache.hpp"

namespace smsc {
namespace inman {
namespace cache {

/* ************************************************************************** *
 * class AbonentCache implementation:
 * ************************************************************************** */
AbonentCache::AbonentCache(AbonentCacheCFG * cfg, Logger * uselog/* = NULL*/)
    : _cfg(*cfg)
{
    logger = uselog ? uselog : Logger::getInstance("smsc.inman.InCache");
    if (_cfg.nmFile)
        load(_cfg.nmFile);
}

AbonentCache::~AbonentCache()
{
    if (_cfg.nmFile)
        save(_cfg.nmFile);
}


// ----------------------------------------
// AbonentCacheITF interface methods:
// ----------------------------------------
void AbonentCache::setAbonentInfo(AbonentId ab_number, AbonentBillType ab_type,
                                    time_t expired /*= 0*/)
{
    if (!expired)
        expired = time(NULL) + _cfg.interval;

    cacheGuard.Lock();
    AbonentRecord ab_rec(ab_type, expired);
    int status = cache.Insert(ab_number, ab_rec);
    cacheGuard.Unlock();
    smsc_log_debug(logger, "InCache: abonent %s is %s: %u",
                   ab_number, status ? "added" : "updated", ab_type);
}

AbonentBillType AbonentCache::getAbonentInfo(AbonentId ab_number)
{
    MutexGuard  guard(cacheGuard);
    AbonentRecord * ab_rec = cache.GetPtr(ab_number);
    if (ab_rec && (ab_rec->ab_type != btUnknown)) {
        if (time(NULL) < ab_rec->tm_expired)
            return ab_rec->ab_type;

        smsc_log_debug(logger, "InCache: abonent %s info is expired", ab_number);
        ab_rec->ab_type = btUnknown; //expired
    }
    return btUnknown;
}


} //cache
} //inman
} //smsc
