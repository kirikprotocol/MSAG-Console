#pragma ident "$Id$"
/* ************************************************************************* *
 * ROS Operations Components factory(ARGUMENT, RESULT, ERRORS).
 * ************************************************************************* */
#ifndef __INMAN_COMP_ROSFACTORY_HPP
#define __INMAN_COMP_ROSFACTORY_HPP

#include <stdarg.h>

#include "logger/Logger.h"
using smsc::logger::Logger;

#include "util/Factory.hpp"
#include "inman/comp/compdefs.hpp"
#include "inman/comp/EncodedOID.hpp"

namespace smsc {
namespace inman {
namespace comp {

typedef smsc::util::FactoryXArg_T<unsigned, Component, Logger> CompFactory;
typedef CompFactory::ProducerITF CompProducer;

//ROS Operations Components factory(ARGUMENT, RESULT, ERRORS)
class ROSComponentsFactory {
protected:
    enum FabId {
        rosArg = 0, rosRes = 1, rosErr = 2
    };
    typedef std::vector<unsigned> OperationErrors;
    typedef std::map<unsigned, OperationErrors*> ROSErrors;

    const EncodedOID &  _acOID;
    CompFactory         _plant[3];
    ROSErrors           errMap;

public:
    //NOTE: ac_oid should be an extern or static object.
    ROSComponentsFactory(const EncodedOID & ac_oid)
        : _acOID(ac_oid)
    { }
    ~ROSComponentsFactory()
    {
        for (ROSErrors::iterator it = errMap.begin(); it != errMap.end(); ++it)
            delete it->second;
        for (unsigned i = 0; i < 3; ++i)
            _plant[i].eraseAll();
    }

    inline const EncodedOID & acOID(void) const { return _acOID; }

    //calling rule: registerArg(opcode1, new CompFactory::ProducerT<ArgType1>());
    inline bool registerArg(unsigned opcode, CompProducer* alloc)
    {
        return _plant[rosArg].registerProduct(opcode, alloc);
    }
    inline bool registerRes(unsigned opcode, CompProducer* alloc)
    {
        return _plant[rosRes].registerProduct(opcode, alloc);
    }
    inline bool registerErr(unsigned errcode, CompProducer* alloc)
    {
        return _plant[rosErr].registerProduct(errcode, alloc);
    }
    //binds error codes to OPERATION
    void bindErrors(unsigned opcode, unsigned errNum, ...)
    {
        OperationErrors * verr = new OperationErrors;
        va_list  errs;
        va_start(errs, errNum);
        for (unsigned i = 0; i < errNum; ++i) {
            unsigned curErr = va_arg(errs, unsigned);
            verr->insert(verr->begin(), curErr);
        }
        va_end(errs);
        errMap.insert(ROSErrors::value_type(opcode, verr));
    }

    inline Component * createArg(unsigned opcode, Logger * use_log = NULL) const
    {
        return _plant[rosArg].create(opcode, use_log); 
    }
    inline Component * createRes(unsigned opcode, Logger * use_log = NULL) const
    {
        return _plant[rosRes].create(opcode, use_log);
    }
    inline Component * createErr(unsigned errcode, Logger * use_log = NULL) const
    {
        return _plant[rosErr].create(errcode, use_log);
    }


    //returns TRUE if OPERATION has RESULT defined
    inline bool hasResult(unsigned opcode) const
    {
        return _plant[rosRes].getProducer(opcode) ? true : false;
    }
    //returns number of ERRORS defined for OPERATION
    inline unsigned hasErrors(unsigned opcode) const
    {
        ROSErrors::const_iterator it = errMap.find(opcode);
        return (it == errMap.end()) ? 0 : (unsigned)(it->second->size());
    }
    //returns TRUE if ERROR identified by errcode is defined for OPERATION
    bool hasError(unsigned opcode, unsigned errcode) const
    {
        ROSErrors::const_iterator it = errMap.find(opcode);
        if (it != errMap.end()) {
            const OperationErrors *verr = it->second;
            for (unsigned i = 0; i < verr->size(); ++i)
                if ((*verr)[i] == errcode)
                    return true;
        }
        return false;
    }
};

typedef ROSComponentsFactory * (*ROSFactoryProducer)(void);

} //comp
} //inman
} //smsc

#endif /* __INMAN_COMP_ROSFACTORY_HPP */

