/* ************************************************************************* *
 * ASN.1 OCTET STRING type definitions.
 * ************************************************************************* */
#ifndef __ABSTRACT_SYNTAX_OCTSTR_DEFS__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ABSTRACT_SYNTAX_OCTSTR_DEFS__

#include "core/buffers/LWArrayT.hpp"
#include "core/buffers/LWArrayTraitsInt.hpp"

namespace eyeline {
namespace asn1 {

/* ------------------------------------------------------------- *
 * Template for OCTET STRING with maximum number of octets = 255.
 * ------------------------------------------------------------- */
typedef smsc::core::buffers::LWArrayExtension_T<
  uint8_t, uint8_t, smsc::core::buffers::LWArrayTraitsPOD_T
> OCTArrayTiny;

static const uint8_t _OCTSTR_TINY_DFLT_SZ = 32;

template <uint8_t _SZ_TArg = _OCTSTR_TINY_DFLT_SZ>
class OCTSTR_TINY_T : public
  smsc::core::buffers::LWArray_T<uint8_t, uint8_t, _SZ_TArg, smsc::core::buffers::LWArrayTraitsPOD_T> {
public:
  typedef smsc::core::buffers::LWArrayExtension_T<
    uint8_t, uint8_t, smsc::core::buffers::LWArrayTraitsPOD_T
  > ArrayType;
  typedef smsc::core::buffers::LWArrayExtension_T<
    uint8_t, uint8_t, smsc::core::buffers::LWArrayTraitsPOD_T
  >::size_type size_type;

  explicit OCTSTR_TINY_T(uint8_t num_to_reserve = 0)
    : smsc::core::buffers::LWArray_T<
        uint8_t, uint8_t, _SZ_TArg, smsc::core::buffers::LWArrayTraitsPOD_T
      >(num_to_reserve)
  { }
  explicit OCTSTR_TINY_T(const uint8_t * use_buf, uint8_t num_elem)
    : smsc::core::buffers::LWArray_T<
        uint8_t, uint8_t, _SZ_TArg, smsc::core::buffers::LWArrayTraitsPOD_T
      >(use_buf, num_elem)
  { }
  OCTSTR_TINY_T(const OCTSTR_TINY_T & use_arr)
    : smsc::core::buffers::LWArray_T<
        uint8_t, uint8_t, _SZ_TArg, smsc::core::buffers::LWArrayTraitsPOD_T
      >(use_arr)
  { }
  ~OCTSTR_TINY_T()
  { }
};
typedef OCTSTR_TINY_T<_OCTSTR_TINY_DFLT_SZ> OCTSTR_TINY;


/* ------------------------------------------------------------- *
 * Template for OCTET STRING with maximum number of octets = 64k.
 * ------------------------------------------------------------- */
typedef smsc::core::buffers::LWArrayExtension_T<
  uint8_t, uint16_t, smsc::core::buffers::LWArrayTraitsPOD_T
> OCTArray64K;

static const uint16_t _OCTSTR_DFLT_SZ = 255;

template <uint16_t _SZ_TArg = _OCTSTR_DFLT_SZ>
class OCTSTR_T : public 
  smsc::core::buffers::LWArray_T<uint8_t, uint16_t, _SZ_TArg, smsc::core::buffers::LWArrayTraitsPOD_T> {
public:
  typedef smsc::core::buffers::LWArrayExtension_T<
    uint8_t, uint16_t, smsc::core::buffers::LWArrayTraitsPOD_T
  > ArrayType;
  typedef smsc::core::buffers::LWArrayExtension_T<
    uint8_t, uint16_t, smsc::core::buffers::LWArrayTraitsPOD_T
  >::size_type size_type;

  explicit OCTSTR_T(uint16_t num_to_reserve = 0)
    : smsc::core::buffers::LWArray_T<
        uint8_t, uint16_t, _SZ_TArg, smsc::core::buffers::LWArrayTraitsPOD_T
      >(num_to_reserve)
  { }
  explicit OCTSTR_T(const uint16_t * use_buf, uint8_t num_elem)
    : smsc::core::buffers::LWArray_T<
        uint8_t, uint16_t, _SZ_TArg, smsc::core::buffers::LWArrayTraitsPOD_T
      >(use_buf, num_elem)
  { }
  OCTSTR_T(const OCTSTR_T & use_arr)
    : smsc::core::buffers::LWArray_T<
        uint8_t, uint16_t, _SZ_TArg, smsc::core::buffers::LWArrayTraitsPOD_T
      >(use_arr)
  { }
  ~OCTSTR_T()
  { }
};
typedef OCTSTR_T<_OCTSTR_DFLT_SZ> OCTSTR;

} //asn1
} //eyeline

#endif /* __ABSTRACT_SYNTAX_OCTSTR_DEFS__ */

