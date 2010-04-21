#include "ActionTLV2.h"
#include "scag/re/base/CommandAdapter2.h"
#include "scag/re/base/CommandBridge.h"

#include "util/Uint64Converter.h"
#include "util/BinDump.hpp"

namespace scag2 {
namespace re {
namespace actions {

const smsc::util::HexDumpCFG HEX_DUMP_CFG = { 1, 0, " ", 0};
const int MAX_INT_SIZE = sizeof(int64_t);
const size_t UINT32_STR_MAX_SIZE = 12;

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
    if(!data.hasBinProperty(smsc::sms::Tag::SMSC_UNKNOWN_OPTIONALS))
        return false;
    buff = data.getBinProperty(smsc::sms::Tag::SMSC_UNKNOWN_OPTIONALS, &len);
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

int64_t ActionTLV::convertToIntX(const char* buf, uint16_t valueLen) {
  valueLen = valueLen > MAX_INT_SIZE ? MAX_INT_SIZE : valueLen;
  uint64_t buf_val = 0;
  memcpy(&buf_val, buf, valueLen);
  int64_t value = (int64_t)smsc::util::Uint64Converter::toHostOrder(buf_val);
  value >>= (MAX_INT_SIZE - valueLen) * 8;
  return value;
}

int64_t ActionTLV::convertToUIntX(const char* buf, uint16_t valueLen) {
  valueLen = valueLen > MAX_INT_SIZE ? MAX_INT_SIZE : valueLen;
  uint64_t buf_val = 0;
  memcpy(&buf_val, buf, valueLen);
  uint64_t value = smsc::util::Uint64Converter::toHostOrder(buf_val);
  value >>= (MAX_INT_SIZE - valueLen) * 8;
  return (int64_t)value;
}

bool ActionTLV::getUnknown(SMS& data, uint16_t fieldId, Property* prop)
{
    uint32_t len, i;
    const char* buff;
    if(getOptionalProperty(data, buff, len) && (i = findField(buff, len, fieldId)) <= len - 4)
    {
        uint16_t valueLen = *(uint16_t *)(buff + i + 2);
        if(i + 4 + valueLen > len) return false;
        getBinTag(buff + i + 4, valueLen, prop, fieldId);
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
        data.setBinProperty(smsc::sms::Tag::SMSC_UNKNOWN_OPTIONALS, tmp.data(), tmp.size());
        return true;
    }
    return false;
}

bool ActionTLV::hexDumpToBytes(const std::string& hex_dump, std::string& bytes) {
  const char* dump_ptr = hex_dump.c_str();
  char hex_val[3];
  memset(hex_val, 0, 3);
  char byte = 0;
  while (*dump_ptr) {
    if (isspace(*dump_ptr)) {
      ++dump_ptr;
      continue;
    }
    if (!isxdigit(*dump_ptr) || !isxdigit(*(dump_ptr + 1))) {
      return false;
    }
    strncpy(hex_val, dump_ptr, 2);
    byte = strtol(hex_val, NULL, 16);
    bytes.append(&byte, 1);
    dump_ptr = dump_ptr + 2;
  }
  return true;
}

void ActionTLV::getPropertyValue(Property* prop, uint16_t tag, const std::string& var, int64_t& int_val, std::string& str_val) {
  if (tlv_type == TT_UNKNOWN || tlv_type == TT_STRN || tlv_type == TT_STR || tlv_type == TT_HEXDUMP) {
      str_val = prop ? prop->getStr().c_str() : var.c_str();
      smsc_log_debug(logger, "Action 'tlv': TAG: %d. SetValue=%s", tag, str_val.c_str());
  } else  {
    if (prop) {
      int_val = prop->getInt();
    } else {
      int_val = atoll(var.c_str());
      if(!int_val && (var[0] != '0' || var.size() != 1)) {
          smsc_log_warn(logger, "Action 'tlv': Invalid value for TAG %d, val=%s", tag, var.c_str());
      }
    }
    smsc_log_debug(logger, "Action 'tlv': TAG: %d. SetValue=%d", tag, int_val);
  }
}

void ActionTLV::setUnknown(SMS& data, uint16_t fieldId, Property* prop, const std::string& var)
{
    uint16_t valueLen = 0;
    int64_t int_val = 0;
    std::string str_val;
    uint32_t len = 0;
    std::string tmp;
    const char* buff;

    if(getOptionalProperty(data, buff, len))
        cutField(buff, len, fieldId, tmp);

    getPropertyValue(prop, fieldId, var, int_val, str_val);

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
      std::string bytes("");
      if (hexDumpToBytes(str_val, bytes) && bytes.size() > 0) {
        valueLen = (uint16_t)bytes.size();
        uint16_t netValueLen = htons(valueLen);
        tmp.append((char*)&netValueLen, 2);
        tmp.append(bytes, 0, valueLen);
      } else {
        valueLen = (uint16_t)len;
        uint16_t netValueLen = htons(valueLen);
        tmp.append((char*)&netValueLen, 2);
        tmp.append(buff, valueLen);
        smsc_log_warn(logger, "Action 'tlv': invalid hex dump: \'%s\'", str_val.c_str());
      }
      break;
    }
    }
    data.setBinProperty(smsc::sms::Tag::SMSC_UNKNOWN_OPTIONALS, tmp.data(), tmp.size());
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

std::string ActionTLV::uint32ToStr(uint32_t uint_val) {
  char buf[UINT32_STR_MAX_SIZE];
  memset(buf, 0, UINT32_STR_MAX_SIZE);
  int n = snprintf(buf, UINT32_STR_MAX_SIZE - 1, "%u", uint_val);
  return n ? std::string(buf, n) : std::string("");
}

void ActionTLV::getIntTag(uint32_t val, Property* prop, int tag) {
  if (tlv_type == TT_STRN || tlv_type == TT_STR) {
    prop->setStr(uint32ToStr(val).c_str());
    smsc_log_debug(logger, "Action 'tlv': Tag: %d. GetValue=%s", tag, prop->getStr().c_str());
  } else if (tlv_type == TT_HEXDUMP) {
    std::string hexDump;
    smsc::util::DumpHex(hexDump, sizeof(uint32_t), (unsigned char*)&val, HEX_DUMP_CFG);
    prop->setStr(hexDump.c_str());
    smsc_log_debug(logger, "Action 'tlv': Tag: %d. GetValue=%s", tag, prop->getStr().c_str());
  } else {
    prop->setInt(val);
    smsc_log_debug(logger, "Action 'tlv': Tag: %d. GetValue=%d", tag, (uint32_t)prop->getInt());
  }
}

void ActionTLV::getStrTag(const std::string& val, Property* prop, int tag) {
  if (tlv_type == TT_STRN || tlv_type == TT_STR || tlv_type == TT_UNKNOWN) {
    prop->setStr(val.c_str());
    smsc_log_debug(logger, "Action 'tlv': Tag: %d. GetValue=%s", tag, prop->getStr().c_str());
  } else if (tlv_type == TT_HEXDUMP) {
    std::string hexDump;
    smsc::util::DumpHex(hexDump, val.size(), (unsigned char*)val.c_str(), HEX_DUMP_CFG);
    prop->setStr(Property::string_type(hexDump.c_str(), hexDump.size()));
    smsc_log_debug(logger, "Action 'tlv': Tag: %d. GetValue=%s", tag, prop->getStr().c_str());
  } else {
    prop->setInt(atoll(val.c_str()));
    smsc_log_debug(logger, "Action 'tlv': Tag: %d. GetValue=%d", tag, (uint32_t)prop->getInt());
  }
}

void ActionTLV::getBinTag(const char* val, uint16_t val_len, Property* prop, int tag) {
  bool int_val = true;
  if (!val || !val_len) {
    return;
  }
  switch (tlv_type) {
  case TT_INT8: {
    if (val_len > sizeof(int8_t)) {
      smsc_log_warn(logger,"Action 'tlv':: value length (%d bytes) greater than, size of value type (%d byte)",
                     val_len, sizeof(int8_t));
    }
    prop->setInt(convertToIntX(val, val_len));
    break;
  }
  case TT_UINT8: {
    if (val_len > sizeof(uint8_t)) {
      smsc_log_warn(logger,"Action 'tlv':: value length (%d bytes) greater than, size of value type (%d byte)",
                     val_len, sizeof(uint8_t));
    }
    prop->setInt(convertToUIntX(val, val_len));
    break;
  }
  case TT_INT16: {
    if (val_len > sizeof(int16_t)) {
      smsc_log_warn(logger,"Action 'tlv':: value length (%d bytes) greater than, size of value type (%d bytes)",
                     val, sizeof(int16_t));
    }
    prop->setInt(convertToIntX(val, val_len));
    break;
  }
  case TT_UINT16: {
    if (val_len > sizeof(uint16_t)) {
      smsc_log_warn(logger,"Action 'tlv':: value length (%d bytes) greater than, size of value type (%d bytes)",
                     val_len, sizeof(uint16_t));
    }
    prop->setInt(convertToUIntX(val, val_len));
    break;
  }
  case TT_INT32: {
    if (val_len > sizeof(int32_t)) {
      smsc_log_warn(logger,"Action 'tlv':: value length (%d bytes) greater than, size of value type (%d bytes)",
                     val_len, sizeof(int32_t));
    }
    prop->setInt(convertToIntX(val, val_len));
    break;
  }
  case TT_UINT32: {
    if (val_len > sizeof(uint32_t)) {
      smsc_log_warn(logger,"Action 'tlv':: value length (%d bytes) greater than, size of value type (%d bytes)",
                     val_len, sizeof(uint32_t));
    }
    prop->setInt(convertToUIntX(val, val_len));
    break;
  }
  case TT_INT64: {
    if (val_len > sizeof(int64_t)) {
      smsc_log_warn(logger,"Action 'tlv':: value length (%d bytes) greater than, size of value type (%d bytes)",
                     val_len, sizeof(int64_t));
    }
    prop->setInt(convertToIntX(val, val_len));
    break;
  }
  case TT_UNKNOWN:
  case TT_HEXDUMP: {
    std::string hexDump;
    smsc::util::DumpHex(hexDump, val_len, (unsigned char*)val, HEX_DUMP_CFG);
    prop->setStr(Property::string_type(hexDump.c_str(),hexDump.size()));
    int_val = false;
    break;
  }
  case TT_STR:
  case TT_STRN: {
    Property::string_type str(val, val_len);
    prop->setStr(str);
    int_val = false;
    break;
  }
  }
  if (int_val) {
    smsc_log_debug(logger, "Action 'tlv': Tag: %d. GetValue=%d", tag, prop->getInt());
  } else {
    smsc_log_debug(logger, "Action 'tlv': Tag: %d. GetValue=%s", tag, prop->getStr().c_str());
  }
}

void ActionTLV::setIntTag(SMS& sms, int tag, Property* prop, const std::string& var) {
  if (tlv_type == TT_STR || tlv_type == TT_STRN) {
    smsc_log_warn(logger, "Action 'tlv': Tag: %d. Can't set string value to int tag", tag);
    return;
  }
  if (tlv_type == TT_HEXDUMP) {
    smsc_log_warn(logger, "Action 'tlv': Tag: %d. Can't set hex dump value to int tag", tag);
    return;
  }
  int int_val;
  if(ftVar == ftUnknown && (tag >> 8) == SMS_INT_TAG)
  {
      int_val = strtol(var.c_str(), NULL, 0);
      if(!int_val && (var[0] != '0' || var.length() != 1)) {
          smsc_log_error(logger, "Action 'tlv': Invalid value for integer TAG %d, var=%s", tag, var.c_str());
          return;
      }
  }
  sms.setIntProperty(tag, prop ? (uint32_t)prop->getInt() : int_val);
  smsc_log_debug(logger, "Action 'tlv': Tag: %d. SetValue=%d", tag, prop ? (uint32_t)prop->getInt() : int_val);
}

void ActionTLV::setStrTag(SMS& sms, int tag, Property* prop, const std::string& var) {
  if (tlv_type == TT_STR || tlv_type == TT_STRN || tlv_type == TT_UNKNOWN) {
    sms.setStrProperty(tag, prop ? prop->getStr().c_str() : var.c_str());
    smsc_log_debug(logger, "Action 'tlv': Tag: %d. SetValue=%s", tag, prop ? prop->getStr().c_str() : var.c_str());
    return;
  }
  if (tlv_type == TT_HEXDUMP) {
    smsc_log_warn(logger, "Action 'tlv': Tag: %d. Can't set hex dump value to string tag", tag);
    return;
  }
  smsc_log_warn(logger, "Action 'tlv': Tag: %d. Can't set int value to string tag", tag);
}

void ActionTLV::setBinTag(SMS& sms, int tag, Property* prop, const std::string& var) {
  string str_val;
  int64_t int_val = 0;
  getPropertyValue(prop, tag, var, int_val, str_val);

  switch (tlv_type) {
  case TT_UNKNOWN:
  case TT_HEXDUMP: {
    std::string bytes("");
    if (hexDumpToBytes(str_val, bytes) && bytes.size() > 0) {
      sms.setBinProperty(tag, bytes.c_str(), bytes.size());
      smsc_log_debug(logger, "Action 'tlv': Tag: %d. SetValue=%s", tag, bytes.c_str());
    } else {
      smsc_log_warn(logger, "Action 'tlv': Tag: %d. Invalid hex dump: \'%s\'",
                     tag, str_val.c_str());
    }
    break;
  }
  case TT_INT8:
  case TT_UINT8: {
    uint8_t set_val = (uint8_t)int_val;
    sms.setBinProperty(tag, (char *)&set_val, sizeof(uint8_t));
  }
  case TT_INT16:
  case TT_UINT16: {
    uint16_t set_val = (uint16_t)int_val;
    sms.setBinProperty(tag, (char *)&set_val, sizeof(uint16_t));
  }
  case TT_INT32:
  case TT_UINT32: {
    uint32_t set_val = (uint32_t)int_val;
    sms.setBinProperty(tag, (char *)&set_val, sizeof(uint32_t));
  }
  case TT_INT64: {
    sms.setBinProperty(tag, (char *)&int_val, sizeof(int64_t));
  }
  case TT_STR: {
    sms.setBinProperty(tag, str_val.c_str(), str_val.size());
    break;
  }
  case TT_STRN: {
    sms.setBinProperty(tag, str_val.c_str(), str_val.size() + 1);
    break;
  }
  }
}

bool ActionTLV::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'tlv' type=%d, %d", type, ftVar);

