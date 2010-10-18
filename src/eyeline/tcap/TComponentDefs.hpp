/* ************************************************************************** *
 * Classes implementing structured TCAP dialogue requests.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_TCOMPONENTDEFS_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif /* __GNUC__ */
#define __EYELINE_TCAP_TCOMPONENTDEFS_HPP__

#include <list>
#include "eyeline/ros/ROSPdu.hpp"


namespace eyeline {
namespace tcap {

using eyeline::ros::InvokeId;
using eyeline::ros::LocalOpCode;

typedef std::list<ros::ROSPdu *> TComponentsPtrList;
typedef std::list<ros::ROSPdu> TComponentsList;

}}

#endif /* __EYELINE_TCAP_TCOMPONENTDEFS_HPP__ */

