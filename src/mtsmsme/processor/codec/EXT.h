#ifndef _EXT_H_
#define _EXT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asn_application.h>

#include <OBJECT_IDENTIFIER.h>
#include <NativeInteger.h>
#include <ObjectDescriptor.h>
#include <DialoguePDU.h>
#include <OCTET_STRING.h>
#include <BIT_STRING.h>
#include <constr_CHOICE.h>
#include <constr_SEQUENCE.h>

typedef enum encoding_PR {
  encoding_PR_NOTHING,  /* No components present */
  encoding_PR_single_ASN1_type,
  encoding_PR_octet_aligned,
  encoding_PR_arbitrary
} encoding_PR;
extern asn_TYPE_descriptor_t asn_DEF_EXT;


typedef struct EXT {
  OBJECT_IDENTIFIER_t *direct_reference /* OPTIONAL */;
  int *indirect_reference /* OPTIONAL */;
  ObjectDescriptor_t  *data_value_descriptor  /* OPTIONAL */;
  struct encoding {
    encoding_PR present;
    union {
      DialoguePDU_t  single_ASN1_type;
      OCTET_STRING_t   octet_aligned;
      BIT_STRING_t   arbitrary;
    } choice;

    /* Context for parsing across buffer boundaries */
    asn_struct_ctx_t _asn_ctx;
  } encoding;

  /* Context for parsing across buffer boundaries */
  asn_struct_ctx_t _asn_ctx;
} EXT_t;


#ifdef __cplusplus
}
#endif

#endif  /* _EXT_H_ */
