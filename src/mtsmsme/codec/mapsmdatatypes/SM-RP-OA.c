#include <asn_internal.h>

#include "SM-RP-OA.h"

static asn_TYPE_member_t asn_MBR_SM_RP_OA_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct SM_RP_OA, choice.msisdn),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"msisdn"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SM_RP_OA, choice.serviceCentreAddressOA),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"serviceCentreAddressOA"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SM_RP_OA, choice.noSM_RP_OA),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"noSM-RP-OA"
		},
};
static asn_TYPE_tag2member_t asn_MAP_SM_RP_OA_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 0, 0, 0 }, /* msisdn at 145 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 1, 0, 0 }, /* serviceCentreAddressOA at 146 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 2, 0, 0 } /* noSM-RP-OA at 147 */
};
static asn_CHOICE_specifics_t asn_SPC_SM_RP_OA_specs_1 = {
	sizeof(struct SM_RP_OA),
	offsetof(struct SM_RP_OA, _asn_ctx),
	offsetof(struct SM_RP_OA, present),
	sizeof(((struct SM_RP_OA *)0)->present),
	asn_MAP_SM_RP_OA_tag2el_1,
	3,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_SM_RP_OA = {
	"SM-RP-OA",
	"SM-RP-OA",
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
	asn_MBR_SM_RP_OA_1,
	3,	/* Elements count */
	&asn_SPC_SM_RP_OA_specs_1	/* Additional specs */
};

