#ifndef _Code_H_
#define _Code_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asn_application.h>

#include <NativeInteger.h>
#include <OBJECT_IDENTIFIER.h>
#include <constr_CHOICE.h>

typedef enum Code_PR {
  Code_PR_NOTHING,  /* No components present */
  Code_PR_local,
  Code_PR_global
} Code_PR;
extern asn_TYPE_descriptor_t asn_DEF_Code;


typedef struct Code {
  Code_PR present;
  union {
    int  local;
    OBJECT_IDENTIFIER_t  global;
  } choice;

  /* Context for parsing across buffer boundaries */
  asn_struct_ctx_t _asn_ctx;
} Code_t;


#ifdef __cplusplus
}
#endif

#endif  /* _Code_H_ */
