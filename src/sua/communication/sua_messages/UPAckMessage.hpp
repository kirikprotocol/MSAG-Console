#ifndef __SUA_COMMUNICATION_SUAMESSAGES_UPACKMESSAGE_HPP__
# define __SUA_COMMUNICATION_SUAMESSAGES_UPACKMESSAGE_HPP__ 1

# include <sua/communication/sua_messages/SUAMessage.hpp>
# include <sua/communication/sua_messages/SuaTLV.hpp>

namespace sua_messages {

class UPAckMessage : public SUAMessage {
public:
  UPAckMessage();

  virtual size_t serialize(communication::TP* resultBuf) const;

  virtual size_t deserialize(const communication::TP& packetBuf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const;

  void setInfoString(const TLV_InfoString& infoString);

  const TLV_InfoString& getInfoString() const;

protected:
  virtual uint32_t getLength() const;

private:
  TLV_InfoString _infoString;

  static const uint32_t _MSG_CODE = 0x0304;
};

}

#endif
