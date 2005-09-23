/*-
 * Copyright (c) 2003, 2004, 2005 Lev Walkin <vlm@lionet.info>.
 * All rights reserved.
 * Redistribution and modifications are permitted subject to BSD license.
 */
#include <asn_internal.h>
#include <INTEGER.h>
#include <asn_codecs_prim.h>	/* Encoder and decoder of a primitive type */
#include <errno.h>

/*
 * INTEGER basic type description.
 */
static ber_tlv_tag_t asn_DEF_INTEGER_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (2 << 2))
};
asn_TYPE_descriptor_t asn_DEF_INTEGER = {
	"INTEGER",
	"INTEGER",
	ASN__PRIMITIVE_TYPE_free,
	INTEGER_print,
	asn_generic_no_constraint,
	ber_decode_primitive,
	INTEGER_encode_der,
#ifndef ASN1_XER_NOT_USED
	INTEGER_decode_xer,
	INTEGER_encode_xer,
#else  /* ASN1_XER_NOT_USED */
	0, 0,
#endif /* ASN1_XER_NOT_USED */
	0, /* Use generic outmost tag fetcher */
	asn_DEF_INTEGER_tags,
	sizeof(asn_DEF_INTEGER_tags) / sizeof(asn_DEF_INTEGER_tags[0]),
	asn_DEF_INTEGER_tags,	/* Same as above */
	sizeof(asn_DEF_INTEGER_tags) / sizeof(asn_DEF_INTEGER_tags[0]),
	0, 0,	/* No members */
	0	/* No specifics */
};

/*
 * Encode INTEGER type using DER.
 */
asn_enc_rval_t
INTEGER_encode_der(asn_TYPE_descriptor_t *td, void *sptr,
	int tag_mode, ber_tlv_tag_t tag,
	asn_app_consume_bytes_f *cb, void *app_key) {
	INTEGER_t *st = (INTEGER_t *)sptr;

	ASN_DEBUG("%s %s as INTEGER (tm=%d)",
		cb?"Encoding":"Estimating", td->name, tag_mode);

	/*
	 * Canonicalize integer in the buffer.
	 * (Remove too long sign extension, remove some first 0x00 bytes)
	 */
	if(st->buf) {
		uint8_t *buf = st->buf;
		uint8_t *end1 = buf + st->size - 1;
		int shift;

		/* Compute the number of superfluous leading bytes */
		for(; buf < end1; buf++) {
			/*
			 * If the contents octets of an integer value encoding
			 * consist of more than one octet, then the bits of the
			 * first octet and bit 8 of the second octet:
			 * a) shall not all be ones; and
			 * b) shall not all be zero.
			 */
			switch(*buf) {
			case 0x00: if((buf[1] & 0x80) == 0)
					continue;
				break;
			case 0xff: if((buf[1] & 0x80))
					continue;
				break;
			}
			break;
		}

		/* Remove leading superfluous bytes from the integer */
		shift = buf - st->buf;
		if(shift) {
			uint8_t *nb = st->buf;
			uint8_t *end;

			st->size -= shift;	/* New size, minus bad bytes */
			end = nb + st->size;

			for(; nb < end; nb++, buf++)
				*nb = *buf;
		}

	} /* if(1) */

	return der_encode_primitive(td, sptr, tag_mode, tag, cb, app_key);
}

static const asn_INTEGER_enum_map_t *INTEGER_map_enum2value(asn_INTEGER_specifics_t *specs, const char *lstart, const char *lstop);

/*
 * INTEGER specific human-readable output.
 */
