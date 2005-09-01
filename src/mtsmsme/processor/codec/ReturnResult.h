#ifndef _ReturnResult_H_
#define _ReturnResult_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asn_application.h>

#include <InvokeIdType.h>
#include <NULL.h>
#include <ANY.h>
#include <constr_SEQUENCE.h>


struct Code;  /* Forward declaration */
extern asn_TYPE_descriptor_t asn_DEF_ReturnResult;


typedef struct ReturnResult {
  InvokeIdType_t   invokeID;
  struct result {
    struct Code *operationCode  /* OPTIONAL */;
    ANY_t  parameter;

    /* Context for parsing across buffer boundaries */
    asn_struct_ctx_t _asn_ctx;
  } *result;

  /* Context for parsing across buffer boundaries */
  asn_struct_ctx_t _asn_ctx;
} ReturnResult_t;


#ifdef __cplusplus
}
#endif

#endif  /* _ReturnResult_H_ */
