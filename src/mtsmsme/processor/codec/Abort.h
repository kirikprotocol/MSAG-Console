#ifndef _Abort_H_
#define _Abort_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asn_application.h>

#include <DestTransactionID.h>
#include <P-AbortCause.h>
#include <DialoguePortion.h>
#include <constr_CHOICE.h>
#include <constr_SEQUENCE.h>

typedef enum reason_PR {
  reason_PR_NOTHING,  /* No components present */
  reason_PR_p_abortCause,
  reason_PR_dialoguePortion
} reason_PR;
extern asn_TYPE_descriptor_t asn_DEF_Abort;


typedef struct Abort {
  DestTransactionID_t  dtid;
  struct reason {
    reason_PR present;
    union {
      P_AbortCause_t   p_abortCause;
      DialoguePortion_t  dialoguePortion;
    } choice;

    /* Context for parsing across buffer boundaries */
    asn_struct_ctx_t _asn_ctx;
  } *reason;

  /* Context for parsing across buffer boundaries */
  asn_struct_ctx_t _asn_ctx;
} Abort_t;


#ifdef __cplusplus
}
#endif

#endif  /* _Abort_H_ */
