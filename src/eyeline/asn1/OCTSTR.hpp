/* ************************************************************************* *
 * ASN.1 OCTET STRING type definitions.
 * ************************************************************************* */
#ifndef __ABSTRACT_SYNTAX_OCTSTR_DEFS__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ABSTRACT_SYNTAX_OCTSTR_DEFS__

#include "eyeline/util/LWArray.hpp"
#include "eyeline/util/LWArrayTraitsInt.hpp"

namespace eyeline {
namespace asn1 {

/* ------------------------------------------------------------- *
 * Template for OCTET STRING with maximum number of octets = 255.
 * ------------------------------------------------------------- */
typedef eyeline::util::LWArrayExtension_T<uint8_t, uint8_t> OCTArrayTiny;

static const uint8_t _OCTSTR_TINY_DFLT_SZ = 32;

template <uint8_t _SZ_TArg = _OCTSTR_TINY_DFLT_SZ>
class OCTSTR_TINY_T : public eyeline::util::LWArray_T<uint8_t, uint8_t, _SZ_TArg> {
public:
  typedef eyeline::util::LWArrayExtension_T<uint8_t, uint8_t> ArrayType;
  typedef eyeline::util::LWArrayExtension_T<uint8_t, uint8_t>::size_type size_type;

  explicit OCTSTR_TINY_T(uint8_t num_to_reserve = 0)
    : eyeline::util::LWArray_T<uint8_t, uint8_t, _SZ_TArg>(num_to_reserve)
  { }
  explicit OCTSTR_TINY_T(const uint8_t * use_buf, uint8_t num_elem)
    : eyeline::util::LWArray_T<uint8_t, uint8_t, _SZ_TArg>(use_buf, num_elem)
  { }
  OCTSTR_TINY_T(const OCTSTR_TINY_T & use_arr)
    : eyeline::util::LWArray_T<uint8_t, uint8_t, _SZ_TArg>(use_arr)
  { }
  ~OCTSTR_TINY_T()
  { }
};
typedef OCTSTR_TINY_T<_OCTSTR_TINY_DFLT_SZ> OCTSTR_TINY;


/* ------------------------------------------------------------- *
 * Template for OCTET STRING with maximum number of octets = 64k.
 * ------------------------------------------------------------- */
typedef eyeline::util::LWArrayExtension_T<uint8_t, uint16_t> OCTArray64K;

static const uint16_t _OCTSTR_DFLT_SZ = 255;

template <uint16_t _SZ_TArg = _OCTSTR_TINY_DFLT_SZ>
class OCTSTR_T : public eyeline::util::LWArray_T<uint8_t, uint16_t, _SZ_TArg> {
public:
  typedef eyeline::util::LWArrayExtension_T<uint8_t, uint16_t> ArrayType;
  typedef eyeline::util::LWArrayExtension_T<uint8_t, uint16_t>::size_type size_type;

  explicit OCTSTR_T(uint16_t num_to_reserve = 0)
    : eyeline::util::LWArray_T<uint8_t, uint16_t, _SZ_TArg>(num_to_reserve)
  { }
  explicit OCTSTR_T(const uint16_t * use_buf, uint8_t num_elem)
    : eyeline::util::LWArray_T<uint8_t, uint16_t, _SZ_TArg>(use_buf, num_elem)
  { }
  OCTSTR_T(const OCTSTR_T & use_arr)
    : eyeline::util::LWArray_T<uint8_t, uint16_t, _SZ_TArg>(use_arr)
  { }
  ~OCTSTR_T()
  { }
};
typedef OCTSTR_T<_OCTSTR_DFLT_SZ> OCTSTR;

} //asn1
} //eyeline

#endif /* __ABSTRACT_SYNTAX_OCTSTR_DEFS__ */

