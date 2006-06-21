#ident "$Id$"

#include <asn_internal.h>

#include "SuppressionOfAnnouncement.h"

int
SuppressionOfAnnouncement_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_consume_bytes_f *app_errlog, void *app_key) {
	/* Replace with underlying type checker */
	td->check_constraints = asn_DEF_NULL.check_constraints;
	return td->check_constraints(td, sptr, app_errlog, app_key);
}

/*
 * This type is implemented using NULL,
 * so here we adjust the DEF accordingly.
 */
static void
SuppressionOfAnnouncement_1_inherit_TYPE_descriptor(asn_TYPE_descriptor_t *td) {
	td->free_struct    = asn_DEF_NULL.free_struct;
	td->print_struct   = asn_DEF_NULL.print_struct;
	td->ber_decoder    = asn_DEF_NULL.ber_decoder;
	td->der_encoder    = asn_DEF_NULL.der_encoder;
#ifndef ASN1_XER_NOT_USED
	td->xer_decoder    = asn_DEF_NULL.xer_decoder;
	td->xer_encoder    = asn_DEF_NULL.xer_encoder;
#else /* ASN1_XER_NOT_USED */
	td->xer_decoder    = NULL;
	td->xer_encoder    = NULL;
#endif /* ASN1_XER_NOT_USED */
	td->elements       = asn_DEF_NULL.elements;
	td->elements_count = asn_DEF_NULL.elements_count;
	td->specifics      = asn_DEF_NULL.specifics;
}

void
SuppressionOfAnnouncement_free(asn_TYPE_descriptor_t *td,
		void *struct_ptr, int contents_only) {
	SuppressionOfAnnouncement_1_inherit_TYPE_descriptor(td);
	td->free_struct(td, struct_ptr, contents_only);
}

int
SuppressionOfAnnouncement_print(asn_TYPE_descriptor_t *td, const void *struct_ptr,
		int ilevel, asn_app_consume_bytes_f *cb, void *app_key) {
	SuppressionOfAnnouncement_1_inherit_TYPE_descriptor(td);
	return td->print_struct(td, struct_ptr, ilevel, cb, app_key);
}

asn_dec_rval_t
SuppressionOfAnnouncement_decode_ber(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const void *bufptr, size_t size, int tag_mode) {
	SuppressionOfAnnouncement_1_inherit_TYPE_descriptor(td);
	return td->ber_decoder(opt_codec_ctx, td, structure, bufptr, size, tag_mode);
}

asn_enc_rval_t
SuppressionOfAnnouncement_encode_der(asn_TYPE_descriptor_t *td,
		void *structure, int tag_mode, ber_tlv_tag_t tag,
		asn_app_consume_bytes_f *cb, void *app_key) {
	SuppressionOfAnnouncement_1_inherit_TYPE_descriptor(td);
	return td->der_encoder(td, structure, tag_mode, tag, cb, app_key);
}

#ifndef ASN1_XER_NOT_USED
asn_dec_rval_t
SuppressionOfAnnouncement_decode_xer(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const char *opt_mname, const void *bufptr, size_t size) {
	SuppressionOfAnnouncement_1_inherit_TYPE_descriptor(td);
	return td->xer_decoder(opt_codec_ctx, td, structure, opt_mname, bufptr, size);
}

asn_enc_rval_t
SuppressionOfAnnouncement_encode_xer(asn_TYPE_descriptor_t *td, void *structure,
		int ilevel, enum xer_encoder_flags_e flags,
		asn_app_consume_bytes_f *cb, void *app_key) {
	SuppressionOfAnnouncement_1_inherit_TYPE_descriptor(td);
	return td->xer_encoder(td, structure, ilevel, flags, cb, app_key);
}

#endif /* ASN1_XER_NOT_USED */
static ber_tlv_tag_t asn_DEF_SuppressionOfAnnouncement_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (5 << 2))
};
asn_TYPE_descriptor_t asn_DEF_SuppressionOfAnnouncement = {
	"SuppressionOfAnnouncement",
	"SuppressionOfAnnouncement",
	SuppressionOfAnnouncement_free,
	SuppressionOfAnnouncement_print,
	SuppressionOfAnnouncement_constraint,
	SuppressionOfAnnouncement_decode_ber,
	SuppressionOfAnnouncement_encode_der,
#ifndef ASN1_XER_NOT_USED
	SuppressionOfAnnouncement_decode_xer,
	SuppressionOfAnnouncement_encode_xer,
#else  /* ASN1_XER_NOT_USED */
	0, 0,
#endif /* ASN1_XER_NOT_USED */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_SuppressionOfAnnouncement_1_tags,
	sizeof(asn_DEF_SuppressionOfAnnouncement_1_tags)
		/sizeof(asn_DEF_SuppressionOfAnnouncement_1_tags[0]), /* 1 */
	asn_DEF_SuppressionOfAnnouncement_1_tags,	/* Same as above */
	sizeof(asn_DEF_SuppressionOfAnnouncement_1_tags)
		/sizeof(asn_DEF_SuppressionOfAnnouncement_1_tags[0]), /* 1 */
	0, 0,	/* No members */
	0	/* No specifics */
};

