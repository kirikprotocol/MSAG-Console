#ifndef __SUA_COMMUNICATION_SUAMESSAGES_DUNAMESSAGE_HPP__
# define __SUA_COMMUNICATION_SUAMESSAGES_DUNAMESSAGE_HPP__ 1

# include <sua/communication/sua_messages/SUAMessage.hpp>
# include <sua/communication/sua_messages/SuaTLV.hpp>

namespace sua_messages {

class DUNAMessage : public SUAMessage {
public:
  DUNAMessage();

  virtual size_t serialize(communication::TP* resultBuf) const;

  virtual size_t deserialize(const communication::TP& packetBuf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const;

  void setAffectedPointCode(const TLV_AffectedPointCode& affectedPointCode);
  const TLV_AffectedPointCode& getAffectedPointCode() const;

  void setSSN(const TLV_SSN& ssn);
  const TLV_SSN& getSSN() const;

  void setSMI(const TLV_SMI& smi);
  const TLV_SMI& getSMI() const;

  void setInfoString(const TLV_InfoString& infoString);
  const TLV_InfoString& getInfoString() const;

protected:
  virtual uint32_t getLength() const;

private:
  static const uint32_t _MSG_CODE = 0x0201;

  TLV_AffectedPointCode _affectedPointCodes;
  TLV_SSN _ssn;
  TLV_SMI _smi;
  TLV_InfoString _infoString;
};

}

#endif
