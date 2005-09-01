#include <asn_internal.h>
#include <constr_SET_OF.h>
#include <asn_SET_OF.h>
#include <assert.h>

/*
 * Number of bytes left for this structure.
 * (ctx->left) indicates the number of bytes _transferred_ for the structure.
 * (size) contains the number of bytes in the buffer passed.
 */
#define LEFT  ((size<(size_t)ctx->left)?size:(size_t)ctx->left)

/*
 * If the subprocessor function returns with an indication that it wants
 * more data, it may well be a fatal decoding problem, because the
 * size is constrained by the <TLV>'s L, even if the buffer size allows
 * reading more data.
 * For example, consider the buffer containing the following TLVs:
 * <T:5><L:1><V> <T:6>...
 * The TLV length clearly indicates that one byte is expected in V, but
 * if the V processor returns with "want more data" even if the buffer
 * contains way more data than the V processor have seen.
 */
#define SIZE_VIOLATION  (ctx->left >= 0 && (size_t)ctx->left <= size)

/*
 * This macro "eats" the part of the buffer which is definitely "consumed",
 * i.e. was correctly converted into local representation or rightfully skipped.
 */
#undef  ADVANCE
#define ADVANCE(num_bytes)  do {    \
    size_t num = num_bytes;   \
    ptr = ((char *)ptr) + num;  \
    size -= num;      \
    if(ctx->left >= 0)    \
      ctx->left -= num; \
    consumed_myself += num;   \
  } while(0)

/*
 * Switch to the next phase of parsing.
 */
#undef  NEXT_PHASE
#undef  PHASE_OUT
#define NEXT_PHASE(ctx) do {      \
    ctx->phase++;     \
    ctx->step = 0;      \
  } while(0)
#define PHASE_OUT(ctx)  do { ctx->phase = 10; } while(0)

/*
 * Return a standardized complex structure.
 */
#undef  RETURN
#define RETURN(_code) do {      \
    rval.code = _code;    \
    rval.consumed = consumed_myself;\
    return rval;      \
  } while(0)

/*
 * The decoder of the SET OF type.
 */
