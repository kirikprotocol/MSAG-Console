/* ************************************************************************* *
 * Global Rerturn Codes Registry.
 * Provides transformation from pair { retrunCodeSpace,errcode} to uint32_t
 * hash and vice versa. 
 * ************************************************************************* */
#ifndef _SMSC_RETCODES_REGISTRY_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define _SMSC_RETCODES_REGISTRY_HPP

#include <inttypes.h>
#include <map>
#include <vector>
#include <string>
#include <assert.h>

#include "util/vformat.hpp"

namespace smsc {
namespace util {

typedef unsigned    RCSpaceIdx;
typedef uint32_t    RCHash;
typedef const char * (*FPRCDescriptor)(uint32_t ret_code);

extern const char * DFLTRCDescriptor(uint32_t ret_code);

class URCSpaceITF {
protected:
  virtual ~URCSpaceITF();

public:
  virtual RCSpaceIdx   Idx(void) const  = 0;
  virtual const char * Ident(void) const = 0;

  virtual RCHash mkhash(uint32_t ret_code) const = 0;
  virtual bool ownhash(RCHash rc_hash) const = 0;
  virtual uint32_t /*ret_code*/ unhash(RCHash rc_hash) const = 0;
  virtual const char * code2Txt(uint32_t ret_code) const = 0;
  virtual const char * hash2Txt(RCHash rc_hash) const = 0;
  virtual std::string explainCode(uint32_t ret_code) const = 0;
  virtual std::string explainHash(RCHash rc_hash) const = 0;
};

class URCRegistry { //singleton
public:
    static URCRegistry* get(void);  //Gets URCRegistry instance

    static const RCHash   _hashUndefined = (RCHash)(-1);
    static const uint32_t _errUndefined = (uint32_t)(-1);
    /* RESERVED ErrorSpaces: */
    static const char * _errSpaceUndefined(void)  { return "errUndefined"; }
    static const char * _errSpaceOk(void)         { return "errOk"; }

protected:
    typedef std::vector<uint32_t> RCSet;
     //Return Codes space (either set or continuous range)
    class URCSpace : public URCSpaceITF {
    protected:
        friend class URCRegistry;
        
        uint32_t    rcMin;
        uint32_t    rcMax;
        FPRCDescriptor fpDescr;
        /* */
        RCSpaceIdx  idx;
        uint32_t    base;
        uint32_t    range;
        const std::string * name;
        RCSet       rcSet;

        URCSpace(uint32_t min_code = 0, uint32_t max_code = 0,
                FPRCDescriptor fp_descr = DFLTRCDescriptor, const RCSet * use_set = NULL)
            : rcMin(min_code), rcMax(max_code), fpDescr(fp_descr)
            , idx(0), base(0), range(rcMax - rcMin + 1), name(0)
        { 
            if (use_set)
                rcSet = *use_set;
        }

    public:
        // -- URCSpaceITF methods implementation
        RCSpaceIdx   Idx(void) const { return idx; }
        const char * Ident(void) const { return name->c_str(); }

        RCHash mkhash(uint32_t ret_code) const
        {
            if (!rcSet.empty()) {
                RCSet::const_iterator it = rcSet.begin();
                for (uint32_t i = 0; it != rcSet.end(); i++, it++) {
                    if (rcSet[i] == ret_code)
                        return base + (i - rcMin);
                }
                return URCRegistry::_hashUndefined;
            }
            if ((ret_code < rcMin) || (ret_code > rcMax))
                return URCRegistry::_hashUndefined;
            return  base + (ret_code - rcMin);
        }
        bool ownhash(RCHash rc_hash) const
        {
            return ((rc_hash >= base) && (rc_hash < (base + range))) ? true : false;
        }
        uint32_t /*ret_code*/ unhash(RCHash rc_hash) const
        {
            if ((rc_hash >= base) && (rc_hash < (base + range))) {
                uint32_t rc = rc_hash + rcMin - base;
                return rcSet.empty() ? rc : rcSet[rc];
            }
            return URCRegistry::_errUndefined;
        }

        const char * code2Txt(uint32_t ret_code) const
        {
            return fpDescr(ret_code);
        }
        const char * hash2Txt(RCHash rc_hash) const
        {
            uint32_t ret_code = unhash(rc_hash);
            return fpDescr(ret_code);
        }

        //format sample: "errOk::23 (undefined error)"
        std::string explainCode(uint32_t ret_code) const
        {
            return format("%s::%d (%s)", Ident(), ret_code, fpDescr(ret_code));
        }
        std::string explainHash(RCHash rc_hash) const
        {
            uint32_t ret_code = unhash(rc_hash);
            return format("%s::%d (%s)", Ident(), ret_code, fpDescr(ret_code));
        }

        //public destructor for std::map (std::pair)
        virtual ~URCSpace()
        { }
    };

    typedef std::map<RCSpaceIdx, URCSpace> RValsDb;
    typedef std::map<std::string, RValsDb::iterator> NamesDb;

    RValsDb     rvalDb;
    NamesDb     spaces;
    unsigned    lastId;

    void adjustRanges(void);
    const URCSpace * insertSpace(const std::string & name, const URCSpace & space);
    const URCSpace * lookUp(RCHash rc_hash) const;

    URCRegistry() : lastId(0)
    {   //insert sample space in order to reserve hash value 0
        insertSpace(std::string(_errSpaceOk()), URCSpace(0, 0));
    }
    ~URCRegistry()
    { }
    void* operator new(size_t);

public:
    //Initializes error space as continuous range of return codes
    //returns NULL if error space is already defined.
    const URCSpaceITF * initRCSpace(const char * name, uint32_t err_min,
                            uint32_t err_max, FPRCDescriptor fp_descr);

    //Initializes error space as set of return codes
    //returns NULL if error space is already defined.
    const URCSpaceITF * initRCSpace(const char * name, FPRCDescriptor fp_descr,
                                    uint32_t rc_num, uint32_t rc0, ... );

    //returns NULL if there is no defined error space owning given hash value.
    static const URCSpaceITF * getRCSpace(RCHash rc_hash);
    //explains the RCHash value - associated error space, return code, description
    static std::string explainHash(RCHash rc_hash);
};

//Initializes the RC space and provides access to its methods
//(as std::auto_ptr does)
//It's helpfull in static initialization of RC spaces.
class URCSpacePTR {
protected:
    const URCSpaceITF *    rcSp;

public:
    URCSpacePTR(const char * name, uint32_t err_min, 
                uint32_t err_max, FPRCDescriptor fp_descr)
    {
        assert(name && (err_max >= err_min));
        assert((rcSp = URCRegistry::get()->initRCSpace(name, err_min, err_max, fp_descr)));
    }
    ~URCSpacePTR()
    { }

    const URCSpaceITF & operator*() const { return *rcSp; }
    const URCSpaceITF * operator->() const { return  rcSp; }
    const URCSpaceITF * get() const { return  rcSp; }
};

} //util
} //smsc
#endif /* _SMSC_RETCODES_REGISTRY_HPP */