static ssize_t
INTEGER__dump(asn_TYPE_descriptor_t *td, const INTEGER_t *st, asn_app_consume_bytes_f *cb, void *app_key, int plainOrXER) {
	asn_INTEGER_specifics_t *specs=(asn_INTEGER_specifics_t *)td->specifics;
	char scratch[32];	/* Enough for 64-bit integer */
	uint8_t *buf = st->buf;
	uint8_t *buf_end = st->buf + st->size;
	/*signed*/ long accum;
	ssize_t wrote = 0;
	char *p;
	int ret;

	/*
	 * Advance buf pointer until the start of the value's body.
	 * This will make us able to process large integers using simple case,
	 * when the actual value is small
	 * (0x0000000000abcdef would yield a fine 0x00abcdef)
	 */
	/* Skip the insignificant leading bytes */
	for(; buf < buf_end-1; buf++) {
		switch(*buf) {
		case 0x00: if((buf[1] & 0x80) == 0) continue; break;
		case 0xff: if((buf[1] & 0x80) != 0) continue; break;
		}
		break;
	}

	/* Simple case: the integer size is small */
	if((size_t)(buf_end - buf) <= sizeof(accum)) {
		const asn_INTEGER_enum_map_t *el;
		size_t scrsize;
		char *scr;

		if(buf == buf_end) {
			accum = 0;
		} else {
			accum = (*buf & 0x80) ? -1 : 0;
			for(; buf < buf_end; buf++)
				accum = (accum << 8) | *buf;
		}

		el = INTEGER_map_value2enum(specs, accum);
		if(el) {
			scrsize = el->enum_len + 32;
			scr = (char *)alloca(scrsize);
			if(plainOrXER == 0)
				ret = snprintf(scr, scrsize,
					"%ld (%s)", accum, el->enum_name);
			else
				ret = snprintf(scr, scrsize,
					"<%s/>", el->enum_name);
		} else if(plainOrXER && specs && specs->strict_enumeration) {
			ASN_DEBUG("ASN.1 forbids dealing with "
				"unknown value of ENUMERATED type");
			errno = EPERM;
			return -1;
		} else {
			scrsize = sizeof(scratch);
			scr = scratch;
			ret = snprintf(scr, scrsize, "%ld", accum);
		}
		assert(ret > 0 && (size_t)ret < scrsize);
		return (cb(scr, ret, app_key) < 0) ? -1 : ret;
	} else if(plainOrXER && specs && specs->strict_enumeration) {
		/*
		 * Here and earlier, we cannot encode the ENUMERATED values
		 * if there is no corresponding identifier.
		 */
		ASN_DEBUG("ASN.1 forbids dealing with "
			"unknown value of ENUMERATED type");
		errno = EPERM;
		return -1;
	}

	/* Output in the long xx:yy:zz... format */
	/* TODO: replace with generic algorithm (Knuth TAOCP Vol 2, 4.3.1) */
	for(p = scratch; buf < buf_end; buf++) {
		static const char *h2c = "0123456789ABCDEF";
		if((p - scratch) >= (ssize_t)(sizeof(scratch) - 4)) {
			/* Flush buffer */
			if(cb(scratch, p - scratch, app_key) < 0)
				return -1;
			wrote += p - scratch;
			p = scratch;
		}
		*p++ = h2c[*buf >> 4];
		*p++ = h2c[*buf & 0x0F];
		*p++ = 0x3a;	/* ":" */
	}
	if(p != scratch)
		p--;	/* Remove the last ":" */

	wrote += p - scratch;
	return (cb(scratch, p - scratch, app_key) < 0) ? -1 : wrote;
}

/*
 * INTEGER specific human-readable output.
 */
int
INTEGER_print(asn_TYPE_descriptor_t *td, const void *sptr, int ilevel,
	asn_app_consume_bytes_f *cb, void *app_key) {
	const INTEGER_t *st = (const INTEGER_t *)sptr;
	ssize_t ret;

	(void)td;
	(void)ilevel;

	if(!st && !st->buf)
		ret = cb("<absent>", 8, app_key);
	else
		ret = INTEGER__dump(td, st, cb, app_key, 0);

	return (ret < 0) ? -1 : 0;
}

struct e2v_key {
	const char *start;
	const char *stop;
	asn_INTEGER_enum_map_t *vemap;
	unsigned int *evmap;
};
static int
INTEGER__compar_enum2value(const void *kp, const void *am) {
	const struct e2v_key *key = (const struct e2v_key *)kp;
	const asn_INTEGER_enum_map_t *el = (const asn_INTEGER_enum_map_t *)am;
	const char *ptr, *end, *name;

	/* Remap the element (sort by different criterion) */
	el = key->vemap + key->evmap[el - key->vemap];

	/* Compare strings */
	for(ptr = key->start, end = key->stop, name = el->enum_name;
			ptr < end; ptr++, name++) {
		if(*ptr != *name)
			return *(const unsigned char *)ptr
				- *(const unsigned char *)name;
	}
	return name[0] ? -1 : 0;
}