asn_dec_rval_t
SET_OF_decode_ber(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
  void **struct_ptr, void *ptr, size_t size, int tag_mode) {
  /*
   * Bring closer parts of structure description.
   */
  asn_SET_OF_specifics_t *specs = (asn_SET_OF_specifics_t *)td->specifics;
  asn_TYPE_member_t *element = td->elements;  /* Single one */

  /*
   * Parts of the structure being constructed.
   */
  void *st = *struct_ptr; /* Target structure. */
  asn_struct_ctx_t *ctx;  /* Decoder context */

  ber_tlv_tag_t tlv_tag;  /* T from TLV */
  asn_dec_rval_t rval;  /* Return code from subparsers */

  ssize_t consumed_myself = 0;  /* Consumed bytes from ptr */

  ASN_DEBUG("Decoding %s as SET OF", td->name);

  /*
   * Create the target structure if it is not present already.
   */
  if(st == 0) {
    st = *struct_ptr = CALLOC(1, specs->struct_size);
    if(st == 0) {
      RETURN(RC_FAIL);
    }
  }

  /*
   * Restore parsing context.
   */
  ctx = (asn_struct_ctx_t *)((char *)st + specs->ctx_offset);

  /*
   * Start to parse where left previously
   */
  switch(ctx->phase) {
  case 0:
    /*
     * PHASE 0.
     * Check that the set of tags associated with given structure
     * perfectly fits our expectations.
     */

    rval = ber_check_tags(opt_codec_ctx, td, ctx, ptr, size,
      tag_mode, 1, &ctx->left, 0);
    if(rval.code != RC_OK) {
      ASN_DEBUG("%s tagging check failed: %d",
        td->name, rval.code);
      return rval;
    }

    if(ctx->left >= 0)
      ctx->left += rval.consumed; /* ?Substracted below! */
    ADVANCE(rval.consumed);

    ASN_DEBUG("Structure consumes %ld bytes, "
      "buffer %ld", (long)ctx->left, (long)size);

    NEXT_PHASE(ctx);
    /* Fall through */
  case 1:
    /*
     * PHASE 1.
     * From the place where we've left it previously,
     * try to decode the next item.
     */
    for(;; ctx->step = 0) {
    ssize_t tag_len;  /* Length of TLV's T */

    if(ctx->step & 1)
      goto microphase2;

    /*
     * MICROPHASE 1: Synchronize decoding.
     */

    if(ctx->left == 0) {
      ASN_DEBUG("End of SET OF %s", td->name);
      /*
       * No more things to decode.
       * Exit out of here.
       */
      PHASE_OUT(ctx);
      RETURN(RC_OK);
    }

    /*
     * Fetch the T from TLV.
     */
    tag_len = ber_fetch_tag(ptr, LEFT, &tlv_tag);
    switch(tag_len) {
    case 0: if(!SIZE_VIOLATION) RETURN(RC_WMORE);
      /* Fall through */
    case -1: RETURN(RC_FAIL);
    }

    if(ctx->left < 0 && ((uint8_t *)ptr)[0] == 0) {
      if(LEFT < 2) {
        if(SIZE_VIOLATION)
          RETURN(RC_FAIL);
        else
          RETURN(RC_WMORE);
      } else if(((uint8_t *)ptr)[1] == 0) {
        /*
         * Found the terminator of the
         * indefinite length structure.
         */
        break;
      }
    }

    /* Outmost tag may be unknown and cannot be fetched/compared */
    if(element->tag != (ber_tlv_tag_t)-1) {
        if(BER_TAGS_EQUAL(tlv_tag, element->tag)) {
      /*
       * The new list member of expected type has arrived.
       */
        } else {
      ASN_DEBUG("Unexpected tag %s fixed SET OF %s",
        ber_tlv_tag_string(tlv_tag), td->name);
      ASN_DEBUG("%s SET OF has tag %s",
        td->name, ber_tlv_tag_string(element->tag));
      RETURN(RC_FAIL);
        }
    }

    /*
     * MICROPHASE 2: Invoke the member-specific decoder.
     */
    ctx->step |= 1;   /* Confirm entering next microphase */
  microphase2:

    /*
     * Invoke the member fetch routine according to member's type
     */
    rval = element->type->ber_decoder(opt_codec_ctx,
        element->type, &ctx->ptr, ptr, LEFT, 0);
    ASN_DEBUG("In %s SET OF %s code %d consumed %d",
      td->name, element->type->name,
      rval.code, (int)rval.consumed);
    switch(rval.code) {
    case RC_OK:
      {
        A_SET_OF(void) *list;
        list = (A_SET_OF(void) *)st;
        if(ASN_SET_ADD(list, ctx->ptr) != 0)
          RETURN(RC_FAIL);
        else
          ctx->ptr = 0;
      }
      break;
    case RC_WMORE: /* More data expected */
      if(!SIZE_VIOLATION) {
        ADVANCE(rval.consumed);
        RETURN(RC_WMORE);
      }
      /* Fall through */
    case RC_FAIL: /* Fatal error */
      RETURN(RC_FAIL);
    } /* switch(rval) */

    ADVANCE(rval.consumed);
    } /* for(all list members) */

    NEXT_PHASE(ctx);
  case 2:
    /*
     * Read in all "end of content" TLVs.
     */
    while(ctx->left < 0) {
      if(LEFT < 2) {
        if(LEFT > 0 && ((char *)ptr)[0] != 0) {
          /* Unexpected tag */
          RETURN(RC_FAIL);
        } else {
          RETURN(RC_WMORE);
        }
      }
      if(((char *)ptr)[0] == 0
      && ((char *)ptr)[1] == 0) {
        ADVANCE(2);
        ctx->left++;
      } else {
        RETURN(RC_FAIL);
      }
    }

    PHASE_OUT(ctx);
  }

  RETURN(RC_OK);
}

/*
 * Internally visible buffer holding a single encoded element.
 */
