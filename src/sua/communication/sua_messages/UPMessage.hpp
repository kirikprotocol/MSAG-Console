#ifndef __SUA_COMMUNICATION_SUAMESSAGES_UPMESSAGE_HPP__
# define __SUA_COMMUNICATION_SUAMESSAGES_UPMESSAGE_HPP__ 1

# include <sua/communication/sua_messages/SUAMessage.hpp>
# include <sua/communication/sua_messages/SuaTLV.hpp>

namespace sua_messages {

class UPMessage : public SUAMessage {
public:
  UPMessage();

  virtual size_t serialize(communication::TP* resultBuf) const;

  virtual size_t deserialize(const communication::TP& packetBuf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const;

  void setAspIdentifier(const TLV_AspIdentifier& aspId);
  const TLV_AspIdentifier& getAspIdentifier() const;

  void setInfoString(const TLV_InfoString& infoString);
  const TLV_InfoString& getInfoString() const;

protected:
  virtual uint32_t getLength() const;

private:
  TLV_AspIdentifier _aspId;
  TLV_InfoString _infoString;

  static const uint32_t _MSG_CODE = 0x0301;
};

}

#endif
