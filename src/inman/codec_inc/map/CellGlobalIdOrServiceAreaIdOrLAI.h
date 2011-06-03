#ifndef	_CellGlobalIdOrServiceAreaIdOrLAI_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_CellGlobalIdOrServiceAreaIdOrLAI_H_


#include <asn_application.h>

/* Including external dependencies */
#include <CellGlobalIdOrServiceAreaIdFixedLength.h>
#include <LAIFixedLength.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum CellGlobalIdOrServiceAreaIdOrLAI_PR {
	CellGlobalIdOrServiceAreaIdOrLAI_PR_NOTHING,	/* No components present */
	CellGlobalIdOrServiceAreaIdOrLAI_PR_cellGlobalIdOrServiceAreaIdFixedLength,
	CellGlobalIdOrServiceAreaIdOrLAI_PR_laiFixedLength
} CellGlobalIdOrServiceAreaIdOrLAI_PR;

/* CellGlobalIdOrServiceAreaIdOrLAI */
typedef struct CellGlobalIdOrServiceAreaIdOrLAI {
	CellGlobalIdOrServiceAreaIdOrLAI_PR present;
	union CellGlobalIdOrServiceAreaIdOrLAI_u {
		CellGlobalIdOrServiceAreaIdFixedLength_t	 cellGlobalIdOrServiceAreaIdFixedLength;
		LAIFixedLength_t	 laiFixedLength;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CellGlobalIdOrServiceAreaIdOrLAI_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CellGlobalIdOrServiceAreaIdOrLAI;

#ifdef __cplusplus
}
#endif

#endif	/* _CellGlobalIdOrServiceAreaIdOrLAI_H_ */