struct _el_buffer {
  uint8_t *buf;
  size_t length;
  size_t size;
};
/* Append bytes to the above structure */
static int _el_addbytes(const void *buffer, size_t size, void *el_buf_ptr) {
  struct _el_buffer *el_buf = (struct _el_buffer *)el_buf_ptr;

  if(el_buf->length + size > el_buf->size)
    return -1;

  memcpy(el_buf->buf + el_buf->length, buffer, size);

  el_buf->length += size;
  return 0;
}
static int _el_buf_cmp(const void *ap, const void *bp) {
  const struct _el_buffer *a = (const struct _el_buffer *)ap;
  const struct _el_buffer *b = (const struct _el_buffer *)bp;
  int ret;
  size_t common_len;

  if(a->length < b->length)
    common_len = a->length;
  else
    common_len = b->length;

  ret = memcmp(a->buf, b->buf, common_len);
  if(ret == 0) {
    if(a->length < b->length)
      ret = -1;
    else if(a->length > b->length)
      ret = 1;
  }

  return ret;
}

/*
 * The DER encoder of the SET OF type.
 */
asn_enc_rval_t
SET_OF_encode_der(asn_TYPE_descriptor_t *td, void *ptr,
  int tag_mode, ber_tlv_tag_t tag,
  asn_app_consume_bytes_f *cb, void *app_key) {
  asn_TYPE_member_t *elm = td->elements;
  asn_TYPE_descriptor_t *elm_type = elm->type;
  der_type_encoder_f *der_encoder = elm_type->der_encoder;
  A_SET_OF(void) *list;
  size_t computed_size = 0;
  ssize_t encoding_size = 0;
  struct _el_buffer *encoded_els;
  size_t max_encoded_len = 1;
  asn_enc_rval_t erval;
  int ret;
  int edx;

  ASN_DEBUG("Estimating size for SET OF %s", td->name);

  /*
   * Gather the length of the underlying members sequence.
   */
  list = (A_SET_OF(void) *)ptr;
  for(edx = 0; edx < list->count; edx++) {
    void *memb_ptr = list->array[edx];
    erval = der_encoder(elm_type, memb_ptr, 0, elm->tag, 0, 0);
    if(erval.encoded == -1)
      return erval;
    computed_size += erval.encoded;

    /* Compute maximum encoding's size */
    if(max_encoded_len < (size_t)erval.encoded)
      max_encoded_len = erval.encoded;
  }

  /*
   * Encode the TLV for the sequence itself.
   */
  encoding_size = der_write_tags(td, computed_size, tag_mode, 1, tag,
    cb, app_key);
  if(encoding_size == -1) {
    erval.encoded = -1;
    erval.failed_type = td;
    erval.structure_ptr = ptr;
    return erval;
  }
  computed_size += encoding_size;

  if(!cb) {
    erval.encoded = computed_size;
    return erval;
  }

  /*
   * DER mandates dynamic sorting of the SET OF elements
   * according to their encodings. Build an array of the
   * encoded elements.
   */
  encoded_els = (struct _el_buffer *)MALLOC(
        list->count * sizeof(encoded_els[0]));
  if(encoded_els == NULL) {
    erval.encoded = -1;
    erval.failed_type = td;
    erval.structure_ptr = ptr;
    return erval;
  }

  ASN_DEBUG("Encoding members of %s SET OF", td->name);

  /*
   * Encode all members.
   */
  for(edx = 0; edx < list->count; edx++) {
    void *memb_ptr = list->array[edx];
    struct _el_buffer *encoded_el = &encoded_els[edx];

    /*
     * Prepare space for encoding.
     */
    encoded_el->buf = (uint8_t *)MALLOC(max_encoded_len);
    if(encoded_el->buf) {
      encoded_el->length = 0;
      encoded_el->size = max_encoded_len;
    } else {
      for(edx--; edx >= 0; edx--)
        FREEMEM(encoded_els[edx].buf);
      FREEMEM(encoded_els);
      erval.encoded = -1;
      erval.failed_type = td;
      erval.structure_ptr = ptr;
      return erval;
    }

    /*
     * Encode the member into the prepared space.
     */
    erval = der_encoder(elm_type, memb_ptr, 0, elm->tag,
      _el_addbytes, encoded_el);
    if(erval.encoded == -1) {
      for(; edx >= 0; edx--)
        FREEMEM(encoded_els[edx].buf);
      FREEMEM(encoded_els);
      return erval;
    }
    encoding_size += erval.encoded;
  }

  /*
   * Sort the encoded elements according to their encoding.
   */
  qsort(encoded_els, list->count, sizeof(encoded_els[0]), _el_buf_cmp);

  /*
   * Report encoded elements to the application.
   * Dispose of temporary sorted members table.
   */
  ret = 0;
  for(edx = 0; edx < list->count; edx++) {
    struct _el_buffer *encoded_el = &encoded_els[edx];
    /* Report encoded chunks to the application */
    if(ret == 0
    && cb(encoded_el->buf, encoded_el->length, app_key) < 0)
      ret = -1;
    FREEMEM(encoded_el->buf);
  }
  FREEMEM(encoded_els);

  if(ret || computed_size != (size_t)encoding_size) {
    /*
     * Standard callback failed, or
     * encoded size is not equal to the computed size.
     */
    erval.encoded = -1;
    erval.failed_type = td;
    erval.structure_ptr = ptr;
  } else {
    erval.encoded = computed_size;
  }

  return erval;
}

