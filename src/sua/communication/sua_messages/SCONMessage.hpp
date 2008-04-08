#ifndef __SUA_SUALAYER_SUASTACK_SCONMESSAGE_HPP__
# define __SUA_SUALAYER_SUASTACK_SCONMESSAGE_HPP__ 1

# include <sua/communication/sua_messages/SUAMessage.hpp>
# include <sua/communication/sua_messages/SuaTLV.hpp>

namespace sua_messages {

class SCONMessage : public SUAMessage {
public:
  SCONMessage();

  virtual size_t serialize(communication::TP* resultBuf) const;

  virtual size_t deserialize(const communication::TP& packetBuf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const;

  virtual void dispatch_handle(const communication::LinkId& linkId) const;

  void setRoutingContext(const TLV_RoutingContext& routingContext);
  const TLV_RoutingContext& getRoutingContext() const;

  void setAffectedPointCode(const TLV_AffectedPointCode& affectedPointCode);
  const TLV_AffectedPointCode& getAffectedPointCode() const;

  void setSSN(const TLV_SSN& ssn);
  const TLV_SSN& getSSN() const;

  void setCongestionLevel(const TLV_CongestionLevel& congestionLevel);
  const TLV_CongestionLevel& getCongestionLevel() const;

  void setSMI(const TLV_SMI& smi);
  const TLV_SMI& getSMI() const;

  void setInfoString(const TLV_InfoString& infoString);
  const TLV_InfoString& getInfoString() const;

protected:
  virtual uint32_t getLength() const;

private:
  static const uint32_t _MSG_CODE = 0x0204;

  TLV_RoutingContext _routingContext;
  TLV_AffectedPointCode _affectedPointCodes;
  TLV_SSN _ssn;
  TLV_CongestionLevel _congestionLevel;
  TLV_SMI _smi;
  TLV_InfoString _infoString;
};

}

#endif
