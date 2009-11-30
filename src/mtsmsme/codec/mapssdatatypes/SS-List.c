#include <asn_internal.h>

#include "SS-List.h"

static asn_TYPE_member_t asn_MBR_SS_List_1[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_SS_Code,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		""
		},
};
static ber_tlv_tag_t asn_DEF_SS_List_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_SS_List_specs_1 = {
	sizeof(struct SS_List),
	offsetof(struct SS_List, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
asn_TYPE_descriptor_t asn_DEF_SS_List = {
	"SS-List",
	"SS-List",
	SEQUENCE_OF_free,
	SEQUENCE_OF_print,
	SEQUENCE_OF_constraint,
	SEQUENCE_OF_decode_ber,
	SEQUENCE_OF_encode_der,
	SEQUENCE_OF_decode_xer,
	SEQUENCE_OF_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_SS_List_tags_1,
	sizeof(asn_DEF_SS_List_tags_1)
		/sizeof(asn_DEF_SS_List_tags_1[0]), /* 1 */
	asn_DEF_SS_List_tags_1,	/* Same as above */
	sizeof(asn_DEF_SS_List_tags_1)
		/sizeof(asn_DEF_SS_List_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_SS_List_1,
	1,	/* Single element */
	&asn_SPC_SS_List_specs_1	/* Additional specs */
};

