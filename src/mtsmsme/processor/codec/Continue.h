#ifndef _Continue_H_
#define _Continue_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asn_application.h>

#include <OrigTransactionID.h>
#include <DestTransactionID.h>
#include <DialoguePortion.h>
#include <ComponentPortion.h>
#include <constr_SEQUENCE.h>


struct DialoguePortion; /* Forward declaration */

struct ComponentPortion;  /* Forward declaration */
extern asn_TYPE_descriptor_t asn_DEF_Continue;


typedef struct Continue {
  OrigTransactionID_t  otid;
  DestTransactionID_t  dtid;
  struct DialoguePortion  *dialoguePortion  /* OPTIONAL */;
  struct ComponentPortion *componenets  /* OPTIONAL */;

  /* Context for parsing across buffer boundaries */
  asn_struct_ctx_t _asn_ctx;
} Continue_t;


#ifdef __cplusplus
}
#endif

#endif  /* _Continue_H_ */
