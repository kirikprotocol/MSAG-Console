#ident "$Id$"

#include <asn_internal.h>

#include "MT-smsCAMELTDP-Criteria.h"

static asn_TYPE_member_t asn_MBR_MT_smsCAMELTDP_Criteria_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct MT_smsCAMELTDP_Criteria, sms_TriggerDetectionPoint),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_SMS_TriggerDetectionPoint,
		0,	/* Defer constraints checking to the member type */
		"sms-TriggerDetectionPoint"
		},
	{ ATF_POINTER, 1, offsetof(struct MT_smsCAMELTDP_Criteria, tpdu_TypeCriterion),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TPDU_TypeCriterion,
		0,	/* Defer constraints checking to the member type */
		"tpdu-TypeCriterion"
		},
};
static ber_tlv_tag_t asn_DEF_MT_smsCAMELTDP_Criteria_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_MT_smsCAMELTDP_Criteria_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 0, 0, 0 }, /* sms-TriggerDetectionPoint at 395 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 1, 0, 0 } /* tpdu-TypeCriterion at 396 */
};
static asn_SEQUENCE_specifics_t asn_SPC_MT_smsCAMELTDP_Criteria_1_specs = {
	sizeof(struct MT_smsCAMELTDP_Criteria),
	offsetof(struct MT_smsCAMELTDP_Criteria, _asn_ctx),
	asn_MAP_MT_smsCAMELTDP_Criteria_1_tag2el,
	2,	/* Count of tags in the map */
	1,	/* Start extensions */
	3	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_MT_smsCAMELTDP_Criteria = {
	"MT-smsCAMELTDP-Criteria",
	"MT-smsCAMELTDP-Criteria",
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
	asn_DEF_MT_smsCAMELTDP_Criteria_1_tags,
	sizeof(asn_DEF_MT_smsCAMELTDP_Criteria_1_tags)
		/sizeof(asn_DEF_MT_smsCAMELTDP_Criteria_1_tags[0]), /* 1 */
	asn_DEF_MT_smsCAMELTDP_Criteria_1_tags,	/* Same as above */
	sizeof(asn_DEF_MT_smsCAMELTDP_Criteria_1_tags)
		/sizeof(asn_DEF_MT_smsCAMELTDP_Criteria_1_tags[0]), /* 1 */
	asn_MBR_MT_smsCAMELTDP_Criteria_1,
	2,	/* Elements count */
	&asn_SPC_MT_smsCAMELTDP_Criteria_1_specs	/* Additional specs */
};

