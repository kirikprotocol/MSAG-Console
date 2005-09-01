#ifndef _Undirectional_H_
#define _Undirectional_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asn_application.h>

#include <DialoguePortion.h>
#include <ComponentPortion.h>
#include <constr_SEQUENCE.h>


struct DialoguePortion; /* Forward declaration */
extern asn_TYPE_descriptor_t asn_DEF_Undirectional;


typedef struct Undirectional {
  struct DialoguePortion  *dialoguePortion  /* OPTIONAL */;
  ComponentPortion_t   components;

  /* Context for parsing across buffer boundaries */
  asn_struct_ctx_t _asn_ctx;
} Undirectional_t;


#ifdef __cplusplus
}
#endif

#endif  /* _Undirectional_H_ */
