#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */
/* ************************************************************************* *
 * Global Rerturn Codes Registry.
 * Provides transformation from pair { retrunCodeSpace,errcode} to uint32_t
 * hash and vice versa. 
 * ************************************************************************* */
#include <memory>
#include "util/URCdb/URCRegistry.hpp"

namespace smsc {
namespace util {

const char * DFLTRCDescriptor(uint32_t ret_code)
{ return !ret_code ? "Ok" : "no description"; }


/* ************************************************************************** *
 * class URCRegistry implementation:
 * ************************************************************************** */
URCRegistry* URCRegistry::get(void)
{
    static URCRegistry instance;
    return &instance;
}

//returns NULL if error space is already defined.
const URCSpaceITF * URCRegistry::initRCSpace(const char * name, uint32_t err_min,
                        uint32_t err_max, FPRCDescriptor fp_descr)
{
    std::string spaceNm(name);
    NamesDb::iterator nit = spaces.find(spaceNm);
    if (nit != spaces.end())
        return NULL;
    return insertSpace(spaceNm, URCSpace(err_min, err_max, fp_descr));
}

//Initializes error space as set of return codes
//returns NULL if error space is already defined.
const URCSpaceITF * URCRegistry::initRCSpace(const char * name, FPRCDescriptor fp_descr,
                                uint32_t rc_num, uint32_t rc0, ... )
{
    std::string spaceNm(name);
    NamesDb::iterator nit = spaces.find(spaceNm);
    if (nit != spaces.end())
        return NULL;

    std::auto_ptr<RCSet> rcSet(new RCSet(rc_num));
    va_list  rcs;
    va_start(rcs, rc_num);
    for (uint32_t i = 0; i < rc_num; i++) {
        uint32_t curRc = va_arg(rcs, uint32_t);
        (*rcSet)[i] = curRc;
    }
    va_end(rcs);
    return insertSpace(spaceNm, URCSpace(0, rc_num - 1, fp_descr, rcSet.release()));
}

//returns NULL if there is no defined error space owning given hash value.
const URCSpaceITF * URCRegistry::getRCSpace(RCHash rc_hash)
{
    return URCRegistry::get()->lookUp(rc_hash);
}

std::string URCRegistry::explainHash(RCHash rc_hash)
{
    const URCSpaceITF * rcs = getRCSpace(rc_hash);
    return rcs ? rcs->explainHash(rc_hash) : _errSpaceUndefined();
}

/* ---------------------------------------------------------------------------------- *
 * Protected/Private methods:
 * ---------------------------------------------------------------------------------- */
void URCRegistry::adjustRanges(void)
{
    RValsDb::iterator it = rvalDb.begin();
    for (uint32_t nextBase = 0; it != rvalDb.end(); it++) {
        (it->second).base = nextBase;
        nextBase += (it->second).range;
    }
}

const URCRegistry::URCSpace * URCRegistry::insertSpace(const std::string & name, const URCSpace & space)
{
    RValsDb::iterator it =
        rvalDb.insert(RValsDb::value_type(++lastId, space)).first;

    NamesDb::iterator nit = 
        spaces.insert(NamesDb::value_type(name, it)).first;
    (it->second).name = &(nit->first);
    (it->second).idx = lastId;
    adjustRanges();
    return &(it->second);
}

const URCRegistry::URCSpace * URCRegistry::lookUp(RCHash rc_hash) const
{
    for (RValsDb::const_iterator it = rvalDb.begin(); it != rvalDb.end(); it++) {
        if ((it->second).ownhash(rc_hash))
            return &(it->second);
    }
    return NULL;
}

} //util
} //smsc


