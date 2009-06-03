#ifndef __EYELINE_TCAP_TCOMPONENTDEFS_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_TCOMPONENTDEFS_HPP__

# include <sys/types.h>

namespace eyeline {
namespace tcap {

typedef uint8_t InvokeId;

typedef uint8_t problem_code_t;

enum problem_code_e { BAD_COMPONENT_PORTION = 1, GENERAL_PROBLEM = 255 };

}}

#endif
