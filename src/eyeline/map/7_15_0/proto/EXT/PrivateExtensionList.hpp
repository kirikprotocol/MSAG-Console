#ifndef __EYELINE_MAP_7F0_PROTO_EXT_PRIVATEEXTENSIONLIST_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_EXT_PRIVATEEXTENSIONLIST_HPP__

# include <list>
# include "eyeline/map/7_15_0/proto/EXT/PrivateExtension.hpp"

namespace eyeline {
namespace map {
namespace ext {

/*
  PrivateExtensionList ::= SEQUENCE SIZE (1..maxNumOfPrivateExtensions) OF
                                PrivateExtension
 */
class PrivateExtensionList : public std::list<PrivateExtension> {
public:
  bool validate() const;
};

}}}

#endif
