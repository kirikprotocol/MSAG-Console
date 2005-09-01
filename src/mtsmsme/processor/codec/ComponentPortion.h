#ifndef _ComponentPortion_H_
#define _ComponentPortion_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asn_application.h>

#include <Component.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

extern asn_TYPE_descriptor_t asn_DEF_ComponentPortion;


typedef struct ComponentPortion {
  A_SEQUENCE_OF(Component_t) list;

  /* Context for parsing across buffer boundaries */
  asn_struct_ctx_t _asn_ctx;
} ComponentPortion_t;


#ifdef __cplusplus
}
#endif

#endif  /* _ComponentPortion_H_ */
