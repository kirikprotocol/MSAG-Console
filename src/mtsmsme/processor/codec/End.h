#ifndef _End_H_
#define _End_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asn_application.h>

#include <DestTransactionID.h>
#include <DialoguePortion.h>
#include <ComponentPortion.h>
#include <constr_SEQUENCE.h>


struct DialoguePortion; /* Forward declaration */

struct ComponentPortion;  /* Forward declaration */
extern asn_TYPE_descriptor_t asn_DEF_End;


typedef struct End {
  DestTransactionID_t  dtid;
  struct DialoguePortion  *dialoguePortion  /* OPTIONAL */;
  struct ComponentPortion *components /* OPTIONAL */;

  /* Context for parsing across buffer boundaries */
  asn_struct_ctx_t _asn_ctx;
} End_t;


#ifdef __cplusplus
}
#endif

#endif  /* _End_H_ */
