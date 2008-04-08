#ifndef __SUA_COMMUNICATION_SUAMESSAGES_DUPUMESSAGE_HPP__
# define __SUA_COMMUNICATION_SUAMESSAGES_DUPUMESSAGE_HPP__ 1

# include <sua/communication/sua_messages/SUAMessage.hpp>
# include <sua/communication/sua_messages/SuaTLV.hpp>

namespace sua_messages {

class DUPUMessage : public SUAMessage {
public:
  DUPUMessage();

  virtual size_t serialize(communication::TP* resultBuf) const;

  virtual size_t deserialize(const communication::TP& packetBuf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const;

  virtual void dispatch_handle(const communication::LinkId& linkId) const;

  void setRoutingContext(const TLV_RoutingContext& routingContext);
  const TLV_RoutingContext& getRoutingContext() const;

  void setAffectedPointCode(const TLV_AffectedPointCode& affectedPointCode);
  const TLV_AffectedPointCode& getAffectedPointCode() const;

  void setUserCause(const TLV_UserCause& user_cause);
  const TLV_UserCause& getUserCause() const;

  void setInfoString(const TLV_InfoString& infoString);
  const TLV_InfoString& getInfoString() const;

protected:
  virtual uint32_t getLength() const;

private:
  static const uint32_t _MSG_CODE = 0x0205;

  TLV_RoutingContext _routingContext;
  TLV_AffectedPointCode _affectedPointCodes;
  TLV_UserCause _user_cause;
  TLV_InfoString _infoString;
};

}

#endif
