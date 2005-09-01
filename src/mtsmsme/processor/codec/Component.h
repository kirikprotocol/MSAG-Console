#ifndef _Component_H_
#define _Component_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asn_application.h>

#include <Invoke.h>
#include <ReturnResult.h>
#include <ReturnError.h>
#include <Reject.h>
#include <constr_CHOICE.h>

typedef enum Component_PR {
  Component_PR_NOTHING, /* No components present */
  Component_PR_invoke,
  Component_PR_returnResultLast,
  Component_PR_returnError,
  Component_PR_reject,
  Component_PR_returnResultNotLast
} Component_PR;
extern asn_TYPE_descriptor_t asn_DEF_Component;


typedef struct Component {
  Component_PR present;
  union {
    Invoke_t   invoke;
    ReturnResult_t   returnResultLast;
    ReturnError_t  returnError;
    Reject_t   reject;
    ReturnResult_t   returnResultNotLast;
  } choice;

  /* Context for parsing across buffer boundaries */
  asn_struct_ctx_t _asn_ctx;
} Component_t;


#ifdef __cplusplus
}
#endif

#endif  /* _Component_H_ */
