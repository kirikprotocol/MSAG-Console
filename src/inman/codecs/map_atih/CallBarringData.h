#ident "$Id$"

#ifndef	_CallBarringData_H_
#define	_CallBarringData_H_


#include <asn_application.h>

/* Including external dependencies */
#include <Ext-CallBarFeatureList.h>
#include <Password.h>
#include <WrongPasswordAttemptsCounter.h>
#include <NULL.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* CallBarringData */
typedef struct CallBarringData {
	Ext_CallBarFeatureList_t	 callBarringFeatureList;
	Password_t	*password	/* OPTIONAL */;
	WrongPasswordAttemptsCounter_t	*wrongPasswordAttemptsCounter	/* OPTIONAL */;
	NULL_t	*notificationToCSE	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CallBarringData_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CallBarringData;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _CallBarringData_H_ */
