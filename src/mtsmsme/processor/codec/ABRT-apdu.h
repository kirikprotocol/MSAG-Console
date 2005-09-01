#ifndef _ABRT_apdu_H_
#define _ABRT_apdu_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asn_application.h>

#include <ABRT-source.h>
#include <MEXT.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

extern asn_TYPE_descriptor_t asn_DEF_ABRT_apdu;


typedef struct ABRT_apdu {
  ABRT_source_t  abort_source;
  struct abrt_user_information {
    A_SEQUENCE_OF(MEXT_t) list;

    /* Context for parsing across buffer boundaries */
    asn_struct_ctx_t _asn_ctx;
  } *abrt_user_information;

  /* Context for parsing across buffer boundaries */
  asn_struct_ctx_t _asn_ctx;
} ABRT_apdu_t;


#ifdef __cplusplus
}
#endif

#endif  /* _ABRT_apdu_H_ */
