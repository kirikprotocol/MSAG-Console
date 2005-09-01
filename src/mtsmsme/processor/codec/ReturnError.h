#ifndef _ReturnError_H_
#define _ReturnError_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asn_application.h>

#include <InvokeIdType.h>
#include <Error.h>
#include <ANY.h>
#include <constr_SEQUENCE.h>

extern asn_TYPE_descriptor_t asn_DEF_ReturnError;


typedef struct ReturnError {
  InvokeIdType_t   invokeID;
  Error_t  errorCode;
  ANY_t *parameter  /* OPTIONAL */;

  /* Context for parsing across buffer boundaries */
  asn_struct_ctx_t _asn_ctx;
} ReturnError_t;


#ifdef __cplusplus
}
#endif

#endif  /* _ReturnError_H_ */
