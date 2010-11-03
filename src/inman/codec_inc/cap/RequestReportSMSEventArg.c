#ident "$Id$"

#include <asn_internal.h>

#include "RequestReportSMSEventArg.h"

static int
memb_sMSEvents_1_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_consume_bytes_f *app_errlog, void *app_key) {
	size_t size;
	
	if(!sptr) {
		_ASN_ERRLOG(app_errlog, app_key,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	/* Determine the number of elements */
	size = _A_CSEQUENCE_FROM_VOID(sptr)->count;
	
	if((size >= 1 && size <= 10)) {
		/* Perform validation of the inner elements */
		return td->check_constraints(td, sptr, app_errlog, app_key);
	} else {
		_ASN_ERRLOG(app_errlog, app_key,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static asn_TYPE_member_t asn_MBR_sMSEvents_2[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_SMSEvent,
		0,	/* Defer constraints checking to the member type */
		""
		},
};
static ber_tlv_tag_t asn_DEF_sMSEvents_2_tags[] = {
	(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_sMSEvents_2_specs = {
	sizeof(struct sMSEvents_s),
	offsetof(struct sMSEvents_s, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_sMSEvents_2 = {
	"sMSEvents",
	"sMSEvents",
	SEQUENCE_OF_free,
	SEQUENCE_OF_print,
	SEQUENCE_OF_constraint,
	SEQUENCE_OF_decode_ber,
	SEQUENCE_OF_encode_der,
#ifndef ASN1_XER_NOT_USED
	SEQUENCE_OF_decode_xer,
	SEQUENCE_OF_encode_xer,
#else  /* ASN1_XER_NOT_USED */
	0, 0,
#endif /* ASN1_XER_NOT_USED */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_sMSEvents_2_tags,
	sizeof(asn_DEF_sMSEvents_2_tags)
		/sizeof(asn_DEF_sMSEvents_2_tags[0]) - 1, /* 1 */
	asn_DEF_sMSEvents_2_tags,	/* Same as above */
	sizeof(asn_DEF_sMSEvents_2_tags)
		/sizeof(asn_DEF_sMSEvents_2_tags[0]), /* 2 */
	asn_MBR_sMSEvents_2,
	1,	/* Single element */
	&asn_SPC_sMSEvents_2_specs	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_RequestReportSMSEventArg_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct RequestReportSMSEventArg, sMSEvents),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_sMSEvents_2,
		memb_sMSEvents_1_constraint,
		"sMSEvents"
		},
	{ ATF_POINTER, 1, offsetof(struct RequestReportSMSEventArg, extensions),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Extensions,
		0,	/* Defer constraints checking to the member type */
		"extensions"
		},
};
static ber_tlv_tag_t asn_DEF_RequestReportSMSEventArg_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_RequestReportSMSEventArg_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* sMSEvents at 245 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 1, 0, 0 } /* extensions at 246 */
};
static asn_SEQUENCE_specifics_t asn_SPC_RequestReportSMSEventArg_1_specs = {
	sizeof(struct RequestReportSMSEventArg),
	offsetof(struct RequestReportSMSEventArg, _asn_ctx),
	asn_MAP_RequestReportSMSEventArg_1_tag2el,
	2,	/* Count of tags in the map */
	1,	/* Start extensions */
	3	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_RequestReportSMSEventArg = {
	"RequestReportSMSEventArg",
	"RequestReportSMSEventArg",
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
	asn_DEF_RequestReportSMSEventArg_1_tags,
	sizeof(asn_DEF_RequestReportSMSEventArg_1_tags)
		/sizeof(asn_DEF_RequestReportSMSEventArg_1_tags[0]), /* 1 */
	asn_DEF_RequestReportSMSEventArg_1_tags,	/* Same as above */
	sizeof(asn_DEF_RequestReportSMSEventArg_1_tags)
		/sizeof(asn_DEF_RequestReportSMSEventArg_1_tags[0]), /* 1 */
	asn_MBR_RequestReportSMSEventArg_1,
	2,	/* Elements count */
	&asn_SPC_RequestReportSMSEventArg_1_specs	/* Additional specs */
};

