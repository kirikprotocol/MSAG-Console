#ifndef __SUA_COMMUNICATION_SUAMESSAGES_ACTIVEMESSAGE_HPP__
# define __SUA_COMMUNICATION_SUAMESSAGES_ACTIVEMESSAGE_HPP__ 1

# include <sua/communication/sua_messages/SUAMessage.hpp>
# include <sua/communication/sua_messages/SuaTLV.hpp>

namespace sua_messages {

class ActiveMessage : public SUAMessage {
public:
  ActiveMessage();

  virtual size_t serialize(communication::TP* resultBuf) const;

  virtual size_t deserialize(const communication::TP& packetBuf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const;

  virtual void dispatch_handle(const communication::LinkId& linkId) const;

  void setRoutingContext(const TLV_RoutingContext& routingContext);
  const TLV_RoutingContext& getRoutingContext() const;

  void setTrafficModyType(const TLV_TrafficModeType& trafficModeType);
  const TLV_TrafficModeType& getTrafficModeType() const;

  void setTIDLabel(const TLV_TIDLabel& tidLabel);
  const TLV_TIDLabel& getTIDLabel() const;

  void setDRNLabel(const TLV_DRNLabel& drnLabel);
  const TLV_DRNLabel& getDRNLabel() const;

  void setInfoString(const TLV_InfoString& infoString);
  const TLV_InfoString& getInfoString() const;

protected:
  virtual uint32_t getLength() const;

private:
  TLV_RoutingContext _routingContext;
  TLV_TrafficModeType _trafficModeType;
  TLV_TIDLabel _tidLabel;
  TLV_DRNLabel _drnLabel;
  TLV_InfoString _infoString;

  static const uint32_t _MSG_CODE = 0x0401;
};

}

#endif
