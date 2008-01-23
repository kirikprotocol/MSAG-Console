#include "ActionTLV.h"
#include "scag/re/CommandAdapter.h"
#include "scag/re/CommandBrige.h"

#include "util/Uint64Converter.h"

namespace scag { namespace re { namespace actions {

const std::string HEX_CHARS("0123456789abcdefABCDEF ");
const smsc::util::HexDumpCFG HEX_DUMP_CFG = { 1, 0, " ", 0};
const MAX_INT_SIZE = sizeof(int64_t);

enum // tlv_types constants
{
    TT_UNKNOWN = 0,
    TT_STR, TT_STRN, TT_INT8, TT_INT16, TT_INT32, TT_INT64,
    TT_UINT8, TT_UINT16, TT_UINT32, TT_HEXDUMP
};

Hash<int> ActionTLV::namesHash = ActionTLV::InitNames();
Hash<int> ActionTLV::typesHash = ActionTLV::InitTypes();

Hash<int> ActionTLV::InitNames()
{
    Hash<int> hs;
    hs["name1"] = 11; // ??
    return hs;
}
Hash<int> ActionTLV::InitTypes()
{
    Hash<int> hs;
    hs["STR"] = TT_STR; hs["STRN"] = TT_STRN; hs["HEXDUMP"] = TT_HEXDUMP;
    hs["INT8"] = TT_INT8; hs["INT16"] = TT_INT16; hs["INT32"] = TT_INT32; hs["INT64"] = TT_INT64;
    hs["UINT8"] = TT_UINT8; hs["UINT16"] = TT_UINT16; hs["UINT32"] = TT_UINT32;
    return hs;
}

bool ActionTLV::getOptionalProperty(SMS& data, const char*& buff, uint32_t& len)
{
    if(!data.hasBinProperty(Tag::SMSC_UNKNOWN_OPTIONALS))
        return false;
    buff = data.getBinProperty(Tag::SMSC_UNKNOWN_OPTIONALS, &len);
    if (len < 4)
    {
        smsc_log_warn(logger, "Unknown optional field is less then 4 bytes");
        return false;
    }
    return true;
}

uint32_t ActionTLV::findField(const char* buff, uint32_t len, uint16_t fieldId)
{
    uint32_t i = 0;
    while (i <= len - 4 && *(uint16_t *)(buff + i) != fieldId) 
        i = i + 4 + *(uint16_t *)(buff + i + 2);
    if(i > len) smsc_log_warn(logger, "Error in TLV field. Index out of bounds.");
    return i;
}

void ActionTLV::cutField(const char* buff, uint32_t len, uint16_t fieldId, std::string& tmp)
{
    uint32_t i = findField(buff, len, fieldId);
    tmp.assign(buff, i < len ? i : len);
    if(i <= len - 4) 
    {
        i = i + 4 + *(uint16_t *)(buff + i + 2);
        if(i < len)
            tmp.append(buff + i, len - i);
    }
}

bool ActionTLV::getUnknown(SMS& data, uint16_t fieldId, std::string& str)
{
    uint32_t len, i;
    const char* buff;
    if(getOptionalProperty(data, buff, len) && (i = findField(buff, len, fieldId)) <= len - 4)
    {
        uint16_t valueLen = *(uint16_t *)(buff + i + 2);
        if(i + 4 + valueLen > len) return false;
        if(valueLen) str.assign((buff + i + 4), valueLen);
        return true;
    }
    return false;
}

bool ActionTLV::checkHexString(const std::string& str) {
  size_t pos = str.find_first_not_of(HEX_CHARS);
  return pos == std::string::npos ? true : false;
}

int64_t ActionTLV::convertToIntX(const char* buf, uint16_t valueLen) {
  valueLen = valueLen > MAX_INT_SIZE ? MAX_INT_SIZE : valueLen;
  uint64_t buf_val = 0;
  memcpy(&buf_val, buf, valueLen);
  int64_t value = (int64_t)smsc::util::Uint64Converter::toHostOrder(buf_val);
  value >>= (MAX_INT_SIZE - valueLen) * 8;
  return value;
}

bool ActionTLV::getUnknown(SMS& data, uint16_t fieldId, Property* prop)
{
    uint32_t len, i;
    const char* buff;
    if(getOptionalProperty(data, buff, len) && (i = findField(buff, len, fieldId)) <= len - 4)
    {
        uint16_t valueLen = *(uint16_t *)(buff + i + 2);
        if(i + 4 + valueLen > len) return false;
        bool int_val = true;
        if (!valueLen) {
          return true;
        }
        const char* valueStart = buff + i + 4;
        switch (tlv_type) {
        case TT_UINT8:
        case TT_INT8: {
          if (valueLen > sizeof(int8_t)) {
            smsc_log_warn(logger,"Action 'tlv':: value length (%d bytes) greater than, size of value type (%d byte)",
                           valueLen, sizeof(int8_t));
          }
          prop->setInt(convertToIntX(valueStart, valueLen));
          break;
        }
        case TT_UINT16:
        case TT_INT16: {
          if (valueLen > sizeof(int16_t)) {
            smsc_log_warn(logger,"Action 'tlv':: value length (%d bytes) greater than, size of value type (%d bytes)",
                           valueLen, sizeof(int16_t));
          }
          prop->setInt(convertToIntX(valueStart, valueLen));
          break;
        }
        case TT_UINT32: 
        case TT_INT32: {
          if (valueLen > sizeof(int32_t)) {
            smsc_log_warn(logger,"Action 'tlv':: value length (%d bytes) greater than, size of value type (%d bytes)",
                           valueLen, sizeof(int32_t));
          }
          prop->setInt(convertToIntX(valueStart, valueLen));
          break;
        }
        case TT_INT64: {
          if (valueLen > sizeof(int64_t)) {
            smsc_log_warn(logger,"Action 'tlv':: value length (%d bytes) greater than, size of value type (%d bytes)",
                           valueLen, sizeof(int64_t));
          }
          prop->setInt(convertToIntX(valueStart, valueLen));
          break;
        }
        case TT_UNKNOWN:
        case TT_HEXDUMP: {
          std::string hexDump;
          smsc::util::DumpHex(hexDump, valueLen, (unsigned char*)valueStart, HEX_DUMP_CFG);
          prop->setStr(hexDump);
          int_val = false;
          break;
        }
        case TT_STR: 
        case TT_STRN: {
          std::string str(valueStart, valueLen);
          prop->setStr(str);
          int_val = false;
          break;
        }
        }
        if (int_val) {
          smsc_log_debug(logger, "Action 'tlv': Tag: %d. GetValue=%d", fieldId, prop->getInt());
        } else {
          smsc_log_debug(logger, "Action 'tlv': Tag: %d. GetValue=%s", fieldId, prop->getStr().c_str());
        }
        return true;
    }
    return false;
}

bool ActionTLV::existUnknown(SMS& data, uint16_t fieldId)
{
    uint32_t len;
    const char* buff;
    return getOptionalProperty(data, buff, len) && findField(buff, len, fieldId) <= len - 4;
}

bool ActionTLV::delUnknown(SMS& data, uint16_t fieldId)
{
    uint32_t i, len;
    const char* buff;
    if(getOptionalProperty(data, buff, len) && (i = findField(buff, len, fieldId)) <= len - 4)
    {
        std::string tmp;
        tmp.assign(buff, i);
        i = i + 4 + *(uint16_t *)(buff + i + 2);
        if(i < len)
            tmp.append(buff + i, len - i);
        data.setBinProperty(Tag::SMSC_UNKNOWN_OPTIONALS, tmp.data(), tmp.size());
        return true;
    }
    return false;
}

void ActionTLV::setUnknown(SMS& data, uint16_t fieldId, const std::string& str)
{
    uint16_t i;
    uint32_t len;
    std::string tmp;
    const char* buff;

    if(getOptionalProperty(data, buff, len))
        cutField(buff, len, fieldId, tmp);

    tmp.append((char*)&fieldId, 2);
    i = str.size();
    tmp.append((char*)&i, 2);
    tmp.append(str.data(), str.size());
    data.setBinProperty(Tag::SMSC_UNKNOWN_OPTIONALS, tmp.data(), tmp.size());
}

void ActionTLV::setUnknown(SMS& data, uint16_t fieldId, Property* prop, const std::string& str)
{
    uint16_t valueLen = 0;
    int64_t int_val = 0;
    std::string str_val;
    uint32_t len = 0;
    std::string tmp;
    const char* buff;

    if(getOptionalProperty(data, buff, len))
        cutField(buff, len, fieldId, tmp);

    if ((tlv_type >= TT_UNKNOWN && tlv_type <= TT_STRN) || (tlv_type == TT_HEXDUMP)) {
        str_val = prop? prop->getStr() : str;
        smsc_log_debug(logger, "Action 'tlv': Unknown Tag: %d. SetValue=%s", fieldId, str_val.c_str());
    }
    if (tlv_type >= TT_INT8 && tlv_type <= TT_UINT32) {
      if (prop) {
        int_val = prop->getInt();
      } else {
        int_val = atoll(str.c_str());
        if(!int_val && (str[0] != '0' || str.size() != 1)) {
            smsc_log_warn(logger, "Action 'tlv': Invalid value for Unknown TAG %d, val=%s", fieldId, str.c_str());
        }
      }
      smsc_log_debug(logger, "Action 'tlv': Unknown Tag: %d. SetValue=%d", fieldId, int_val);
    }
    uint16_t netFieldId = htons(fieldId);
    tmp.append((char*)&netFieldId, 2);
    switch (tlv_type) {
    case TT_UNKNOWN:
    case TT_STR: {
      valueLen = str_val.size();
      uint16_t netValueLen = htons(valueLen);
      tmp.append((char*)&netValueLen, 2);
      tmp.append(str_val);
      break;
    }
    case TT_STRN: {
      valueLen = str_val.size() + 1;
      uint16_t netValueLen = htons(valueLen);
      tmp.append((char*)&netValueLen, 2);
      tmp.append(str_val);
      tmp += '\0';
      break;
    }
    case TT_INT8: 
    case TT_UINT8: {
      valueLen = sizeof(uint8_t);
      uint16_t netValueLen = htons(valueLen);
      tmp.append((char*)&netValueLen, 2);
      uint8_t val = (uint8_t)int_val;
      tmp.append((char*)&val, valueLen);
      break;
    }
    case TT_INT16: 
    case TT_UINT16: {
      valueLen = sizeof(uint16_t);
      uint16_t netValueLen = htons(valueLen);
      tmp.append((char*)&netValueLen, 2);
      uint16_t val = htons((uint16_t)int_val);
      tmp.append((char*)&val, valueLen);
      break;
    }
    case TT_INT32: 
    case TT_UINT32: {
      valueLen = sizeof(uint32_t);
      uint16_t netValueLen = htons(valueLen);
      tmp.append((char*)&netValueLen, 2);
      uint32_t val = htonl((uint32_t)int_val);
      tmp.append((char*)&val, valueLen);
      break;
    }
    case TT_INT64: {
      valueLen = sizeof(int64_t);
      uint16_t netValueLen = htons(valueLen);
      tmp.append((char*)&netValueLen, 2);
      uint64_t val = smsc::util::Uint64Converter::toNetworkOrder((uint64_t)int_val);
      tmp.append((char*)&val, valueLen);
      break;
    }
    case TT_HEXDUMP: {
      valueLen = str_val.size();
      uint16_t netValueLen = htons(valueLen);
      tmp.append((char*)&netValueLen, 2);
      tmp.append(str_val);
      if (!checkHexString(str_val)) {
        smsc_log_warn(logger, "Action 'tlv': invalid hex string: \'%s\'", str_val.c_str());
      }
      break;
    }
    }
    data.setBinProperty(Tag::SMSC_UNKNOWN_OPTIONALS, tmp.data(), tmp.size());
}

void ActionTLV::init(const SectionParams& params,PropertyObject propertyObject)
{
    bool bExist;

    m_tag = 0; tlv_type = TT_UNKNOWN;
    ftTag = CheckParameter(params, propertyObject, "tlv", "tag", false, true, strTag, byTag);
    if(!byTag)
    {
        ftTag = CheckParameter(params, propertyObject, "tlv", "name", false, true, strTag, bExist);
        if(ftTag == ftUnknown)
        {
            int *p = namesHash.GetPtr(strTag.c_str());
            if(!p)
                throw SCAGException("Action 'tlv': Invalid NAME value");            
            m_tag = *p;
        }
    }
    else if(ftTag == ftUnknown && !(m_tag = strtol(strTag.c_str(), NULL, 0)))
        throw SCAGException("Action 'tlv': Invalid TAG value");
    
    if(type == TLV_SET || type == TLV_GET || type == TLV_EXIST)
        ftVar = CheckParameter(params, propertyObject, "tlv", type != TLV_EXIST ? "var" : "exist", true, type == TLV_SET, strVar, bExist);
    
    if(type != TLV_EXIST && type != TLV_DEL) // init tlv_type
    { 
        ftTLVType = CheckParameter(params, propertyObject, "tlv", "type", false, true, strTag, bExist);
        if(bExist) {
            int *p = typesHash.GetPtr(strTag.c_str());
            if(ftTLVType != ftUnknown || !p) 
                throw SCAGException("Action 'tlv': Invalid TYPE argument"); // should be defined constant
            tlv_type = *p;
        }
        else smsc_log_debug(logger, "Action 'tlv': TLV type treated as unknown");
    }

    smsc_log_debug(logger,"Action 'tlv':: inited");
}

bool ActionTLV::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'tlv' type=%d, %d", type, ftVar);

