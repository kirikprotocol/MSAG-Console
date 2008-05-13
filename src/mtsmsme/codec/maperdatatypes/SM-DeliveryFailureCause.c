#include <asn_internal.h>

#include "SM-DeliveryFailureCause.h"

static asn_TYPE_member_t asn_MBR_SM_DeliveryFailureCause_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct SM_DeliveryFailureCause, sm_EnumeratedDeliveryFailureCause),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_SM_EnumeratedDeliveryFailureCause,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"sm-EnumeratedDeliveryFailureCause"
		},
	{ ATF_POINTER, 2, offsetof(struct SM_DeliveryFailureCause, diagnosticInfo),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_SignalInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"diagnosticInfo"
		},
	{ ATF_POINTER, 1, offsetof(struct SM_DeliveryFailureCause, extensionContainer),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_SM_DeliveryFailureCause_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_SM_DeliveryFailureCause_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 1, 0, 0 }, /* diagnosticInfo at 282 */
    { (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 0, 0, 0 }, /* sm-EnumeratedDeliveryFailureCause at 281 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 2, 0, 0 } /* extensionContainer at 283 */
};
static asn_SEQUENCE_specifics_t asn_SPC_SM_DeliveryFailureCause_specs_1 = {
	sizeof(struct SM_DeliveryFailureCause),
	offsetof(struct SM_DeliveryFailureCause, _asn_ctx),
	asn_MAP_SM_DeliveryFailureCause_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	2,	/* Start extensions */
	4	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_SM_DeliveryFailureCause = {
	"SM-DeliveryFailureCause",
	"SM-DeliveryFailureCause",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_SM_DeliveryFailureCause_tags_1,
	sizeof(asn_DEF_SM_DeliveryFailureCause_tags_1)
		/sizeof(asn_DEF_SM_DeliveryFailureCause_tags_1[0]), /* 1 */
	asn_DEF_SM_DeliveryFailureCause_tags_1,	/* Same as above */
	sizeof(asn_DEF_SM_DeliveryFailureCause_tags_1)
		/sizeof(asn_DEF_SM_DeliveryFailureCause_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_SM_DeliveryFailureCause_1,
	3,	/* Elements count */
	&asn_SPC_SM_DeliveryFailureCause_specs_1	/* Additional specs */
};

