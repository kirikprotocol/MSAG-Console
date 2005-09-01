#ifndef _Error_H_
#define _Error_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asn_application.h>

#include <NativeInteger.h>
#include <OBJECT_IDENTIFIER.h>
#include <constr_CHOICE.h>

typedef enum Error_PR {
  Error_PR_NOTHING, /* No components present */
  Error_PR_local,
  Error_PR_global
} Error_PR;
extern asn_TYPE_descriptor_t asn_DEF_Error;


typedef struct Error {
  Error_PR present;
  union {
    int  local;
    OBJECT_IDENTIFIER_t  global;
  } choice;

  /* Context for parsing across buffer boundaries */
  asn_struct_ctx_t _asn_ctx;
} Error_t;


#ifdef __cplusplus
}
#endif

#endif  /* _Error_H_ */