static const asn_INTEGER_enum_map_t *
INTEGER_map_enum2value(asn_INTEGER_specifics_t *specs, const char *lstart, const char *lstop) {
	asn_INTEGER_enum_map_t *el_found;
	int count = specs ? specs->map_count : 0;
	struct e2v_key key;
	const char *lp;

	if(!count) return NULL;

	/* Guaranteed: assert(lstart < lstop); */
	/* Figure out the tag name */
	for(lstart++, lp = lstart; lp < lstop; lp++) {
		switch(*lp) {
		case 9: case 10: case 11: case 12: case 13: case 32: /* WSP */
		case 0x2f: /* '/' */ case 0x3e: /* '>' */
			break;
		default:
			continue;
		}
		break;
	}
	if(lp == lstop) return NULL;	/* No tag found */
	lstop = lp;

	key.start = lstart;
	key.stop = lstop;
	key.vemap = specs->value2enum;
	key.evmap = specs->enum2value;
	el_found = (asn_INTEGER_enum_map_t *)bsearch(&key,
		specs->value2enum, count, sizeof(specs->value2enum[0]),
		INTEGER__compar_enum2value);
	if(el_found) {
		/* Remap enum2value into value2enum */
		el_found = key.vemap + key.evmap[el_found - key.vemap];
	}
	return el_found;
}

static int
INTEGER__compar_value2enum(const void *kp, const void *am) {
	long a = *(const long *)kp;
	const asn_INTEGER_enum_map_t *el = (const asn_INTEGER_enum_map_t *)am;
	long b = el->nat_value;
	if(a < b) return -1;
	else if(a == b) return 0;
	else return 1;
}

const asn_INTEGER_enum_map_t *
INTEGER_map_value2enum(asn_INTEGER_specifics_t *specs, long value) {
	int count = specs ? specs->map_count : 0;
	if(!count) return 0;
	return (asn_INTEGER_enum_map_t *)bsearch(&value, specs->value2enum,
		count, sizeof(specs->value2enum[0]),
		INTEGER__compar_value2enum);
}

#ifndef ASN1_XER_NOT_USED
/*
 * Decode the chunk of XML text encoding INTEGER.
 */
static enum xer_pbd_rval
INTEGER__xer_body_decode(asn_TYPE_descriptor_t *td, void *sptr, const void *chunk_buf, size_t chunk_size) {
	INTEGER_t *st = (INTEGER_t *)sptr;
	long sign = 1;
	long value;
	const char *lp;
	const char *lstart = (const char *)chunk_buf;
	const char *lstop = lstart + chunk_size;
	enum {
		ST_SKIPSPACE,
		ST_WAITDIGITS,
		ST_DIGITS,
		ST_EXTRASTUFF
	} state = ST_SKIPSPACE;

	/*
	 * We may have received a tag here. It will be processed inline.
	 * Use strtoul()-like code and serialize the result.
	 */
	for(value = 0, lp = lstart; lp < lstop; lp++) {
		int lv = *lp;
		switch(lv) {
		case 0x09: case 0x0a: case 0x0d: case 0x20:
			if(state == ST_SKIPSPACE) continue;
			break;
		case 0x2d:	/* '-' */
			if(state == ST_SKIPSPACE) {
				sign = -1;
				state = ST_WAITDIGITS;
				continue;
			}
			break;
		case 0x2b:	/* '+' */
			if(state == ST_SKIPSPACE) {
				state = ST_WAITDIGITS;
				continue;
			}
			break;
		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34:
		case 0x35: case 0x36: case 0x37: case 0x38: case 0x39:
			if(state != ST_DIGITS) state = ST_DIGITS;

		    {
			long new_value = value * 10;

			if(new_value / 10 != value)
				/* Overflow */
				return XPBD_DECODER_LIMIT;

			value = new_value + (lv - 0x30);
			/* Check for two's complement overflow */
			if(value < 0) {
				/* Check whether it is a LONG_MIN */
				if(sign == -1
				&& (unsigned long)value
						== ~((unsigned long)-1 >> 1)) {
					sign = 1;
				} else {
					/* Overflow */
					return XPBD_DECODER_LIMIT;
				}
			}
		    }
			continue;
		case 0x3c:	/* '<' */
			if(state == ST_SKIPSPACE) {
				const asn_INTEGER_enum_map_t *el;
				el = INTEGER_map_enum2value(
					(asn_INTEGER_specifics_t *)
					td->specifics, lstart, lstop);
				if(el) {
					ASN_DEBUG("Found \"%s\" => %ld",
						el->enum_name, el->nat_value);
					state = ST_DIGITS;
					value = el->nat_value;
					lp = lstop - 1;
					continue;
				}
				ASN_DEBUG("Unknown identifier for INTEGER");
			}
			return XPBD_BROKEN_ENCODING;
		}

		/* Found extra non-numeric stuff */
		state = ST_EXTRASTUFF;
		break;
	}

	if(state != ST_DIGITS) {
		if(xer_is_whitespace(lp, lstop - lp)) {
			if(state != ST_EXTRASTUFF)
				return XPBD_NOT_BODY_IGNORE;
			/* Fall through */
		} else {
			ASN_DEBUG("No useful digits in output");
			return XPBD_BROKEN_ENCODING;	/* No digits */
		}
	}

	value *= sign;	/* Change sign, if needed */

	if(asn_long2INTEGER(st, value))
		return XPBD_SYSTEM_FAILURE;

	return XPBD_BODY_CONSUMED;
}