    SMS& sms = CommandBrige::getSMS((SmppCommand&)context.getSCAGCommand());

    int tag = m_tag;

    if(!tag)
    {
        Property* p = context.getProperty(strTag);
        if(!p) throw SCAGException("Action 'tlv': Invalid TAG property: %s", strTag.c_str());
        if(byTag) {
            tag = strtol(p->getStr().c_str(), NULL, 0);
            if(!tag)
                throw SCAGException("Action 'tlv': Invalid TAG value");
        } else  {
            int *i = namesHash.GetPtr(p->getStr().c_str());
            if(!i)
                throw SCAGException("Action 'tlv': Invalid NAME value");
            tag = *i;
        }
    }
    
    int tt = tag >> 8;

    Property* prop = NULL;
    if(type != TLV_DEL && ftVar != ftUnknown)
    {
        prop = context.getProperty(strVar);
        if (!prop) {
            smsc_log_warn(logger,"Action 'tlv':: invalid result property '%s'",strVar.c_str());
            return true;
        }
    }
        
    if(type == TLV_EXIST)
    {
        prop->setBool(tag <= SMS_LAST_TAG ? sms.hasProperty(tag) : existUnknown(sms, tag));
        smsc_log_debug(logger, "Action 'tlv': Tag: %d is %s", tag, prop->getBool() ? "set" : "not set");
    }
    else if(type == TLV_DEL)
    {
        if(tag <= SMS_LAST_TAG) sms.dropProperty(tag);
        else delUnknown(sms, tag);
        smsc_log_debug(logger, "Action 'tlv': Tag: %d deleted", tag);
    }
    else if(type == TLV_GET)
    {
        // TODO: add tlv_type required conversion
        if(tag <= SMS_LAST_TAG)
        {
            if(!sms.hasProperty(tag)) {
                smsc_log_warn(logger, "Action 'tlv': Get of not set tag %d.", tag);
                return true;
            }
            if(tt == SMS_INT_TAG) {
                prop->setInt(sms.getIntProperty(tag));
                smsc_log_debug(logger, "Action 'tlv': Tag: %d. GetValue=%d", tag, (uint32_t)prop->getInt());
            }
            else if(tt == SMS_STR_TAG) {
                prop->setStr(sms.getStrProperty(tag | (SMS_STR_TAG << 8)));
                smsc_log_debug(logger, "Action 'tlv': Tag: %d. GetValue=%s", tag, prop->getStr().c_str());
            }
        }
        else
        {
            //if(getUnknown(sms, tag, prop->_setStr()))
              //  smsc_log_debug(logger, "Action 'tlv': Tag: %d. GetValue=%s", tag, prop->getStr().c_str());
            //else
              //  smsc_log_warn(logger, "Action 'tlv': Get of not set tag %d.", tag);
            if(!getUnknown(sms, tag, prop)) {
              smsc_log_warn(logger, "Action 'tlv': Get of not set tag %d.", tag);
            }
        }
    }
    else if(type == TLV_SET)
    {
        // TODO: add tlv_type required conversion
        if(tag <= SMS_LAST_TAG)
        {
            if(tt == SMS_INT_TAG)
            {
                int val;
                if(ftVar == ftUnknown && (tag >> 8) == SMS_INT_TAG)
                {
                    val = strtol(strVar.c_str(), NULL, 0);
                    if(!val && (strVar[0] != '0' || strVar.length() != 1)) {
                        smsc_log_error(logger, "Action 'tlv': Invalid value for integer TAG %d, var=%s", tag, strVar.c_str());
                        return true;
                    }
                }
                sms.setIntProperty(tag, prop ? (uint32_t)prop->getInt() : val);
                smsc_log_debug(logger, "Action 'tlv': Tag: %d. SetValue=%d", tag, prop ? (uint32_t)prop->getInt() : val);
            }
            else if(tt == SMS_STR_TAG)
            {
                sms.setStrProperty(tag, prop ? prop->getStr().c_str() : strVar.c_str());
                smsc_log_debug(logger, "Action 'tlv': Tag: %d. SetValue=%s", tag, prop ? prop->getStr().c_str() : strVar.c_str());
            }
        }
        else
        {
            //const std::string& strValue = prop ? prop->getStr() : strVar;
            //setUnknown(sms, tag, strValue);
            //smsc_log_debug(logger, "Action 'tlv': Unknown Tag: %d. SetValue=%s", tag, strValue.c_str());
          setUnknown(sms, tag, prop, strVar);
        }
    }
    return true;
}

IParserHandler * ActionTLV::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'tlv' cannot include child objects");
}

bool ActionTLV::FinishXMLSubSection(const std::string& name)
{
    return true;
}

}}}
