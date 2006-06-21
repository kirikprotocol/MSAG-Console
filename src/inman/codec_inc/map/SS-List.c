#ident "$Id$"

#include <asn_internal.h>

#include "SS-List.h"

static asn_TYPE_member_t asn_MBR_SS_List_1[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_SS_Code,
		0,	/* Defer constraints checking to the member type */
		""
		},
};
static ber_tlv_tag_t asn_DEF_SS_List_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_SS_List_1_specs = {
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
#ifndef ASN1_XER_NOT_USED
	SEQUENCE_OF_decode_xer,
	SEQUENCE_OF_encode_xer,
#else  /* ASN1_XER_NOT_USED */
	0, 0,
#endif /* ASN1_XER_NOT_USED */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_SS_List_1_tags,
	sizeof(asn_DEF_SS_List_1_tags)
		/sizeof(asn_DEF_SS_List_1_tags[0]), /* 1 */
	asn_DEF_SS_List_1_tags,	/* Same as above */
	sizeof(asn_DEF_SS_List_1_tags)
		/sizeof(asn_DEF_SS_List_1_tags[0]), /* 1 */
	asn_MBR_SS_List_1,
	1,	/* Single element */
	&asn_SPC_SS_List_1_specs	/* Additional specs */
};

