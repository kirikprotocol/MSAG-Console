#ifndef _AARQ_apdu_H_
#define _AARQ_apdu_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asn_application.h>

#include <BIT_STRING.h>
#include <OBJECT_IDENTIFIER.h>
#include <MEXT.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

extern asn_TYPE_descriptor_t asn_DEF_AARQ_apdu;


typedef struct AARQ_apdu {
  BIT_STRING_t  *protocol_version /* DEFAULT { version1} */;
  OBJECT_IDENTIFIER_t  application_context_name;
  struct aarq_user_information {
    A_SEQUENCE_OF(MEXT_t) list;

    /* Context for parsing across buffer boundaries */
    asn_struct_ctx_t _asn_ctx;
  } *aarq_user_information;

  /* Context for parsing across buffer boundaries */
  asn_struct_ctx_t _asn_ctx;
} AARQ_apdu_t;


#ifdef __cplusplus
}
#endif

#endif  /* _AARQ_apdu_H_ */
