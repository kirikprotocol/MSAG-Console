#include <asn_application.h>
#include <asn_internal.h>
#include <per_encoder.h>

/* Flush partially filled buffer */
static int _uper_encode_flush_outp(asn_per_outp_t *po);

asn_enc_rval_t
uper_encode(asn_TYPE_descriptor_t *td, void *sptr, asn_app_consume_bytes_f *cb, void *app_key) {
	asn_per_outp_t po;
	asn_enc_rval_t er;

	/*
	 * Invoke type-specific encoder.
	 */
	if(!td || !td->uper_encoder)
		_ASN_ENCODE_FAILED;	/* PER is not compiled in */

	po.buffer = po.tmpspace;
	po.nboff = 0;
	po.nbits = 8 * sizeof(po.tmpspace);
	po.outper = cb;
	po.op_key = app_key;
	po.flushed_bytes = 0;

	er = td->uper_encoder(td, 0, sptr, &po);
	if(er.encoded != -1) {
		size_t bits_to_flush;

		bits_to_flush = ((po.buffer - po.tmpspace) << 3) + po.nboff;

		/* Set number of bits encoded to a firm value */
		er.encoded = (po.flushed_bytes << 3) + bits_to_flush;

		if(_uper_encode_flush_outp(&po))
			_ASN_ENCODE_FAILED;
	}

	return er;
}

/*
 * Argument type and callback necessary for uper_encode_to_buffer().
 */
typedef struct enc_to_buf_arg {
	void *buffer;
	size_t left;
} enc_to_buf_arg;
static int encode_to_buffer_cb(const void *buffer, size_t size, void *key) {
	enc_to_buf_arg *arg = (enc_to_buf_arg *)key;

	if(arg->left < size)
		return -1;	/* Data exceeds the available buffer size */

	memcpy(arg->buffer, buffer, size);
	arg->buffer = ((char *)arg->buffer) + size;
	arg->left -= size;

	return 0;
}

asn_enc_rval_t
uper_encode_to_buffer(asn_TYPE_descriptor_t *td, void *sptr, void *buffer, size_t buffer_size) {
	enc_to_buf_arg key;

	/*
	 * Invoke type-specific encoder.
	 */
	if(!td || !td->uper_encoder)
		_ASN_ENCODE_FAILED;	/* PER is not compiled in */

	key.buffer = buffer;
	key.left = buffer_size;

	ASN_DEBUG("Encoding \"%s\" using UNALIGNED PER", td->name);

	return uper_encode(td, sptr, encode_to_buffer_cb, &key);
}

static int
_uper_encode_flush_outp(asn_per_outp_t *po) {
	uint8_t *buf;

	if(po->nboff == 0 && po->buffer == po->tmpspace)
		return 0;

	buf = po->buffer + (po->nboff >> 3);
	/* Make sure we account for the last, partially filled */
	if(po->nboff & 0x07) {
		buf[0] &= 0xff << (8 - (po->nboff & 0x07));
		buf++;
	}

	return po->outper(po->tmpspace, buf - po->tmpspace, po->op_key);
}
