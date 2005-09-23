/*-
 * Copyright (c) 2004 Lev Walkin <vlm@lionet.info>. All rights reserved.
 * Redistribution and modifications are permitted subject to BSD license.
 */
/*
 * Please read the NativeInteger.h for the explanation wrt. differences between
 * INTEGER and NativeInteger.
 * Basically, both are decoders and encoders of ASN.1 INTEGER type, but this
 * implementation deals with the standard (machine-specific) representation
 * of them instead of using the platform-independent buffer.
 */
#include <asn_internal.h>
#include <NativeEnumerated.h>

/*
 * NativeEnumerated basic type description.
 */
static ber_tlv_tag_t asn_DEF_NativeEnumerated_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
asn_TYPE_descriptor_t asn_DEF_NativeEnumerated = {
	"ENUMERATED",			/* The ASN.1 type is still ENUMERATED */
	"ENUMERATED",
	NativeInteger_free,
	NativeInteger_print,
	asn_generic_no_constraint,
	NativeInteger_decode_ber,
	NativeInteger_encode_der,
#ifndef ASN1_XER_NOT_USED
	NativeInteger_decode_xer,
	NativeEnumerated_encode_xer,
#else  /* ASN1_XER_NOT_USED */
	0, 0,
#endif /* ASN1_XER_NOT_USED */
	0, /* Use generic outmost tag fetcher */
	asn_DEF_NativeEnumerated_tags,
	sizeof(asn_DEF_NativeEnumerated_tags) / sizeof(asn_DEF_NativeEnumerated_tags[0]),
	asn_DEF_NativeEnumerated_tags,	/* Same as above */
	sizeof(asn_DEF_NativeEnumerated_tags) / sizeof(asn_DEF_NativeEnumerated_tags[0]),
	0, 0,	/* No members */
	0	/* No specifics */
};

#ifndef ASN1_XER_NOT_USED
asn_enc_rval_t
NativeEnumerated_encode_xer(asn_TYPE_descriptor_t *td, void *sptr,
        int ilevel, enum xer_encoder_flags_e flags,
                asn_app_consume_bytes_f *cb, void *app_key) {
	asn_INTEGER_specifics_t *specs=(asn_INTEGER_specifics_t *)td->specifics;
        asn_enc_rval_t er;
        const long *native = (const long *)sptr;
	const asn_INTEGER_enum_map_t *el;

        (void)ilevel;
        (void)flags;

        if(!native) _ASN_ENCODE_FAILED;

	el = INTEGER_map_value2enum(specs, *native);
	if(el) {
		size_t srcsize = el->enum_len + 5;
		char *src = (char *)alloca(srcsize);

		er.encoded = snprintf(src, srcsize, "<%s/>", el->enum_name);
		assert(er.encoded > 0 && (size_t)er.encoded < srcsize);
		if(cb(src, er.encoded, app_key) < 0) _ASN_ENCODE_FAILED;
		return er;
	} else {
		ASN_DEBUG("ASN.1 forbids dealing with "
			"unknown value of ENUMERATED type");
		_ASN_ENCODE_FAILED;
	}

        return er;
}
#endif /* ASN1_XER_NOT_USED */
