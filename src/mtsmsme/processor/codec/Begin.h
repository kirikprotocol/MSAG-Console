#ifndef _Begin_H_
#define _Begin_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asn_application.h>

#include <OrigTransactionID.h>
#include <DialoguePortion.h>
#include <ComponentPortion.h>
#include <constr_SEQUENCE.h>


struct DialoguePortion; /* Forward declaration */

struct ComponentPortion;  /* Forward declaration */
extern asn_TYPE_descriptor_t asn_DEF_Begin;


typedef struct Begin {
  OrigTransactionID_t  otid;
  struct DialoguePortion  *dialoguePortion  /* OPTIONAL */;
  struct ComponentPortion *components /* OPTIONAL */;

  /* Context for parsing across buffer boundaries */
  asn_struct_ctx_t _asn_ctx;
} Begin_t;


#ifdef __cplusplus
}
#endif

#endif  /* _Begin_H_ */
