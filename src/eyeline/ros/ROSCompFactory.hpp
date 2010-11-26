/* ************************************************************************* *
 * ROS Operations Components factory(ARGUMENT, RESULT, ERRORS).
 * ************************************************************************* */
#ifndef __ROS_COMPONENTS_FACTORY_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ROS_COMPONENTS_FACTORY_HPP

#include <vector>

#include "util/Factory.hpp"
#include "eyeline/asn1/AbstractValue.hpp"
#include "eyeline/asn1/EncodedOID.hpp"

namespace eyeline {
namespace ros {

//Remote Operation Factory
typedef smsc::util::FactoryT<unsigned, asn1::ASTypeValueAC> ROSFactory;
typedef ROSFactory::Producer ROSCompProducer;


//Remote Operations Components factory(ARGUMENT, RESULT, ERRORS)
class ROSComponentsFactory {
protected:
  enum FabId_e { rosArg = 0, rosRes = 1, rosErr = 2 };
  typedef std::vector<unsigned> OperationErrors;
  typedef std::map<unsigned, OperationErrors*> ROSErrors;

  const asn1::EncodedOID &  _acOID;

  ROSFactory  _plant[3];
  ROSErrors   _errMap;

public:
  //NOTE: ac_oid should be an extern or static object.
  ROSComponentsFactory(const asn1::EncodedOID & ac_oid)
      : _acOID(ac_oid)
  { }
  ~ROSComponentsFactory()
  {
    for (ROSErrors::iterator it = _errMap.begin(); it != _errMap.end(); ++it)
      delete it->second;
    for (unsigned i = 0; i < 3; ++i)
      _plant[i].eraseAll();
  }

  const asn1::EncodedOID & acOID(void) const { return _acOID; }

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
  void bindErrors(unsigned opcode, unsigned errNum, /*errCode_1*/ ... /*errCode_N*/);

  asn1::ASTypeValueAC * createArg(unsigned opcode) const
  {
    return _plant[rosArg].create(opcode); 
  }
  asn1::ASTypeValueAC * createRes(unsigned opcode) const
  {
    return _plant[rosRes].create(opcode);
  }
  asn1::ASTypeValueAC * createErr(unsigned errcode) const
  {
    return _plant[rosErr].create(errcode);
  }


  //returns TRUE if OPERATION has RESULT defined
  bool hasResult(unsigned opcode) const
  {
    return _plant[rosRes].getProducer(opcode) ? true : false;
  }
  //returns number of ERRORS defined for OPERATION
  unsigned hasErrors(unsigned opcode) const
  {
    ROSErrors::const_iterator it = _errMap.find(opcode);
    return (it == _errMap.end()) ? 0 : (unsigned)(it->second->size());
  }
  //returns TRUE if ERROR identified by errcode is defined for OPERATION
  bool hasError(unsigned opcode, unsigned errcode) const;
};

//Function that creates ROSComponentsFactory
typedef ROSComponentsFactory * (*ROSCompFactoryProducingFunc)(void);

} //ros
} //eyeline

#endif /* __ROS_COMPONENTS_FACTORY_HPP */

