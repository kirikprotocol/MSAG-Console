#include <asn_internal.h>

#include "T-BCSM-CAMEL-TDP-CriteriaList.h"

static asn_TYPE_member_t asn_MBR_T_BCSM_CAMEL_TDP_CriteriaList_1[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_T_BCSM_CAMEL_TDP_Criteria,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		""
		},
};
static ber_tlv_tag_t asn_DEF_T_BCSM_CAMEL_TDP_CriteriaList_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_T_BCSM_CAMEL_TDP_CriteriaList_specs_1 = {
	sizeof(struct T_BCSM_CAMEL_TDP_CriteriaList),
	offsetof(struct T_BCSM_CAMEL_TDP_CriteriaList, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
asn_TYPE_descriptor_t asn_DEF_T_BCSM_CAMEL_TDP_CriteriaList = {
	"T-BCSM-CAMEL-TDP-CriteriaList",
	"T-BCSM-CAMEL-TDP-CriteriaList",
	SEQUENCE_OF_free,
	SEQUENCE_OF_print,
	SEQUENCE_OF_constraint,
	SEQUENCE_OF_decode_ber,
	SEQUENCE_OF_encode_der,
	SEQUENCE_OF_decode_xer,
	SEQUENCE_OF_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_T_BCSM_CAMEL_TDP_CriteriaList_tags_1,
	sizeof(asn_DEF_T_BCSM_CAMEL_TDP_CriteriaList_tags_1)
		/sizeof(asn_DEF_T_BCSM_CAMEL_TDP_CriteriaList_tags_1[0]), /* 1 */
	asn_DEF_T_BCSM_CAMEL_TDP_CriteriaList_tags_1,	/* Same as above */
	sizeof(asn_DEF_T_BCSM_CAMEL_TDP_CriteriaList_tags_1)
		/sizeof(asn_DEF_T_BCSM_CAMEL_TDP_CriteriaList_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_T_BCSM_CAMEL_TDP_CriteriaList_1,
	1,	/* Single element */
	&asn_SPC_T_BCSM_CAMEL_TDP_CriteriaList_specs_1	/* Additional specs */
};

