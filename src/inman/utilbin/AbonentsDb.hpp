/* ************************************************************************** *
 * Helpers: AbonentInfo registry.
 * ************************************************************************** */
#ifndef __INMAN_ABONENTS_DB_HPP__
#ident "@(#)$Id$"
#define __INMAN_ABONENTS_DB_HPP__

#include "util/TonNpiAddress.hpp"

namespace smsc  {
namespace inman {

using smsc::util::MAPConst;
using smsc::util::TonNpiAddress;

struct AbonentInfo {
    TonNpiAddress  addr;
    char           imsi[MAPConst::MAX_IMSI_AddressValueLength];
    GsmSCFinfo     scf;

    AbonentInfo() { imsi[0] = 0; }
};

struct AbonentISDN {
    const char *  addr;
    const char *  imsi;

    AbonentISDN(const char * use_addr = NULL, const char *use_imsi = NULL)
        : addr(use_addr), imsi(use_imsi)
    { }
};

class KnownAbonents {
private:
    std::vector<AbonentISDN> _known;

protected:
    //forbid destruction and instantiation
    void* operator new(size_t);

    KnownAbonents()
    {
        //Nezhinsky phone(prepaid):
        _known.push_back(AbonentISDN(".1.1.79139343290", "250013900405871"));
        //Ryzhkov phone(postpaid):
        _known.push_back(AbonentISDN(".1.1.79139859489", "250013901464251"));
        //Stupnik phone(postpaid):
        _known.push_back(AbonentISDN(".1.1.79139033669", "250013901464251"));
    }
    ~KnownAbonents()
    { }

public:
    static KnownAbonents & getInstance(void)
    {
        static KnownAbonents instance;
        return instance;
    }

    unsigned size(void) const { return (unsigned)_known.size(); }
    const AbonentISDN * get(void) const { return &(_known[0]); }
};


class AbonentsDb {
protected:
    typedef std::map<unsigned, AbonentInfo> AbonentsMAP;

    mutable Mutex   _sync;
    AbonentsMAP     registry;
    unsigned        lastAbnId;

public:
    static void printAbnInfo(FILE * stream, const AbonentInfo & abn, unsigned ab_id)
    {
        fprintf(stream, "abn.%u: isdn <%s>, imsi <%s>\n\tSCF <%s>, servKey %u\n",
                ab_id, abn.addr.length ? abn.addr.toString().c_str() : " ",
                abn.imsi[0] ? abn.imsi : "none",
                abn.scf.scfAddress.length ? abn.scf.scfAddress.getSignals() : "none",
                abn.scf.serviceKey);
    }

    AbonentsDb(void) : lastAbnId(0)
    { }
    ~AbonentsDb()
    { }

    unsigned Init(unsigned n_abn, const AbonentISDN * p_abn)
    {
        for (unsigned i = 0; i < n_abn; ++i) {
            AbonentInfo  abn;
            if (p_abn[i].addr)
                abn.addr.fromText(p_abn[i].addr);
            else
                abn.addr.clear();
            if (p_abn[i].imsi[0])
                strcpy(abn.imsi, p_abn[i].imsi);
            else
                abn.imsi[0] = 0;

            registry.insert(AbonentsMAP::value_type(++lastAbnId, abn));
        }
        return (unsigned)registry.size();
    }

    unsigned Init(void)
    {
        return Init(KnownAbonents::getInstance().size(),
                    KnownAbonents::getInstance().get());
    }
    unsigned getMaxAbId(void) const { return lastAbnId; }

    unsigned addAbonent(TonNpiAddress * p_isdn, char * p_imsi)
    {
        MutexGuard  grd(_sync);
        AbonentInfo  abn;
        if (p_isdn && p_isdn->length)
            abn.addr = *p_isdn;
        else
            abn.addr.clear();
        if (p_imsi && p_imsi[0])
            strcpy(abn.imsi, p_imsi);
        else
            abn.imsi[0] = 0;

        registry.insert(AbonentsMAP::value_type(++lastAbnId, abn));
        return lastAbnId;
    }

    const AbonentInfo * getAbnInfo(unsigned ab_id) const
    {
        MutexGuard  grd(_sync);
        AbonentsMAP::const_iterator it = registry.find(ab_id);
        return (it != registry.end()) ? &((*it).second) : NULL;
    }

    bool setAbnInfo(unsigned ab_id, const GsmSCFinfo * p_scf = NULL, const char * p_imsi = NULL)
    {
        MutexGuard  grd(_sync);
        AbonentsMAP::iterator it = registry.find(ab_id);
        if (it != registry.end()) {
            AbonentInfo & abn = it->second;
            if (p_scf)
                abn.scf = *p_scf;
            else {
                abn.scf.serviceKey = 0;
                abn.scf.scfAddress.clear();
            }
                
            if (p_imsi && p_imsi[0])
                strcpy(abn.imsi, p_imsi);
            return true;
        }
        return false;
    }

    unsigned lookUp(const TonNpiAddress & subscr) const
    {
        MutexGuard  grd(_sync);
        for (AbonentsMAP::const_iterator it = registry.begin();
                                    it != registry.end(); ++it) {
            const AbonentInfo & abn = it->second;
            if (abn.addr.length && (abn.addr == subscr))
                return it->first;
        }
        return 0; //unknown
    }
    unsigned lookUp(const std::string & addr) const
    {
        TonNpiAddress   subscr;
        if (!subscr.fromText(addr.c_str()))
            return 0;
        return lookUp(subscr);
    }

    void printAbnInfo(FILE * stream, unsigned ab_id) const
    {
        MutexGuard  grd(_sync);
        AbonentsMAP::const_iterator it = registry.find(ab_id);
        if (it != registry.end()) {
            const AbonentInfo & abn = it->second;
            AbonentsDb::printAbnInfo(stream, abn, ab_id);
        }
    }

    //0, 0 - print ALL
    void printAbnInfo(FILE * stream, unsigned min_id, unsigned max_id) const
    {
        MutexGuard  grd(_sync);
        if (!registry.size())
            return;
        if (!min_id || (min_id > registry.size()))
            min_id = 1;
        if (!max_id || (max_id > registry.size()))
            max_id = (unsigned)registry.size();

        AbonentsMAP::const_iterator it = registry.find(min_id);
        while (min_id <= max_id) {
            const AbonentInfo & abn = it->second;
            printAbnInfo(stream, abn, min_id);
            ++min_id; ++it;
        }
    }
};

} //inman
} //smsc
#endif /* __INMAN_ABONENTS_DB_HPP__ */

