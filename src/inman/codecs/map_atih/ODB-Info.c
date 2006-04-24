#ident "$Id$"

#include <asn_internal.h>

#include "ODB-Info.h"

static asn_TYPE_member_t asn_MBR_ODB_Info_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct ODB_Info, odb_Data),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ODB_Data,
		0,	/* Defer constraints checking to the member type */
		"odb-Data"
		},
	{ ATF_POINTER, 2, offsetof(struct ODB_Info, notificationToCSE),
		(ASN_TAG_CLASS_UNIVERSAL | (5 << 2)),
		0,
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"notificationToCSE"
		},
	{ ATF_POINTER, 1, offsetof(struct ODB_Info, extensionContainer),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_ODB_Info_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_ODB_Info_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (5 << 2)), 1, 0, 0 }, /* notificationToCSE at 828 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 0, 0, 1 }, /* odb-Data at 827 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 2, -1, 0 } /* extensionContainer at 829 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ODB_Info_1_specs = {
	sizeof(struct ODB_Info),
	offsetof(struct ODB_Info, _asn_ctx),
	asn_MAP_ODB_Info_1_tag2el,
	3,	/* Count of tags in the map */
	2,	/* Start extensions */
	4	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_ODB_Info = {
	"ODB-Info",
	"ODB-Info",
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
	asn_DEF_ODB_Info_1_tags,
	sizeof(asn_DEF_ODB_Info_1_tags)
		/sizeof(asn_DEF_ODB_Info_1_tags[0]), /* 1 */
	asn_DEF_ODB_Info_1_tags,	/* Same as above */
	sizeof(asn_DEF_ODB_Info_1_tags)
		/sizeof(asn_DEF_ODB_Info_1_tags[0]), /* 1 */
	asn_MBR_ODB_Info_1,
	3,	/* Elements count */
	&asn_SPC_ODB_Info_1_specs	/* Additional specs */
};

