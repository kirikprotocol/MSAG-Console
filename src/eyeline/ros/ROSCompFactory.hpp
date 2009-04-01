/* ************************************************************************* *
 * ROS Operations Components factory(ARGUMENT, RESULT, ERRORS).
 * ************************************************************************* */
#ifndef __ROS_COMPONENTS_FACTORY_HPP
#ident "@(#)$Id$"
#define __ROS_COMPONENTS_FACTORY_HPP

#include <stdarg.h>

#include "logger/Logger.h"
using smsc::logger::Logger;

#include "util/Factory.hpp"
#include "eyeline/ros/ROSComponent.hpp"

namespace eyeline {
namespace ros {

//Remote Operation Factory
typedef smsc::util::FactoryXArg_T<unsigned, ROSComponentPrimitive, Logger> ROSFactory;
typedef ROSFactory::ProducerITF ROSCompProducer;


//Remote Operations Components factory(ARGUMENT, RESULT, ERRORS)
class ROSComponentsFactory {
protected:
  enum FabId {
      rosArg = 0, rosRes = 1, rosErr = 2
  };
  typedef std::vector<unsigned> OperationErrors;
  typedef std::map<unsigned, OperationErrors*> ROSErrors;

  const EncodedOID &  _acOID;
  ROSFactory          _plant[3];
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

  const EncodedOID & acOID(void) const { return _acOID; }

  //calling rule: registerArg(opcode1, new ROSFactory::ProducerT<ArgType1>());
  bool registerArg(unsigned opcode, ROSCompProducer * alloc)
  {
    return _plant[rosArg].registerProduct(opcode, alloc);
  }
  bool registerRes(unsigned opcode, ROSCompProducer * alloc)
  {
    return _plant[rosRes].registerProduct(opcode, alloc);
  }
  bool registerErr(unsigned errcode, ROSCompProducer * alloc)
  {
    return _plant[rosErr].registerProduct(errcode, alloc);
  }
  //binds error codes to OPERATION
  void bindErrors(unsigned opcode, unsigned errNum, ...)
  {
    std::auto_ptr<OperationErrors> verr(new OperationErrors);
    va_list  errs;
    va_start(errs, errNum);
    for (unsigned i = 0; i < errNum; ++i) {
      unsigned curErr = va_arg(errs, unsigned);
      verr->insert(verr->begin(), curErr);
    }
    va_end(errs);
    errMap.insert(ROSErrors::value_type(opcode, verr.get()));
    verr.release();
  }

  ROSComponentPrimitive * createArg(unsigned opcode, Logger * use_log = NULL) const
  {
    return _plant[rosArg].create(opcode, use_log); 
  }
  ROSComponentPrimitive * createRes(unsigned opcode, Logger * use_log = NULL) const
  {
    return _plant[rosRes].create(opcode, use_log);
  }
  ROSComponentPrimitive * createErr(unsigned errcode, Logger * use_log = NULL) const
  {
    return _plant[rosErr].create(errcode, use_log);
  }


  //returns TRUE if OPERATION has RESULT defined
  bool hasResult(unsigned opcode) const
  {
    return _plant[rosRes].getProducer(opcode) ? true : false;
  }
  //returns number of ERRORS defined for OPERATION
  unsigned hasErrors(unsigned opcode) const
  {
    ROSErrors::const_iterator it = errMap.find(opcode);
    return (it == errMap.end()) ? 0 : (unsigned)(it->second->size());
  }
  //returns TRUE if ERROR identified by errcode is defined for OPERATION
  bool hasError(unsigned opcode, unsigned errcode) const
  {
    ROSErrors::const_iterator it = errMap.find(opcode);
    if (it != errMap.end()) {
      const OperationErrors * verr = it->second;
      for (unsigned i = 0; i < verr->size(); ++i) {
        if ((*verr)[i] == errcode)
          return true;
      }
    }
    return false;
  }
};

typedef ROSComponentsFactory * (*ROSFactoryProducer)(void);

} //ros
} //eyeline

#endif /* __ROS_COMPONENTS_FACTORY_HPP */

