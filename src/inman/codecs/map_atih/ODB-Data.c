#ident "$Id$"

#include <asn_internal.h>

#include "ODB-Data.h"

static asn_TYPE_member_t asn_MBR_ODB_Data_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct ODB_Data, odb_GeneralData),
		(ASN_TAG_CLASS_UNIVERSAL | (3 << 2)),
		0,
		&asn_DEF_ODB_GeneralData,
		0,	/* Defer constraints checking to the member type */
		"odb-GeneralData"
		},
	{ ATF_POINTER, 2, offsetof(struct ODB_Data, odb_HPLMN_Data),
		(ASN_TAG_CLASS_UNIVERSAL | (3 << 2)),
		0,
		&asn_DEF_ODB_HPLMN_Data,
		0,	/* Defer constraints checking to the member type */
		"odb-HPLMN-Data"
		},
	{ ATF_POINTER, 1, offsetof(struct ODB_Data, extensionContainer),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_ODB_Data_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_ODB_Data_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (3 << 2)), 0, 0, 1 }, /* odb-GeneralData at 266 */
    { (ASN_TAG_CLASS_UNIVERSAL | (3 << 2)), 1, -1, 0 }, /* odb-HPLMN-Data at 267 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 2, 0, 0 } /* extensionContainer at 268 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ODB_Data_1_specs = {
	sizeof(struct ODB_Data),
	offsetof(struct ODB_Data, _asn_ctx),
	asn_MAP_ODB_Data_1_tag2el,
	3,	/* Count of tags in the map */
	2,	/* Start extensions */
	4	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_ODB_Data = {
	"ODB-Data",
	"ODB-Data",
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
	asn_DEF_ODB_Data_1_tags,
	sizeof(asn_DEF_ODB_Data_1_tags)
		/sizeof(asn_DEF_ODB_Data_1_tags[0]), /* 1 */
	asn_DEF_ODB_Data_1_tags,	/* Same as above */
	sizeof(asn_DEF_ODB_Data_1_tags)
		/sizeof(asn_DEF_ODB_Data_1_tags[0]), /* 1 */
	asn_MBR_ODB_Data_1,
	3,	/* Elements count */
	&asn_SPC_ODB_Data_1_specs	/* Additional specs */
};

