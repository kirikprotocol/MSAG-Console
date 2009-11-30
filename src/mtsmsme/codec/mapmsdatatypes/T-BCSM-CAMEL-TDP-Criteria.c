#include <asn_internal.h>

#include "T-BCSM-CAMEL-TDP-Criteria.h"

static asn_TYPE_member_t asn_MBR_T_BCSM_CAMEL_TDP_Criteria_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct T_BCSM_CAMEL_TDP_Criteria, t_BCSM_TriggerDetectionPoint),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_T_BcsmTriggerDetectionPoint,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"t-BCSM-TriggerDetectionPoint"
		},
	{ ATF_POINTER, 2, offsetof(struct T_BCSM_CAMEL_TDP_Criteria, basicServiceCriteria),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BasicServiceCriteria,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"basicServiceCriteria"
		},
	{ ATF_POINTER, 1, offsetof(struct T_BCSM_CAMEL_TDP_Criteria, t_CauseValueCriteria),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_T_CauseValueCriteria,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"t-CauseValueCriteria"
		},
};
static ber_tlv_tag_t asn_DEF_T_BCSM_CAMEL_TDP_Criteria_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_T_BCSM_CAMEL_TDP_Criteria_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 0, 0, 0 }, /* t-BCSM-TriggerDetectionPoint at 248 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 1, 0, 0 }, /* basicServiceCriteria at 249 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 2, 0, 0 } /* t-CauseValueCriteria at 250 */
};
static asn_SEQUENCE_specifics_t asn_SPC_T_BCSM_CAMEL_TDP_Criteria_specs_1 = {
	sizeof(struct T_BCSM_CAMEL_TDP_Criteria),
	offsetof(struct T_BCSM_CAMEL_TDP_Criteria, _asn_ctx),
	asn_MAP_T_BCSM_CAMEL_TDP_Criteria_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
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
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_T_BCSM_CAMEL_TDP_Criteria_tags_1,
	sizeof(asn_DEF_T_BCSM_CAMEL_TDP_Criteria_tags_1)
		/sizeof(asn_DEF_T_BCSM_CAMEL_TDP_Criteria_tags_1[0]), /* 1 */
	asn_DEF_T_BCSM_CAMEL_TDP_Criteria_tags_1,	/* Same as above */
	sizeof(asn_DEF_T_BCSM_CAMEL_TDP_Criteria_tags_1)
		/sizeof(asn_DEF_T_BCSM_CAMEL_TDP_Criteria_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_T_BCSM_CAMEL_TDP_Criteria_1,
	3,	/* Elements count */
	&asn_SPC_T_BCSM_CAMEL_TDP_Criteria_specs_1	/* Additional specs */
};

