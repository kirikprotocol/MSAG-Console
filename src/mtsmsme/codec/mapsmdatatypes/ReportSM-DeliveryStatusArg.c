#include <asn_internal.h>

#include "ReportSM-DeliveryStatusArg.h"

static asn_TYPE_member_t asn_MBR_ReportSM_DeliveryStatusArg_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct ReportSM_DeliveryStatusArg, msisdn),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"msisdn"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct ReportSM_DeliveryStatusArg, serviceCentreAddress),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"serviceCentreAddress"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct ReportSM_DeliveryStatusArg, sm_DeliveryOutcome),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_SM_DeliveryOutcome,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"sm-DeliveryOutcome"
		},
	{ ATF_POINTER, 6, offsetof(struct ReportSM_DeliveryStatusArg, absentSubscriberDiagnosticSM),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_AbsentSubscriberDiagnosticSM,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"absentSubscriberDiagnosticSM"
		},
	{ ATF_POINTER, 5, offsetof(struct ReportSM_DeliveryStatusArg, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
	{ ATF_POINTER, 4, offsetof(struct ReportSM_DeliveryStatusArg, gprsSupportIndicator),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"gprsSupportIndicator"
		},
	{ ATF_POINTER, 3, offsetof(struct ReportSM_DeliveryStatusArg, deliveryOutcomeIndicator),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"deliveryOutcomeIndicator"
		},
	{ ATF_POINTER, 2, offsetof(struct ReportSM_DeliveryStatusArg, additionalSM_DeliveryOutcome),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SM_DeliveryOutcome,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"additionalSM-DeliveryOutcome"
		},
	{ ATF_POINTER, 1, offsetof(struct ReportSM_DeliveryStatusArg, additionalAbsentSubscriberDiagnosticSM),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_AbsentSubscriberDiagnosticSM,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"additionalAbsentSubscriberDiagnosticSM"
		},
};
static ber_tlv_tag_t asn_DEF_ReportSM_DeliveryStatusArg_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_ReportSM_DeliveryStatusArg_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 0, 0, 1 }, /* msisdn at 150 */
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 1, -1, 0 }, /* serviceCentreAddress at 151 */
    { (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 2, 0, 0 }, /* sm-DeliveryOutcome at 152 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 3, 0, 0 }, /* absentSubscriberDiagnosticSM at 154 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 4, 0, 0 }, /* extensionContainer at 155 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 5, 0, 0 }, /* gprsSupportIndicator at 157 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 6, 0, 0 }, /* deliveryOutcomeIndicator at 160 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 7, 0, 0 }, /* additionalSM-DeliveryOutcome at 163 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 8, 0, 0 } /* additionalAbsentSubscriberDiagnosticSM at 166 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ReportSM_DeliveryStatusArg_specs_1 = {
	sizeof(struct ReportSM_DeliveryStatusArg),
	offsetof(struct ReportSM_DeliveryStatusArg, _asn_ctx),
	asn_MAP_ReportSM_DeliveryStatusArg_tag2el_1,
	9,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	4,	/* Start extensions */
	10	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_ReportSM_DeliveryStatusArg = {
	"ReportSM-DeliveryStatusArg",
	"ReportSM-DeliveryStatusArg",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_ReportSM_DeliveryStatusArg_tags_1,
	sizeof(asn_DEF_ReportSM_DeliveryStatusArg_tags_1)
		/sizeof(asn_DEF_ReportSM_DeliveryStatusArg_tags_1[0]), /* 1 */
	asn_DEF_ReportSM_DeliveryStatusArg_tags_1,	/* Same as above */
	sizeof(asn_DEF_ReportSM_DeliveryStatusArg_tags_1)
		/sizeof(asn_DEF_ReportSM_DeliveryStatusArg_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_ReportSM_DeliveryStatusArg_1,
	9,	/* Elements count */
	&asn_SPC_ReportSM_DeliveryStatusArg_specs_1	/* Additional specs */
};

