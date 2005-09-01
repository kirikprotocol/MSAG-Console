#ifndef _MessageType_H_
#define _MessageType_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asn_application.h>

#include <Undirectional.h>
#include <Begin.h>
#include <End.h>
#include <Continue.h>
#include <Abort.h>
#include <constr_CHOICE.h>

typedef enum MessageType_PR {
  MessageType_PR_NOTHING, /* No components present */
  MessageType_PR_undirectional,
  MessageType_PR_begin,
  MessageType_PR_end,
  MessageType_PR_contiinue,
  MessageType_PR_abort
} MessageType_PR;
extern asn_TYPE_descriptor_t asn_DEF_MessageType;


typedef struct MessageType {
  MessageType_PR present;
  union {
    Undirectional_t  undirectional;
    Begin_t  begin;
    End_t  end;
    Continue_t   contiinue;
    Abort_t  abort;
  } choice;

  /* Context for parsing across buffer boundaries */
  asn_struct_ctx_t _asn_ctx;
} MessageType_t;


#ifdef __cplusplus
}
#endif

#endif  /* _MessageType_H_ */
