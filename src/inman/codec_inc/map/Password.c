#ident "$Id$"

#include <asn_internal.h>

#include "Password.h"

static int check_permitted_alphabet_1(const void *sptr) {
	/* The underlying type is NumericString */
	const NumericString_t *st = (const NumericString_t *)sptr;
	const uint8_t *ch = st->buf;
	const uint8_t *end = ch + st->size;
	
	for(; ch < end; ch++) {
		uint8_t cv = *ch;
		if(!(cv >= 48 && cv <= 57)) return -1;
	}
	return 0;
}

int
Password_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_consume_bytes_f *app_errlog, void *app_key) {
	const NumericString_t *st = (const NumericString_t *)sptr;
	size_t size;
	
	if(!sptr) {
		_ASN_ERRLOG(app_errlog, app_key,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	size = st->size;
	
	if((size == 4)
		 && !check_permitted_alphabet_1(st)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		_ASN_ERRLOG(app_errlog, app_key,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

/*
 * This type is implemented using NumericString,
 * so here we adjust the DEF accordingly.
 */
static void
Password_1_inherit_TYPE_descriptor(asn_TYPE_descriptor_t *td) {
	td->free_struct    = asn_DEF_NumericString.free_struct;
	td->print_struct   = asn_DEF_NumericString.print_struct;
	td->ber_decoder    = asn_DEF_NumericString.ber_decoder;
	td->der_encoder    = asn_DEF_NumericString.der_encoder;
#ifndef ASN1_XER_NOT_USED
	td->xer_decoder    = asn_DEF_NumericString.xer_decoder;
	td->xer_encoder    = asn_DEF_NumericString.xer_encoder;
#else /* ASN1_XER_NOT_USED */
	td->xer_decoder    = NULL;
	td->xer_encoder    = NULL;
#endif /* ASN1_XER_NOT_USED */
	td->elements       = asn_DEF_NumericString.elements;
	td->elements_count = asn_DEF_NumericString.elements_count;
	td->specifics      = asn_DEF_NumericString.specifics;
}

void
Password_free(asn_TYPE_descriptor_t *td,
		void *struct_ptr, int contents_only) {
	Password_1_inherit_TYPE_descriptor(td);
	td->free_struct(td, struct_ptr, contents_only);
}

int
Password_print(asn_TYPE_descriptor_t *td, const void *struct_ptr,
		int ilevel, asn_app_consume_bytes_f *cb, void *app_key) {
	Password_1_inherit_TYPE_descriptor(td);
	return td->print_struct(td, struct_ptr, ilevel, cb, app_key);
}

asn_dec_rval_t
Password_decode_ber(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const void *bufptr, size_t size, int tag_mode) {
	Password_1_inherit_TYPE_descriptor(td);
	return td->ber_decoder(opt_codec_ctx, td, structure, bufptr, size, tag_mode);
}

asn_enc_rval_t
Password_encode_der(asn_TYPE_descriptor_t *td,
		void *structure, int tag_mode, ber_tlv_tag_t tag,
		asn_app_consume_bytes_f *cb, void *app_key) {
	Password_1_inherit_TYPE_descriptor(td);
	return td->der_encoder(td, structure, tag_mode, tag, cb, app_key);
}

#ifndef ASN1_XER_NOT_USED
asn_dec_rval_t
Password_decode_xer(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const char *opt_mname, const void *bufptr, size_t size) {
	Password_1_inherit_TYPE_descriptor(td);
	return td->xer_decoder(opt_codec_ctx, td, structure, opt_mname, bufptr, size);
}

asn_enc_rval_t
Password_encode_xer(asn_TYPE_descriptor_t *td, void *structure,
		int ilevel, enum xer_encoder_flags_e flags,
		asn_app_consume_bytes_f *cb, void *app_key) {
	Password_1_inherit_TYPE_descriptor(td);
	return td->xer_encoder(td, structure, ilevel, flags, cb, app_key);
}

#endif /* ASN1_XER_NOT_USED */
static ber_tlv_tag_t asn_DEF_Password_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (18 << 2))
};
asn_TYPE_descriptor_t asn_DEF_Password = {
	"Password",
	"Password",
	Password_free,
	Password_print,
	Password_constraint,
	Password_decode_ber,
	Password_encode_der,
#ifndef ASN1_XER_NOT_USED
	Password_decode_xer,
	Password_encode_xer,
#else  /* ASN1_XER_NOT_USED */
	0, 0,
#endif /* ASN1_XER_NOT_USED */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_Password_1_tags,
	sizeof(asn_DEF_Password_1_tags)
		/sizeof(asn_DEF_Password_1_tags[0]), /* 1 */
	asn_DEF_Password_1_tags,	/* Same as above */
	sizeof(asn_DEF_Password_1_tags)
		/sizeof(asn_DEF_Password_1_tags[0]), /* 1 */
	0, 0,	/* No members */
	0	/* No specifics */
};

