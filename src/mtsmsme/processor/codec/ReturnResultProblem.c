#include <asn_internal.h>

#include <ReturnResultProblem.h>

int
ReturnResultProblem_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
      asn_app_consume_bytes_f *app_errlog, void *app_key) {
  /* Replace with underlying type checker */
  td->check_constraints = asn_DEF_NativeInteger.check_constraints;
  return td->check_constraints(td, sptr, app_errlog, app_key);
}

/*
 * This type is implemented using NativeInteger,
 * so here we adjust the DEF accordingly.
 */
static void
ReturnResultProblem_inherit_TYPE_descriptor(asn_TYPE_descriptor_t *td) {
  td->free_struct    = asn_DEF_NativeInteger.free_struct;
  td->print_struct   = asn_DEF_NativeInteger.print_struct;
  td->ber_decoder    = asn_DEF_NativeInteger.ber_decoder;
  td->der_encoder    = asn_DEF_NativeInteger.der_encoder;
  td->xer_decoder    = asn_DEF_NativeInteger.xer_decoder;
  td->xer_encoder    = asn_DEF_NativeInteger.xer_encoder;
  td->elements       = asn_DEF_NativeInteger.elements;
  td->elements_count = asn_DEF_NativeInteger.elements_count;
  td->specifics      = asn_DEF_NativeInteger.specifics;
}

void
ReturnResultProblem_free(asn_TYPE_descriptor_t *td,
    void *struct_ptr, int contents_only) {
  ReturnResultProblem_inherit_TYPE_descriptor(td);
  td->free_struct(td, struct_ptr, contents_only);
}

int
ReturnResultProblem_print(asn_TYPE_descriptor_t *td, const void *struct_ptr,
    int ilevel, asn_app_consume_bytes_f *cb, void *app_key) {
  ReturnResultProblem_inherit_TYPE_descriptor(td);
  return td->print_struct(td, struct_ptr, ilevel, cb, app_key);
}

asn_dec_rval_t
ReturnResultProblem_decode_ber(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
    void **structure, void *bufptr, size_t size, int tag_mode) {
  ReturnResultProblem_inherit_TYPE_descriptor(td);
  return td->ber_decoder(opt_codec_ctx, td, structure, bufptr, size, tag_mode);
}

asn_enc_rval_t
ReturnResultProblem_encode_der(asn_TYPE_descriptor_t *td,
    void *structure, int tag_mode, ber_tlv_tag_t tag,
    asn_app_consume_bytes_f *cb, void *app_key) {
  ReturnResultProblem_inherit_TYPE_descriptor(td);
  return td->der_encoder(td, structure, tag_mode, tag, cb, app_key);
}

asn_dec_rval_t
ReturnResultProblem_decode_xer(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
    void **structure, const char *opt_mname, void *bufptr, size_t size) {
  ReturnResultProblem_inherit_TYPE_descriptor(td);
  return td->xer_decoder(opt_codec_ctx, td, structure, opt_mname, bufptr, size);
}

asn_enc_rval_t
ReturnResultProblem_encode_xer(asn_TYPE_descriptor_t *td, void *structure,
    int ilevel, enum xer_encoder_flags_e flags,
    asn_app_consume_bytes_f *cb, void *app_key) {
  ReturnResultProblem_inherit_TYPE_descriptor(td);
  return td->xer_encoder(td, structure, ilevel, flags, cb, app_key);
}

static ber_tlv_tag_t asn_DEF_ReturnResultProblem_tags[] = {
  (ASN_TAG_CLASS_UNIVERSAL | (2 << 2))
};
asn_TYPE_descriptor_t asn_DEF_ReturnResultProblem = {
  "ReturnResultProblem",
  "ReturnResultProblem",
  ReturnResultProblem_free,
  ReturnResultProblem_print,
  ReturnResultProblem_constraint,
  ReturnResultProblem_decode_ber,
  ReturnResultProblem_encode_der,
  ReturnResultProblem_decode_xer,
  ReturnResultProblem_encode_xer,
  0,  /* Use generic outmost tag fetcher */
  asn_DEF_ReturnResultProblem_tags,
  sizeof(asn_DEF_ReturnResultProblem_tags)
    /sizeof(asn_DEF_ReturnResultProblem_tags[0]), /* 1 */
  asn_DEF_ReturnResultProblem_tags, /* Same as above */
  sizeof(asn_DEF_ReturnResultProblem_tags)
    /sizeof(asn_DEF_ReturnResultProblem_tags[0]), /* 1 */
  0, 0, /* No members */
  0 /* No specifics */
};
