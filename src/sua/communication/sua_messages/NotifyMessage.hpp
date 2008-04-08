#ifndef __SUA_COMMUNICATION_SUAMESSAGES_NOTIFYMESSAGE_HPP__
# define __SUA_COMMUNICATION_SUAMESSAGES_NOTIFYMESSAGE_HPP__ 1

# include <sua/communication/sua_messages/SUAMessage.hpp>
# include <sua/communication/sua_messages/SuaTLV.hpp>

namespace sua_messages {

class NotifyMessage : public SUAMessage {
public:
  NotifyMessage();

  virtual size_t serialize(communication::TP* resultBuf) const;

  virtual size_t deserialize(const communication::TP& packetBuf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const;

  virtual void dispatch_handle(const communication::LinkId& linkId) const;

  void setStatus(const TLV_ApplicationStatus& status);
  const TLV_ApplicationStatus& getStatus() const;

  void setAspIdentifier(TLV_AspIdentifier aspIdentifier);
  const TLV_AspIdentifier& getAspIdentifier() const;

  void setRoutingContext(const TLV_RoutingContext& routingContext);
  const TLV_RoutingContext& getRoutingContext() const;

  void setDiagnosticInfo(const TLV_DiagnosticInformation& diagnosticInfo);
  const TLV_DiagnosticInformation& getDiagnosticInfo() const;

protected:
  virtual uint32_t getLength() const;

private:
  static const uint32_t _MSG_CODE = 0x0001;

  TLV_ApplicationStatus _status;
  TLV_AspIdentifier _aspIdentifier;
  TLV_RoutingContext _routingContext;
  TLV_DiagnosticInformation _diagnosticInfo;
};

}

#endif
