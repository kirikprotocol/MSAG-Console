#ifndef __SUA_COMMUNICATION_SUAMESSAGES_ERRORMESSAGE_HPP__
# define __SUA_COMMUNICATION_SUAMESSAGES_ERRORMESSAGE_HPP__ 1

# include <sua/communication/sua_messages/SUAMessage.hpp>
# include <sua/communication/sua_messages/SuaTLV.hpp>

namespace sua_messages {

class ErrorMessage : public SUAMessage {
public:
  ErrorMessage();

  virtual size_t serialize(communication::TP* resultBuf) const;

  virtual size_t deserialize(const communication::TP& packetBuf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const;

  virtual void dispatch_handle(const communication::LinkId& linkId) const;

  void setErrorCode(const TLV_ErrorCode& errorCode);
  const TLV_ErrorCode& getErrorCode() const;

  void setRoutingContext(const TLV_RoutingContext& routingContext);
  const TLV_RoutingContext& getRoutingContext() const;

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
  TLV_RoutingContext _routingContext;
  TLV_AffectedPointCode _affectedPointCodes;
  TLV_NetworkAppearance _networkAppearance;
  TLV_DiagnosticInformation _diagnosticInfo;
};

}

#endif
