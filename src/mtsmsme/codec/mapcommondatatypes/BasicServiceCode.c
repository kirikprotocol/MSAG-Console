#include <asn_internal.h>

#include "BasicServiceCode.h"

static asn_TYPE_member_t asn_MBR_BasicServiceCode_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct BasicServiceCode, choice.bearerService),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BearerServiceCode,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"bearerService"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct BasicServiceCode, choice.teleservice),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TeleserviceCode,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"teleservice"
		},
};
static asn_TYPE_tag2member_t asn_MAP_BasicServiceCode_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 0, 0, 0 }, /* bearerService at 344 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 1, 0, 0 } /* teleservice at 346 */
};
static asn_CHOICE_specifics_t asn_SPC_BasicServiceCode_specs_1 = {
	sizeof(struct BasicServiceCode),
	offsetof(struct BasicServiceCode, _asn_ctx),
	offsetof(struct BasicServiceCode, present),
	sizeof(((struct BasicServiceCode *)0)->present),
	asn_MAP_BasicServiceCode_tag2el_1,
	2,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_BasicServiceCode = {
	"BasicServiceCode",
	"BasicServiceCode",
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
	asn_MBR_BasicServiceCode_1,
	2,	/* Elements count */
	&asn_SPC_BasicServiceCode_specs_1	/* Additional specs */
};

