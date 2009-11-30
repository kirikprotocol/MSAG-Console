#include <asn_internal.h>

#include "O-BcsmCamelTDP-Criteria.h"

static asn_TYPE_member_t asn_MBR_O_BcsmCamelTDP_Criteria_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct O_BcsmCamelTDP_Criteria, o_BcsmTriggerDetectionPoint),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_O_BcsmTriggerDetectionPoint,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"o-BcsmTriggerDetectionPoint"
		},
	{ ATF_POINTER, 5, offsetof(struct O_BcsmCamelTDP_Criteria, destinationNumberCriteria),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DestinationNumberCriteria,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"destinationNumberCriteria"
		},
	{ ATF_POINTER, 4, offsetof(struct O_BcsmCamelTDP_Criteria, basicServiceCriteria),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BasicServiceCriteria,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"basicServiceCriteria"
		},
	{ ATF_POINTER, 3, offsetof(struct O_BcsmCamelTDP_Criteria, callTypeCriteria),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CallTypeCriteria,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"callTypeCriteria"
		},
	{ ATF_POINTER, 2, offsetof(struct O_BcsmCamelTDP_Criteria, o_CauseValueCriteria),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_O_CauseValueCriteria,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"o-CauseValueCriteria"
		},
	{ ATF_POINTER, 1, offsetof(struct O_BcsmCamelTDP_Criteria, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_O_BcsmCamelTDP_Criteria_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_O_BcsmCamelTDP_Criteria_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 0, 0, 0 }, /* o-BcsmTriggerDetectionPoint at 239 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 1, 0, 0 }, /* destinationNumberCriteria at 240 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 2, 0, 0 }, /* basicServiceCriteria at 241 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 3, 0, 0 }, /* callTypeCriteria at 242 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 4, 0, 0 }, /* o-CauseValueCriteria at 244 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 5, 0, 0 } /* extensionContainer at 245 */
};
static asn_SEQUENCE_specifics_t asn_SPC_O_BcsmCamelTDP_Criteria_specs_1 = {
	sizeof(struct O_BcsmCamelTDP_Criteria),
	offsetof(struct O_BcsmCamelTDP_Criteria, _asn_ctx),
	asn_MAP_O_BcsmCamelTDP_Criteria_tag2el_1,
	6,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	3,	/* Start extensions */
	7	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_O_BcsmCamelTDP_Criteria = {
	"O-BcsmCamelTDP-Criteria",
	"O-BcsmCamelTDP-Criteria",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_O_BcsmCamelTDP_Criteria_tags_1,
	sizeof(asn_DEF_O_BcsmCamelTDP_Criteria_tags_1)
		/sizeof(asn_DEF_O_BcsmCamelTDP_Criteria_tags_1[0]), /* 1 */
	asn_DEF_O_BcsmCamelTDP_Criteria_tags_1,	/* Same as above */
	sizeof(asn_DEF_O_BcsmCamelTDP_Criteria_tags_1)
		/sizeof(asn_DEF_O_BcsmCamelTDP_Criteria_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_O_BcsmCamelTDP_Criteria_1,
	6,	/* Elements count */
	&asn_SPC_O_BcsmCamelTDP_Criteria_specs_1	/* Additional specs */
};

