#include <asn_internal.h>

#include "SM-RP-DA.h"

static asn_TYPE_member_t asn_MBR_SM_RP_DA_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct SM_RP_DA, choice.imsi),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_IMSI,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"imsi"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SM_RP_DA, choice.lmsi),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_LMSI,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"lmsi"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SM_RP_DA, choice.serviceCentreAddressDA),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"serviceCentreAddressDA"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SM_RP_DA, choice.noSM_RP_DA),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"noSM-RP-DA"
		},
};
static asn_TYPE_tag2member_t asn_MAP_SM_RP_DA_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* imsi at 139 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* lmsi at 140 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 2, 0, 0 }, /* serviceCentreAddressDA at 141 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 3, 0, 0 } /* noSM-RP-DA at 142 */
};
static asn_CHOICE_specifics_t asn_SPC_SM_RP_DA_specs_1 = {
	sizeof(struct SM_RP_DA),
	offsetof(struct SM_RP_DA, _asn_ctx),
	offsetof(struct SM_RP_DA, present),
	sizeof(((struct SM_RP_DA *)0)->present),
	asn_MAP_SM_RP_DA_tag2el_1,
	4,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_SM_RP_DA = {
	"SM-RP-DA",
	"SM-RP-DA",
	CHOICE_free,
	CHOICE_print,
	CHOICE_constraint,
	CHOICE_decode_ber,
	CHOICE_encode_der,
	CHOICE_decode_xer,
	CHOICE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	CHOICE_outmost_tag,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	0,	/* No PER visible constraints */
	asn_MBR_SM_RP_DA_1,
	4,	/* Elements count */
	&asn_SPC_SM_RP_DA_specs_1	/* Additional specs */
};

