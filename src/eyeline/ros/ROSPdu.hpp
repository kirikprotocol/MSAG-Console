/* ************************************************************************** *
 * Remote-Operations-Generic-ROS-PDUs according to 
 * joint-iso-itu-t(2) remote-operations(4) generic-ROS-PDUs(6) version1(0)
  * ************************************************************************** */
#ifndef __ROS_PDUS_DEFS_HPP
#ident "@(#)$Id$"
#define __ROS_PDUS_DEFS_HPP

#include "eyeline/util/ChoiceOfT.hpp"
#include "eyeline/ros/ROSPrimitives.hpp"

namespace eyeline {
namespace ros {

typedef eyeline::util::ChoiceOfBased5_T<
  ROSPduPrimitiveAC, ROSInvokePdu, ROSResultPdu, ROSErrorPdu, ROSRejectPdu, ROSResultNLPdu
> ChoiceOfROSPrimitives;

class ROSPdu : public ChoiceOfROSPrimitives {
public:
  ROSPdu() : ChoiceOfROSPrimitives()
  { }
  ~ROSPdu()
  { }


  ROSPduPrimitiveAC::Kind_e getKind(void) const
  {
    return get() ? get()->getKind() : ROSPduPrimitiveAC::rosCancel;
  }

  Alternative_T<ROSInvokePdu>      invoke()        { return alternative0(); }
  ConstAlternative_T<ROSInvokePdu> invoke() const  { return alternative0(); }

  Alternative_T<ROSResultPdu>      result()        { return alternative1(); }
  ConstAlternative_T<ROSResultPdu> result() const  { return alternative1(); }

  Alternative_T<ROSErrorPdu>       error()         { return alternative2(); }
  ConstAlternative_T<ROSErrorPdu>  error() const   { return alternative2(); }

  Alternative_T<ROSRejectPdu>      reject()        { return alternative3(); }
  ConstAlternative_T<ROSRejectPdu> reject() const  { return alternative3(); }

  Alternative_T<ROSResultNLPdu>      resultNL()        { return alternative4(); }
  ConstAlternative_T<ROSResultNLPdu> resultNL() const  { return alternative4(); }
};

} //ros
} //eyeline

#endif /* __ROS_PDUS_DEFS_HPP */

