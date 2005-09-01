#ifndef _Invoke_H_
#define _Invoke_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asn_application.h>

#include <InvokeIdType.h>
#include <Code.h>
#include <ANY.h>
#include <constr_SEQUENCE.h>

extern asn_TYPE_descriptor_t asn_DEF_Invoke;


typedef struct Invoke {
  InvokeIdType_t   invokeID;
  InvokeIdType_t  *linkedID /* OPTIONAL */;
  Code_t   opcode;
  ANY_t *argument /* OPTIONAL */;

  /* Context for parsing across buffer boundaries */
  asn_struct_ctx_t _asn_ctx;
} Invoke_t;


#ifdef __cplusplus
}
#endif

#endif  /* _Invoke_H_ */