typedef struct xer_tmp_enc_s {
  void *buffer;
  size_t offset;
  size_t size;
} xer_tmp_enc_t;
static int
SET_OF_encode_xer_callback(const void *buffer, size_t size, void *key) {
  xer_tmp_enc_t *t = (xer_tmp_enc_t *)key;
  if(t->offset + size >= t->size) {
    size_t newsize = (t->size << 2) + size;
    void *p = REALLOC(t->buffer, newsize);
    if(!p) return -1;
    t->buffer = p;
    t->size = newsize;
  }
  memcpy((char *)t->buffer + t->offset, buffer, size);
  t->offset += size;
  return 0;
}
static int
SET_OF_xer_order(const void *aptr, const void *bptr) {
  const xer_tmp_enc_t *a = (const xer_tmp_enc_t *)aptr;
  const xer_tmp_enc_t *b = (const xer_tmp_enc_t *)bptr;
  size_t minlen = a->offset;
  int ret;
  if(b->offset < minlen) minlen = b->offset;
  /* Well-formed UTF-8 has this nice lexicographical property... */
  ret = memcmp(a->buffer, b->buffer, minlen);
  if(ret != 0) return ret;
  if(a->offset == b->offset)
    return 0;
  if(a->offset == minlen)
    return -1;
  return 1;
}


asn_enc_rval_t
SET_OF_encode_xer(asn_TYPE_descriptor_t *td, void *sptr,
  int ilevel, enum xer_encoder_flags_e flags,
    asn_app_consume_bytes_f *cb, void *app_key) {
  asn_enc_rval_t er;
  asn_SET_OF_specifics_t *specs=(asn_SET_OF_specifics_t *)td->specifics;
  asn_TYPE_member_t *element = td->elements;
  A_SET_OF(void) *list;
  const char *mname = specs->as_XMLValueList
    ? 0 : ((*element->name)
      ? element->name : element->type->xml_tag);
  size_t mlen = mname ? strlen(mname) : 0;
  int xcan = (flags & XER_F_CANONICAL);
  xer_tmp_enc_t *encs = 0;
  size_t encs_count = 0;
  void *original_app_key = app_key;
  asn_app_consume_bytes_f *original_cb = cb;
  int i;

  if(!sptr) _ASN_ENCODE_FAILED;

  list = (A_SET_OF(void) *)sptr;

  if(xcan) {
    encs = (xer_tmp_enc_t *)MALLOC(list->count * sizeof(encs[0]));
    if(!encs) _ASN_ENCODE_FAILED;
    cb = SET_OF_encode_xer_callback;
  }

  er.encoded = 0;

  for(i = 0; i < list->count; i++) {
    asn_enc_rval_t tmper;

    void *memb_ptr = list->array[i];
    if(!memb_ptr) continue;

    if(encs) {
      memset(&encs[encs_count], 0, sizeof(encs[0]));
      app_key = &encs[encs_count];
      encs_count++;
    }

    if(mname) {
      if(!xcan) _i_ASN_TEXT_INDENT(1, ilevel);
      _ASN_CALLBACK3("<", 1, mname, mlen, ">", 1);
    }

    tmper = element->type->xer_encoder(element->type, memb_ptr,
        ilevel + 1, flags, cb, app_key);
    if(tmper.encoded == -1) {
      td = tmper.failed_type;
      sptr = tmper.structure_ptr;
      goto cb_failed;
    }

    if(mname) {
      _ASN_CALLBACK3("</", 2, mname, mlen, ">", 1);
      er.encoded += 5;
    }

    er.encoded += (2 * mlen) + tmper.encoded;
  }

  if(!xcan) _i_ASN_TEXT_INDENT(1, ilevel - 1);

  if(encs) {
    xer_tmp_enc_t *enc = encs;
    xer_tmp_enc_t *end = encs + encs_count;
    ssize_t control_size = 0;

    cb = original_cb;
    app_key = original_app_key;
    qsort(encs, encs_count, sizeof(encs[0]), SET_OF_xer_order);

    for(; enc < end; enc++) {
      _ASN_CALLBACK(enc->buffer, enc->offset);
      FREEMEM(enc->buffer);
      enc->buffer = 0;
      control_size += enc->offset;
    }
    assert(control_size == er.encoded);
  }

  goto cleanup;
cb_failed:
  er.encoded = -1;
  er.failed_type = td;
  er.structure_ptr = sptr;
cleanup:
  if(encs) {
    while(encs_count-- > 0) {
      if(encs[encs_count].buffer)
        FREEMEM(encs[encs_count].buffer);
    }
    free(encs);
  }
  return er;
}