asn_dec_rval_t
INTEGER_decode_xer(asn_codec_ctx_t *opt_codec_ctx,
	asn_TYPE_descriptor_t *td, void **sptr, const char *opt_mname,
		const void *buf_ptr, size_t size) {

	return xer_decode_primitive(opt_codec_ctx, td,
		sptr, sizeof(INTEGER_t), opt_mname,
		buf_ptr, size, INTEGER__xer_body_decode);
}

asn_enc_rval_t
INTEGER_encode_xer(asn_TYPE_descriptor_t *td, void *sptr,
	int ilevel, enum xer_encoder_flags_e flags,
		asn_app_consume_bytes_f *cb, void *app_key) {
	const INTEGER_t *st = (const INTEGER_t *)sptr;
	asn_enc_rval_t er;

	(void)ilevel;
	(void)flags;
	
	if(!st && !st->buf)
		_ASN_ENCODE_FAILED;

	er.encoded = INTEGER__dump(td, st, cb, app_key, 1);
	if(er.encoded < 0) _ASN_ENCODE_FAILED;

	return er;
}
#endif /* ASN1_XER_NOT_USED */

int
asn_INTEGER2long(const INTEGER_t *iptr, long *lptr) {
	uint8_t *b, *end;
	size_t size;
	long l;

	/* Sanity checking */
	if(!iptr || !iptr->buf || !lptr) {
		errno = EINVAL;
		return -1;
	}

	/* Cache the begin/end of the buffer */
	b = iptr->buf;	/* Start of the INTEGER buffer */
	size = iptr->size;
	end = b + size;	/* Where to stop */

	if(size > sizeof(long)) {
		uint8_t *end1 = end - 1;
		/*
		 * Slightly more advanced processing,
		 * able to >sizeof(long) bytes,
		 * when the actual value is small
		 * (0x0000000000abcdef would yield a fine 0x00abcdef)
		 */
		/* Skip out the insignificant leading bytes */
		for(; b < end1; b++) {
			switch(*b) {
			case 0x00: if((b[1] & 0x80) == 0) continue; break;
			case 0xff: if((b[1] & 0x80) != 0) continue; break;
			}
			break;
		}

		size = end - b;
		if(size > sizeof(long)) {
			/* Still cannot fit the long */
			errno = ERANGE;
			return -1;
		}
	}

	/* Shortcut processing of a corner case */
	if(end == b) {
		*lptr = 0;
		return 0;
	}

	/* Perform the sign initialization */
	/* Actually l = -(*b >> 7); gains nothing, yet unreadable! */
	if((*b >> 7)) l = -1; else l = 0;

	/* Conversion engine */
	for(; b < end; b++)
		l = (l << 8) | *b;

	*lptr = l;
	return 0;
}

int
asn_long2INTEGER(INTEGER_t *st, long value) {
	uint8_t *buf, *bp;
	uint8_t *p;
	uint8_t *pstart;
	uint8_t *pend1;
	int littleEndian = 1;	/* Run-time detection */
	int add;

	if(!st) {
		errno = EINVAL;
		return -1;
	}

	buf = (uint8_t *)MALLOC(sizeof(value));
	if(!buf) return -1;

	if(*(char *)&littleEndian) {
		pstart = (uint8_t *)&value + sizeof(value) - 1;
		pend1 = (uint8_t *)&value;
		add = -1;
	} else {
		pstart = (uint8_t *)&value;
		pend1 = pstart + sizeof(value) - 1;
		add = 1;
	}

	/*
	 * If the contents octet consists of more than one octet,
	 * then bits of the first octet and bit 8 of the second octet:
	 * a) shall not all be ones; and
	 * b) shall not all be zero.
	 */
	for(p = pstart; p != pend1; p += add) {
		switch(*p) {
		case 0x00: if((*(p+add) & 0x80) == 0)
				continue;
			break;
		case 0xff: if((*(p+add) & 0x80))
				continue;
			break;
		}
		break;
	}
	/* Copy the integer body */
	for(pstart = p, bp = buf, pend1 += add; p != pend1; p += add)
		*bp++ = *p;

	if(st->buf) FREEMEM(st->buf);
	st->buf = buf;
	st->size = bp - buf;

	return 0;
}
