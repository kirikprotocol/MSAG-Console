#ifndef	_MiscCallInfo_H_
#define	_MiscCallInfo_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum messageType {
	messageType_request	= 0,
	messageType_notification	= 1
	/*
	 * Enumeration is extensible
	 */
} messageType_e;

/* MiscCallInfo */
typedef struct MiscCallInfo {
	long	 messageType;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} MiscCallInfo_t;

/* Implementation */
/* extern asn_TYPE_descriptor_t asn_DEF_messageType_2;	// (Use -fall-defs-global to expose) */
extern asn_TYPE_descriptor_t asn_DEF_MiscCallInfo;

#ifdef __cplusplus
}
#endif

#endif	/* _MiscCallInfo_H_ */
