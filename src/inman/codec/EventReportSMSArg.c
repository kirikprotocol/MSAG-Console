#include <asn_internal.h>

#include "EventReportSMSArg.h"

static asn_TYPE_member_t asn_MBR_EventReportSMSArg_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct EventReportSMSArg, eventTypeSMS),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_EventTypeSMS,
		0,	/* Defer constraints checking to the member type */
		"eventTypeSMS"
		},
	{ ATF_POINTER, 3, offsetof(struct EventReportSMSArg, eventSpecificInformationSMS),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_EventSpecificInformationSMS,
		0,	/* Defer constraints checking to the member type */
		"eventSpecificInformationSMS"
		},
	{ ATF_POINTER, 2, offsetof(struct EventReportSMSArg, miscCallInfo),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_MiscCallInfo,
		0,	/* Defer constraints checking to the member type */
		"miscCallInfo"
		},
	{ ATF_POINTER, 1, offsetof(struct EventReportSMSArg, extensions),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Extensions,
		0,	/* Defer constraints checking to the member type */
		"extensions"
		},
};
static ber_tlv_tag_t asn_DEF_EventReportSMSArg_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_EventReportSMSArg_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* eventTypeSMS at 1104 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* eventSpecificInformationSMS at 1105 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* miscCallInfo at 1106 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 3, 0, 0 } /* extensions at 1107 */
};
static asn_SEQUENCE_specifics_t asn_SPC_EventReportSMSArg_1_specs = {
	sizeof(struct EventReportSMSArg),
	offsetof(struct EventReportSMSArg, _asn_ctx),
	asn_MAP_EventReportSMSArg_1_tag2el,
	4,	/* Count of tags in the map */
	3,	/* Start extensions */
	5	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_EventReportSMSArg = {
	"EventReportSMSArg",
	"EventReportSMSArg",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
#ifndef ASN1_XER_NOT_USED
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
#else  /* ASN1_XER_NOT_USED */
	0, 0,
#endif /* ASN1_XER_NOT_USED */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_EventReportSMSArg_1_tags,
	sizeof(asn_DEF_EventReportSMSArg_1_tags)
		/sizeof(asn_DEF_EventReportSMSArg_1_tags[0]), /* 1 */
	asn_DEF_EventReportSMSArg_1_tags,	/* Same as above */
	sizeof(asn_DEF_EventReportSMSArg_1_tags)
		/sizeof(asn_DEF_EventReportSMSArg_1_tags[0]), /* 1 */
	asn_MBR_EventReportSMSArg_1,
	4,	/* Elements count */
	&asn_SPC_EventReportSMSArg_1_specs	/* Additional specs */
};

