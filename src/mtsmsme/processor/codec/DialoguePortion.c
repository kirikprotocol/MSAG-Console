#include <asn_internal.h>

#include <DialoguePortion.h>

int
DialoguePortion_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
      asn_app_consume_bytes_f *app_errlog, void *app_key) {
  /* Replace with underlying type checker */
  td->check_constraints = asn_DEF_EXT.check_constraints;
  return td->check_constraints(td, sptr, app_errlog, app_key);
}

/*
 * This type is implemented using EXT,
 * so here we adjust the DEF accordingly.
 */
static void
DialoguePortion_inherit_TYPE_descriptor(asn_TYPE_descriptor_t *td) {
  td->free_struct    = asn_DEF_EXT.free_struct;
  td->print_struct   = asn_DEF_EXT.print_struct;
  td->ber_decoder    = asn_DEF_EXT.ber_decoder;
  td->der_encoder    = asn_DEF_EXT.der_encoder;
  td->xer_decoder    = asn_DEF_EXT.xer_decoder;
  td->xer_encoder    = asn_DEF_EXT.xer_encoder;
  td->elements       = asn_DEF_EXT.elements;
  td->elements_count = asn_DEF_EXT.elements_count;
  td->specifics      = asn_DEF_EXT.specifics;
}

void
DialoguePortion_free(asn_TYPE_descriptor_t *td,
    void *struct_ptr, int contents_only) {
  DialoguePortion_inherit_TYPE_descriptor(td);
  td->free_struct(td, struct_ptr, contents_only);
}

int
DialoguePortion_print(asn_TYPE_descriptor_t *td, const void *struct_ptr,
    int ilevel, asn_app_consume_bytes_f *cb, void *app_key) {
  DialoguePortion_inherit_TYPE_descriptor(td);
  return td->print_struct(td, struct_ptr, ilevel, cb, app_key);
}

asn_dec_rval_t
DialoguePortion_decode_ber(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
    void **structure, void *bufptr, size_t size, int tag_mode) {
  DialoguePortion_inherit_TYPE_descriptor(td);
  return td->ber_decoder(opt_codec_ctx, td, structure, bufptr, size, tag_mode);
}

asn_enc_rval_t
DialoguePortion_encode_der(asn_TYPE_descriptor_t *td,
    void *structure, int tag_mode, ber_tlv_tag_t tag,
    asn_app_consume_bytes_f *cb, void *app_key) {
  DialoguePortion_inherit_TYPE_descriptor(td);
  return td->der_encoder(td, structure, tag_mode, tag, cb, app_key);
}

asn_dec_rval_t
DialoguePortion_decode_xer(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
    void **structure, const char *opt_mname, void *bufptr, size_t size) {
  DialoguePortion_inherit_TYPE_descriptor(td);
  return td->xer_decoder(opt_codec_ctx, td, structure, opt_mname, bufptr, size);
}

asn_enc_rval_t
DialoguePortion_encode_xer(asn_TYPE_descriptor_t *td, void *structure,
    int ilevel, enum xer_encoder_flags_e flags,
    asn_app_consume_bytes_f *cb, void *app_key) {
  DialoguePortion_inherit_TYPE_descriptor(td);
  return td->xer_encoder(td, structure, ilevel, flags, cb, app_key);
}

static ber_tlv_tag_t asn_DEF_DialoguePortion_tags[] = {
  (ASN_TAG_CLASS_APPLICATION | (11 << 2)),
  (ASN_TAG_CLASS_UNIVERSAL | (8 << 2)),
  (ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
asn_TYPE_descriptor_t asn_DEF_DialoguePortion = {
  "DialoguePortion",
  "DialoguePortion",
  DialoguePortion_free,
  DialoguePortion_print,
  DialoguePortion_constraint,
  DialoguePortion_decode_ber,
  DialoguePortion_encode_der,
  DialoguePortion_decode_xer,
  DialoguePortion_encode_xer,
  0,  /* Use generic outmost tag fetcher */
  asn_DEF_DialoguePortion_tags,
  sizeof(asn_DEF_DialoguePortion_tags)
    /sizeof(asn_DEF_DialoguePortion_tags[0]) - 1, /* 2 */
  asn_DEF_DialoguePortion_tags, /* Same as above */
  sizeof(asn_DEF_DialoguePortion_tags)
    /sizeof(asn_DEF_DialoguePortion_tags[0]), /* 3 */
  0, 0, /* Defined elsewhere */
  0 /* No specifics */
};
