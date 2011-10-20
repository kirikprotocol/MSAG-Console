#ifndef __SCAG_RULE_ENGINE_ACTION_TLV2__
#define __SCAG_RULE_ENGINE_ACTION_TLV2__

#include "scag/re/base/Action2.h"
#include "sms/sms.h"

namespace scag2 {
namespace transport {
namespace smpp {
class SmsResp;
}
}

namespace re {
namespace actions {

enum{
    TLV_SET=1,
    TLV_GET,
    TLV_DEL,
    TLV_EXIST
};

class ActionTLV : public Action
{
    ActionTLV(const ActionTLV &);

    int  type, tlv_type, m_tag;
    std::string strVar, strTag;
    static Hash<int> namesHash;
    static Hash<int> typesHash;
    bool byTag;
    FieldType ftVar, ftTag, ftTLVType;
    std::string opname_;

protected:

    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);

    /// extract the buffer from SMS/SmsResp
    bool getOptionalProperty( smsc::sms::SMS& data, const char*& buff, uint32_t& len);
    bool getOptionalProperty( transport::smpp::SmsResp& data, const char*& buf, uint32_t& len);

    /// find a tlv field in the buffer
    /// @return the position of the field or >=len
    uint32_t findField(const char* buff, uint32_t len, uint16_t fieldId);
    
    /// cut a tlv field from the buffer.
    /// @return true if the field was found, in that case tmp will contain new content
    bool cutField(const char* buff, uint32_t len, uint16_t fieldId, std::string& tmp);

    /// functions working with SMS (deliver,submit,datasm)
    bool getUnknown(const char* buff, uint32_t len, uint16_t fieldId, Property* prop);
    bool existUnknown(const char* buff, uint32_t len, uint16_t fieldId);
    /// delete tagged field, return true if deleted and tmp will contain the new content
    bool delUnknown(const char* buff, uint32_t len, uint16_t fieldId, std::string& tmp);
    /// append a tagged field to the content of tmp which should be already prepared
    void setUnknown(std::string& tmp, uint16_t fieldId, Property* prop, const std::string& str);

    void getIntTag(uint32_t val, Property* prop, int tag);
    void getStrTag(const std::string& val, Property* prop, int tag);
    void getBinTag(const char* val, uint16_t val_len, Property* prop, int tag);
    void setIntTag(smsc::sms::SMS& sms, int tag, Property* prop, const std::string& var);
    void setStrTag(smsc::sms::SMS& sms, int tag, Property* prop, const std::string& var);
    void setBinTag(smsc::sms::SMS& sms, int tag, Property* prop, const std::string& var);

    int64_t convertToIntX(const char* buf, uint16_t valeLen);
    int64_t convertToUIntX(const char* buf, uint16_t valeLen);
    bool hexDumpToBytes(const std::string& hex_dump, std::string& bytes);
    std::string uint32ToStr(uint32_t uint_val);
    void getPropertyValue(Property* prop, uint16_t tag, const std::string& var,
                          int64_t& int_val, std::string& str_val);
public:

    virtual bool run(ActionContext& context);

    ActionTLV(int t) : type(t) {
        switch (t) {
        case TLV_SET : opname_ = "smpp:set_tlv"; break;
        case TLV_GET : opname_ = "smpp:get_tlv"; break;
        case TLV_DEL : opname_ = "smpp:del_tlv"; break;
        case TLV_EXIST : opname_ = "smpp:exist_tlv"; break;
        default: opname_ = "smpp:???_tlv";
        }
    }
    virtual const char* opname() const { return opname_.c_str(); }
    virtual void init(const SectionParams& params,PropertyObject propertyObject);

    static Hash<int> InitNames();
    static Hash<int> InitTypes();
};



}}}


#endif
