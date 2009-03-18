#ifndef __EYELINE_SUA_COMMUNICATION_SUAMESSAGES_ERRORMESSAGE_HPP__
# define __EYELINE_SUA_COMMUNICATION_SUAMESSAGES_ERRORMESSAGE_HPP__

# include <eyeline/sua/communication/sua_messages/SUAMessage.hpp>
# include <eyeline/sua/communication/sua_messages/SuaTLV.hpp>

namespace eyeline {
namespace sua {
namespace communication {
namespace sua_messages {

class ErrorMessage : public SUAMessage {
public:
  ErrorMessage();

  virtual size_t serialize(communication::TP* resultBuf) const;

  virtual size_t deserialize(const communication::TP& packetBuf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const;

  void setErrorCode(const TLV_ErrorCode& errorCode);
  const TLV_ErrorCode& getErrorCode() const;

  void setAffectedPointCode(const TLV_AffectedPointCode& affectedPointCode);
  const TLV_AffectedPointCode& getAffectedPointCode() const;

  void setNetworkAppearance(const TLV_NetworkAppearance& networkAppearance);
  const TLV_NetworkAppearance& getNetworkAppearance() const;

  void setDiagnosticInformation(const TLV_DiagnosticInformation& diagnosticInformation);
  const TLV_DiagnosticInformation& getDiagnosticInformation() const;

protected:
  virtual uint32_t getLength() const;

private:
  static const uint32_t _MSG_CODE = 0x0000;

  TLV_ErrorCode _errorCode;
  TLV_AffectedPointCode _affectedPointCodes;
  TLV_NetworkAppearance _networkAppearance;
  TLV_DiagnosticInformation _diagnosticInfo;
};

}}}}

#endif
