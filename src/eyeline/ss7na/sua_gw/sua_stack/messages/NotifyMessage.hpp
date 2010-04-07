#ifndef __EYELINE_SS7NA_SUAGW_SUASTACK_MESSAGES_NOTIFYMESSAGE_HPP__
# define __EYELINE_SS7NA_SUAGW_SUASTACK_MESSAGES_NOTIFYMESSAGE_HPP__

# include "eyeline/ss7na/common/AdaptationLayer_Message.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/SuaTLV.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

class NotifyMessage : public common::AdaptationLayer_Message {
public:
  NotifyMessage ()
  : common::AdaptationLayer_Message(_MSG_CODE) {}

  virtual size_t serialize(common::TP* result_buf) const;

  virtual size_t deserialize(const common::TP& packet_buf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const { return "NOTIFY"; }

  void setStatus(const common::TLV_ApplicationStatus& status) { _status = status; }
  const common::TLV_ApplicationStatus& getStatus() const;

  void setAspIdentifier(common::TLV_AspIdentifier asp_identifier) { _aspIdentifier = asp_identifier; }
  const common::TLV_AspIdentifier& getAspIdentifier() const;

  void setInfoString(const common::TLV_InfoString& info_string) { _infoString = info_string; }
  const common::TLV_InfoString& getInfoString() const;

protected:
  virtual uint32_t getLength() const;

private:
  static const uint32_t _MSG_CODE = 0x0001;

  common::TLV_ApplicationStatus _status;
  common::TLV_AspIdentifier _aspIdentifier;
  common::TLV_InfoString _infoString;
};

}}}}}

#endif
