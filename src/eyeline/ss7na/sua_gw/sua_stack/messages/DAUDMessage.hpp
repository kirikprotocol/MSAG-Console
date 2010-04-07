#ifndef __EYELINE_SS7NA_SUAGW_SUASTACK_MESSAGES_DAUDMESSAGE_HPP__
# define __EYELINE_SS7NA_SUAGS_SUASTACK_MESSAGES_DAUDMESSAGE_HPP__

# include "eyeline/ss7na/common/AdaptationLayer_Message.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/SuaTLV.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

class DAUDMessage : public common::AdaptationLayer_Message {
public:
  DAUDMessage ()
  : common::AdaptationLayer_Message(_MSG_CODE) {}

  virtual size_t serialize(common::TP* result_buf) const;

  virtual size_t deserialize(const common::TP& packet_buf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const { return "DAUD"; }

  void setAffectedPointCode(const common::TLV_AffectedPointCode& affected_point_code) { _affectedPointCodes = affected_point_code; }
  const common::TLV_AffectedPointCode& getAffectedPointCode() const;

  void setSSN(const TLV_SSN& ssn) { _ssn = ssn; }
  const TLV_SSN& getSSN() const;

  void setUserCause(const TLV_UserCause& user_cause) { _user_cause = user_cause; }
  const TLV_UserCause& getUserCause() const;

  void setInfoString(const common::TLV_InfoString& info_string) { _infoString = info_string; }
  const common::TLV_InfoString& getInfoString() const;

protected:
  virtual uint32_t getLength() const;

private:
  static const uint32_t _MSG_CODE = 0x0203;

  common::TLV_AffectedPointCode _affectedPointCodes;
  TLV_SSN _ssn;
  TLV_UserCause _user_cause;
  common::TLV_InfoString _infoString;
};

}}}}}

#endif
