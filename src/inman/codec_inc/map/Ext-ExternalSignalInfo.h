#ifndef	_Ext_ExternalSignalInfo_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_Ext_ExternalSignalInfo_H_


#include <asn_application.h>

/* Including external dependencies */
#include <Ext-ProtocolId.h>
#include <SignalInfo.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* Ext-ExternalSignalInfo */
typedef struct Ext_ExternalSignalInfo {
	Ext_ProtocolId_t	 ext_ProtocolId;
	SignalInfo_t	 signalInfo;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Ext_ExternalSignalInfo_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Ext_ExternalSignalInfo;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _Ext_ExternalSignalInfo_H_ */
