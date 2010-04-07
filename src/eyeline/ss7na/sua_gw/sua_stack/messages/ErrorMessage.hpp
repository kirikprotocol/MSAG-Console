#ifndef __EYELINE_SS7NA_SUAGW_SUASTACK_MESSAGES_ERRORMESSAGE_HPP__
# define __EYELINE_SS7NA_SUAGW_SUASTACK_MESSAGES_ERRORMESSAGE_HPP__

# include "eyeline/ss7na/common/AdaptationLayer_Message.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/SuaTLV.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

class ErrorMessage : public common::AdaptationLayer_Message {
public:
  ErrorMessage ()
  : common::AdaptationLayer_Message(_MSG_CODE) {}

  virtual size_t serialize(common::TP* result_buf) const;

  virtual size_t deserialize(const common::TP& packet_buf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const { return "ERROR"; }

  void setErrorCode(const common::TLV_ErrorCode& error_code) { _errorCode = error_code; }
  const common::TLV_ErrorCode& getErrorCode() const;

  void setAffectedPointCode(const common::TLV_AffectedPointCode& affected_point_code) { _affectedPointCodes = affected_point_code; }
  const common::TLV_AffectedPointCode& getAffectedPointCode() const;

  void setNetworkAppearance(const TLV_NetworkAppearance& network_appearance) { _networkAppearance = network_appearance; }
  const TLV_NetworkAppearance& getNetworkAppearance() const;

  void setDiagnosticInformation(const common::TLV_DiagnosticInformation& diagnostic_info) { _diagnosticInfo = diagnostic_info; }
  const common::TLV_DiagnosticInformation& getDiagnosticInformation() const;

protected:
  virtual uint32_t getLength() const;

private:
  static const uint32_t _MSG_CODE = 0x0000;

  common::TLV_ErrorCode _errorCode;
  common::TLV_AffectedPointCode _affectedPointCodes;
  TLV_NetworkAppearance _networkAppearance;
  common::TLV_DiagnosticInformation _diagnosticInfo;
};

}}}}}

#endif
