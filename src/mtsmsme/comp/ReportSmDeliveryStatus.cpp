static char const ident[] = "$Id$";

#include "mtsmsme/comp/ReportSmDeliveryStatus.hpp"
#include "mtsmsme/processor/util.hpp"
namespace smsc{namespace mtsmsme{namespace comp{

using std::vector;
using std::string;

ReportSmDeliveryStatusReq::ReportSmDeliveryStatusReq(
  const string& msisdn,
  const string& scaddress)
{
  //TODO arg.msisdn;
  //TODO arg.serviceCentreAddress;
  arg.sm_DeliveryOutcome = SM_DeliveryOutcome_successfulTransfer;
  arg.absentSubscriberDiagnosticSM = 0;
  arg.extensionContainer = 0;
  arg.gprsSupportIndicator = 0;
  arg.deliveryOutcomeIndicator = 0;
  arg.additionalSM_DeliveryOutcome = 0;
  arg.additionalAbsentSubscriberDiagnosticSM = 0;
}
void ReportSmDeliveryStatusReq::encode(vector<unsigned char>& buf)
{
  asn_enc_rval_t er;
  er = der_encode(&asn_DEF_ReportSM_DeliveryStatusArg, &arg, print2vec, &buf);
}
void ReportSmDeliveryStatusReq::decode(const vector<unsigned char>& buf) {}
ReportSmDeliveryStatusInd::ReportSmDeliveryStatusInd(Logger* _logger) :
  logger(_logger)
{
}
void ReportSmDeliveryStatusInd::decode(const vector<unsigned char>& buf)
{
  void *structure = 0;
  asn_codec_ctx_t s_codec_ctx;
  asn_codec_ctx_t *opt_codec_ctx = 0;
  opt_codec_ctx = &s_codec_ctx;
  asn_dec_rval_t rval;
  asn_TYPE_descriptor_t *def = &asn_DEF_ReportSM_DeliveryStatusArg;

  rval = ber_decode(0/*opt_codec_ctx*/, def,
      (void **) &structure, (void *) &buf[0], buf.size());
  if (rval.code != RC_OK)
    smsc_log_error(logger,
        "ReportSmDeliveryStatusInd::decode consumes %d/%d and returns code %d",
        rval.consumed,buf.size(), rval.code);
  if (structure)
  {
    /* Invoke type-specific printer */
    std::vector<unsigned char> stream;
    def->print_struct(def, structure, 1, print2vec, &stream);
    /* Create and return resulting string */
    string result((char*)&stream[0],(char*)&stream[0]+stream.size());
    smsc_log_debug(logger, "ReportSmDeliveryStatusInd\n%s", result.c_str());
    /* free decoded tree */
    def->free_struct(def, structure, 0);
  }
}
void ReportSmDeliveryStatusInd::encode(vector<unsigned char>& buf) {}
/* namespace comp */ } /* namespace mtsmsme */ } /* namespace smsc */ }