int
SET_OF_print(asn_TYPE_descriptor_t *td, const void *sptr, int ilevel,
    asn_app_consume_bytes_f *cb, void *app_key) {
  asn_TYPE_member_t *element = td->elements;
  const A_SET_OF(void) *list;
  int ret;
  int i;

  if(!sptr) return (cb("<absent>", 8, app_key) < 0) ? -1 : 0;

  /* Dump preamble */
  if(cb(td->name, strlen(td->name), app_key) < 0
  || cb(" ::= {", 6, app_key) < 0)
    return -1;

  list = (const A_SET_OF(void) *)sptr;
  for(i = 0; i < list->count; i++) {
    const void *memb_ptr = list->array[i];
    if(!memb_ptr) continue;

    _i_INDENT(1);

    ret = element->type->print_struct(element->type, memb_ptr,
      ilevel + 1, cb, app_key);
    if(ret) return ret;
  }

  ilevel--;
  _i_INDENT(1);

  return (cb("}", 1, app_key) < 0) ? -1 : 0;
}

void
SET_OF_free(asn_TYPE_descriptor_t *td, void *ptr, int contents_only) {
  if(td && ptr) {
    asn_TYPE_member_t *element = td->elements;
    A_SET_OF(void) *list;
    int i;

    /*
     * Could not use set_of_empty() because of (*free)
     * incompatibility.
     */
    list = (A_SET_OF(void) *)ptr;
    for(i = 0; i < list->count; i++) {
      void *memb_ptr = list->array[i];
      if(memb_ptr)
      element->type->free_struct(element->type, memb_ptr, 0);
    }
    list->count = 0;  /* No meaningful elements left */

    asn_set_empty(list);  /* Remove (list->array) */

    if(!contents_only) {
      FREEMEM(ptr);
    }
  }
}

int
SET_OF_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
    asn_app_consume_bytes_f *app_errlog, void *app_key) {
  asn_TYPE_member_t *element = td->elements;
  asn_constr_check_f *constr;
  const A_SET_OF(void) *list;
  int i;

  if(!sptr) {
    _ASN_ERRLOG(app_errlog, app_key,
      "%s: value not given (%s:%d)",
      td->name, __FILE__, __LINE__);
    return -1;
  }

  list = (const A_SET_OF(void) *)sptr;

  constr = element->memb_constraints;
  if(!constr) constr = element->type->check_constraints;

  /*
   * Iterate over the members of an array.
   * Validate each in turn, until one fails.
   */
  for(i = 0; i < list->count; i++) {
    const void *memb_ptr = list->array[i];
    int ret;

    if(!memb_ptr) continue;

    ret = constr(element->type, memb_ptr, app_errlog, app_key);
    if(ret) return ret;
  }

  /*
   * Cannot inherit it eralier:
   * need to make sure we get the updated version.
   */
  if(!element->memb_constraints)
    element->memb_constraints = element->type->check_constraints;

  return 0;
}
