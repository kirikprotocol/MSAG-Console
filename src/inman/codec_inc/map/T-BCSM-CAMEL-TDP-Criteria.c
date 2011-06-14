/* #ident "$Id$" */

#include <asn_internal.h>

#include "T-BCSM-CAMEL-TDP-Criteria.h"

static asn_TYPE_member_t asn_MBR_T_BCSM_CAMEL_TDP_Criteria_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct T_BCSM_CAMEL_TDP_Criteria, t_BCSM_TriggerDetectionPoint),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_T_BcsmTriggerDetectionPoint,
		0,	/* Defer constraints checking to the member type */
		"t-BCSM-TriggerDetectionPoint"
		},
	{ ATF_POINTER, 2, offsetof(struct T_BCSM_CAMEL_TDP_Criteria, basicServiceCriteria),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BasicServiceCriteria,
		0,	/* Defer constraints checking to the member type */
		"basicServiceCriteria"
		},
	{ ATF_POINTER, 1, offsetof(struct T_BCSM_CAMEL_TDP_Criteria, t_CauseValueCriteria),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_T_CauseValueCriteria,
		0,	/* Defer constraints checking to the member type */
		"t-CauseValueCriteria"
		},
};
static ber_tlv_tag_t asn_DEF_T_BCSM_CAMEL_TDP_Criteria_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_T_BCSM_CAMEL_TDP_Criteria_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 0, 0, 0 }, /* t-BCSM-TriggerDetectionPoint at 239 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 1, 0, 0 }, /* basicServiceCriteria at 240 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 2, 0, 0 } /* t-CauseValueCriteria at 241 */
};
static asn_SEQUENCE_specifics_t asn_SPC_T_BCSM_CAMEL_TDP_Criteria_1_specs = {
	sizeof(struct T_BCSM_CAMEL_TDP_Criteria),
	offsetof(struct T_BCSM_CAMEL_TDP_Criteria, _asn_ctx),
	asn_MAP_T_BCSM_CAMEL_TDP_Criteria_1_tag2el,
	3,	/* Count of tags in the map */
	2,	/* Start extensions */
	4	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_T_BCSM_CAMEL_TDP_Criteria = {
	"T-BCSM-CAMEL-TDP-Criteria",
	"T-BCSM-CAMEL-TDP-Criteria",
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
	asn_DEF_T_BCSM_CAMEL_TDP_Criteria_1_tags,
	sizeof(asn_DEF_T_BCSM_CAMEL_TDP_Criteria_1_tags)
		/sizeof(asn_DEF_T_BCSM_CAMEL_TDP_Criteria_1_tags[0]), /* 1 */
	asn_DEF_T_BCSM_CAMEL_TDP_Criteria_1_tags,	/* Same as above */
	sizeof(asn_DEF_T_BCSM_CAMEL_TDP_Criteria_1_tags)
		/sizeof(asn_DEF_T_BCSM_CAMEL_TDP_Criteria_1_tags[0]), /* 1 */
	asn_MBR_T_BCSM_CAMEL_TDP_Criteria_1,
	3,	/* Elements count */
	&asn_SPC_T_BCSM_CAMEL_TDP_Criteria_1_specs	/* Additional specs */
};

