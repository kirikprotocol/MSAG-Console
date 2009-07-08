#ifndef __SCAG_RULE_ENGINE_ACTION_TLV2__
#define __SCAG_RULE_ENGINE_ACTION_TLV2__

#include "scag/re/base/Action2.h"
#include "sms/sms.h"

namespace scag2 {
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

    bool getOptionalProperty( smsc::sms::SMS& data, const char*& buff, uint32_t& len);
    uint32_t findField(const char* buff, uint32_t len, uint16_t fieldId);
    void cutField(const char* buff, uint32_t len, uint16_t fieldId, std::string& tmp);
    bool getUnknown(smsc::sms::SMS& data, uint16_t fieldId, Property* prop);
    bool existUnknown(smsc::sms::SMS& data, uint16_t fieldId);
    bool delUnknown(smsc::sms::SMS& data, uint16_t fieldId);
    void setUnknown(smsc::sms::SMS& data, uint16_t fieldId, Property* prop, const std::string& str);

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
