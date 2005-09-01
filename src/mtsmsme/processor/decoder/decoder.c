/*
 * This is a generic BER decoder template for any ASN.1 type.
 *
 * To compile, please redefine the asn_DEF as shown:
 *
 *   cc -Dasn_DEF=asn_DEF_MyCustomType -o myTypeDecoder.o -c decoder-template.c
 */
#ifdef  HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h> /* for fstat(2)*/
#include <unistd.h> /* for getopt */
#include <string.h> /* for strerror(3) */
#include <errno.h>  /* for errno */
#include <assert.h> /* for assert(3) */
#include <sysexits.h> /* for EX_* exit codes */

#include <constr_TYPE.h>
#include <MessageType.h>
#include <Begin.h>
#include <ComponentPortion.h>
extern asn_TYPE_descriptor_t asn_DEF; /* ASN.1 type to be decoded */

/*
 * Open file and parse its BER contens.
 */
static void *data_decode_from_file(const char *fname, ssize_t suggested_bufsize);
static void *decode_from_file(const char *fname, ssize_t suggested_bufsize);
int getoid(void* ptr);
int Invoke_print(asn_TYPE_descriptor_t *td, const void *sptr, int ilevel,
    asn_app_consume_bytes_f *cb, void *app_key);


       int opt_debug = 1; /* -d */
static int opt_stack = 8192;

#define DEBUG(fmt, args...) do {    \
  if(!opt_debug) break;     \
  fprintf(stderr, fmt, ##args);   \
  fprintf(stderr, "\n");      \
} while(0)

int
decoder(int ac, char **av) {
  ssize_t suggested_bufsize = 8192;  /* close or equal to stdio buffer */
  int ch;

  /*
   * Pocess the command-line argments.
   */
  while((ch = getopt(ac, av, "h")) != -1)
  switch(ch) {
  case 'h':
  default:
    fprintf(stderr,
    "Usage: %s <data.ber> ...\n"
    , av[0]);
    exit(EX_USAGE);
  }

  ac -= optind;
  av += optind;

  if(ac < 1) {
    fprintf(stderr, "Error: missing filename\n");
    exit(EX_USAGE);
  }

  setvbuf(stdout, 0, _IOLBF, 0);

  int ac_i;
  /*
   * Process all files in turn.
   */
  for(ac_i = 0; ac_i < ac; ac_i++) {
    char *fname = av[ac_i];
    char *buf;
    ssize_t bufsz;
    struct stat fst;
    size_t rd;
    FILE *fp;

    fp = fopen(fname, "r");

    if(!fp) {
      fprintf(stderr, "%s: %s\n", fname, strerror(errno));
      return 0;
    }

    /* calculate size of file buffer */
    fstat(fileno(fp), &fst);
    bufsz = fst.st_size;

    /* prepare the file buffer */
    buf = (char *)malloc(bufsz);
    if(!buf) {
      perror("realloc()");
      exit(EX_OSERR);
    }

    rd = fread(buf, 1, bufsz, fp);
    if (rd != bufsz) {
      return 0;
    }

    fclose(fp);

    DEBUG("buffer sz=%ld read from file sz=%ld",
      (long)bufsz, rd);
    void *structure = 0;
    static asn_codec_ctx_t s_codec_ctx;
    asn_codec_ctx_t *opt_codec_ctx = 0;
    opt_codec_ctx = &s_codec_ctx;

    /*
     * Decode the encoded structure from file.
     */
    //structure = data_decode_from_file(fname, suggested_bufsize);
    asn_dec_rval_t rval;
    rval = ber_decode(opt_codec_ctx, &asn_DEF,
      (void **)&structure, buf, bufsz);
    DEBUG("ber_decode(%ld) consumed %ld, code %d",
      (long)bufsz, (long)rval.consumed, rval.code);

    if(!structure) {
      /* Error message is already printed */
      exit(EX_DATAERR);
    }
    getoid(structure);
    /* fix print function */
    asn_DEF_Invoke.print_struct = Invoke_print;
    fprintf(stderr, "%s: decoded successfully\n", fname);

    asn_fprint(stdout, &asn_DEF, structure);

    /* Check ASN.1 constraints */
    char errbuf[128];
    size_t errlen = sizeof(errbuf);
    if(asn_check_constraints(&asn_DEF, structure,
      errbuf, &errlen)) {
      fprintf(stderr, "%s: ASN.1 constraint "
        "check failed: %s\n", fname, errbuf);
      exit(EX_DATAERR);
    }

    asn_DEF.free_struct(&asn_DEF, structure, 0);
    free(buf);
  }

  return 0;
}

int getoid(void* ptr) {
  MessageType_t* pmsg = (MessageType_t*)ptr;
  if((*pmsg).present == MessageType_PR_begin) {
    printf("This is BEGIN\n");
    struct ComponentPortion* comps = pmsg->choice.begin.components;
    if (comps) {
      printf("Component is present\n");
      Component_t* comp = comps->list.array[0];
      printf("omponent type=%d\n",comp->present);
    }
  }
  return 0;
}
