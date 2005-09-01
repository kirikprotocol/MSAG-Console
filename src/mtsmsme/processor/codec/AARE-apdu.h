#ifndef _AARE_apdu_H_
#define _AARE_apdu_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asn_application.h>

#include <BIT_STRING.h>
#include <OBJECT_IDENTIFIER.h>
#include <Associate-result.h>
#include <Associate-source-diagnostic.h>
#include <MEXT.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

extern asn_TYPE_descriptor_t asn_DEF_AARE_apdu;


typedef struct AARE_apdu {
  BIT_STRING_t  *protocol_version /* DEFAULT { version1} */;
  OBJECT_IDENTIFIER_t  application_context_name;
  Associate_result_t   result;
  Associate_source_diagnostic_t  result_source_diagnostic;
  struct aare_user_information {
    A_SEQUENCE_OF(MEXT_t) list;

    /* Context for parsing across buffer boundaries */
    asn_struct_ctx_t _asn_ctx;
  } *aare_user_information;

  /* Context for parsing across buffer boundaries */
  asn_struct_ctx_t _asn_ctx;
} AARE_apdu_t;


#ifdef __cplusplus
}
#endif

#endif  /* _AARE_apdu_H_ */