    SMS& sms = CommandBridge::getSMS((SmppCommand&)context.getSCAGCommand());

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
        if(tag <= SMS_LAST_TAG)
        {
            if(!sms.hasProperty(tag)) {
                smsc_log_warn(logger, "Action 'tlv': Get of not set tag %d.", tag);
                return true;
            }
            if(tt == SMS_INT_TAG) {
              uint32_t val = sms.getIntProperty(tag);
              getIntTag(val, prop, tag);
            }
            else if(tt == SMS_STR_TAG) {
              std::string val = sms.getStrProperty(tag | (SMS_STR_TAG << 8));
              getStrTag(val, prop, tag);
            }
            else if (tt == SMS_BIN_TAG) {
              unsigned val_len = 0;
              const char* val = sms.getBinProperty(tag | (SMS_BIN_TAG << 8), &val_len);
              getBinTag(val, (uint16_t)val_len, prop, tag);
            }
        }
        else
        {
            if(!getUnknown(sms, tag, prop)) {
              smsc_log_warn(logger, "Action 'tlv': Get of not set tag %d.", tag);
            }
        }
    }
    else if(type == TLV_SET)
    {
        if(tag <= SMS_LAST_TAG)
        {
          if(tt == SMS_INT_TAG)
          {
            setIntTag(sms, tag, prop, strVar);
          }
          else if(tt == SMS_STR_TAG)
          {
            setStrTag(sms, tag, prop, strVar);
          }
          else if (tt == SMS_BIN_TAG)
          {
            setBinTag(sms, tag, prop, strVar);
          }
        }
        else
        {
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
