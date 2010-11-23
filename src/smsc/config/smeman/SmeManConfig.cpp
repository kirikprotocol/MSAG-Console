#include <fstream>
#include <sstream>
#include <xercesc/dom/DOM.hpp>

#include "logger/Logger.h"
#include "util/xml/utilFunctions.h"
#include "util/xml/DOMTreeReader.h"
#include "smsc/smeman/smeinfo.h"

#include "SmeManConfig.h"

namespace smsc {
namespace config {
namespace smeman {

using namespace xercesc;
using namespace smsc::util::xml;

SmeManConfig::RecordIterator::RecordIterator(SRVector const &records_vector)
{
  iter = records_vector.begin();
  end = records_vector.end();
}

bool SmeManConfig::RecordIterator::hasRecord()
{
  return iter != end;
}

SmeManConfig::status SmeManConfig::RecordIterator::fetchNext(SmeRecord *&record)
{
  if (iter != end)
  {
    record = *iter;
    iter++;
    return success;
  } else {
    return fail;
  }
}

SmeManConfig::SmeManConfig()
  : logger(smsc::logger::Logger::getInstance("smsc.util.config.smeman.SmeManConfig"))
{
}

SmeManConfig::~SmeManConfig()
{
  clear();
}

void SmeManConfig::clear()
{
  for (SRVector::const_iterator i = records.begin(); i != records.end(); i++)
  {
    if (*i != 0)
    {
      delete (*i);
    }
  }
  records.clear();
}

SmeManConfig::status SmeManConfig::putRecord(SmeRecord *record)
{
  records.push_back(record);
  return success;
}



SmeManConfig::status SmeManConfig::load(const char * const filename)
{
  try
  {
    configFileName.reset(cStringCopy(filename));
    smsc::util::xml::DOMTreeReader reader;
    DOMDocument *document = reader.read(filename);
    DOMElement *elem = document->getDocumentElement();
    DOMNodeList *list = elem->getElementsByTagName(XmlStr("smerecord").x_str());
    size_t listLength = list->getLength();
    for (size_t i=0; i<listLength; i++)
    {
      DOMNode *node = list->item(i);
      DOMNamedNodeMap *attrs = node->getAttributes();
      SmeRecord * record = new SmeRecord();
      record->smeUid = XmlStr(attrs->getNamedItem(XmlStr("uid").x_str())->getNodeValue()).c_release();
      record->rectype = SMPP_SME;
      DOMNodeList *childs = node->getChildNodes();
      size_t childsLength = childs->getLength();
      for (size_t j=0; j<childsLength; j++)
      {
        DOMNode *child = childs->item(j);
        if (child->getNodeType() == DOMNode::ELEMENT_NODE)
        {
          DOMNamedNodeMap *childAttrs = child->getAttributes();
          XmlStr name(childAttrs->getNamedItem(XmlStr("name").x_str())->getNodeValue());
          XmlStr value(childAttrs->getNamedItem(XmlStr("value").x_str())->getNodeValue());
          if (strcmp(name.c_str(), "interfaceVersion") == 0)
          {
            record->smppSme.interfaceVersion = atoi(value.c_str());
            using namespace smsc::smeman;
            if((record->smppSme.interfaceVersion&0x0f)==0x09)
            {
              record->smppSme.flags=sfCarryOrgAbonentInfo|sfCarryOrgDescriptor|sfCarrySccpInfo|sfFillExtraDescriptor;
            }
            if((record->smppSme.interfaceVersion&0xf0)==0x50)
            {
              record->smppSme.flags|=sfForceReceiptToSme;
            }

          } else if (strcmp(name.c_str(), "systemType") == 0) {
            record->smppSme.systemType = value.c_release();
          } else if (strcmp(name.c_str(), "password") == 0) {
            record->smppSme.password = value.c_release();
          } else if (strcmp(name.c_str(), "addrRange") == 0) {
            record->smppSme.addrRange = value.c_release();
          } else if (strcmp(name.c_str(), "smeN") == 0) {
            record->smppSme.smeN = atoi(value.c_str());
          } else if (strcmp(name.c_str(), "priority") == 0) {
            record->priority = atoi(value.c_str());
          } else if (strcmp(name.c_str(), "timeout") == 0) {
            record->smppSme.timeout = atoi(value.c_str());
          } else if (strcmp(name.c_str(), "proclimit") == 0) {
            record->smppSme.proclimit = atoi(value.c_str());
          } else if (strcmp(name.c_str(), "schedlimit") == 0) {
            record->smppSme.schedlimit = atoi(value.c_str());
          } else if (strcmp(name.c_str(),"forceDC") == 0) {
            record->smppSme.flags|=!strcmp(value.c_str(),"true")?smsc::smeman::sfForceGsmDatacoding:0;
          } else if (strcmp(name.c_str(),"wantAlias") == 0) {
            record->smppSme.wantAlias=!strcmp(value.c_str(),"yes");
          } else if (strcmp(name.c_str(), "receiptSchemeName") == 0) {
            record->smppSme.receiptSchemeName = value.c_release();
          } else if (strcmp(name.c_str(), "disabled") == 0) {
            record->smppSme.disabled = !strcmp(value.c_str(),"true");
          } else if (strcmp(name.c_str(), "mode") == 0)
          {
            if (strcmp(value.c_str(),"tx") == 0)
            {
              record->smppSme.mode = MODE_TX;
            } else if (strcmp(value.c_str(),"rx") == 0)
            {
              record->smppSme.mode = MODE_RX;
            } else if (strcmp(value.c_str(),"trx") == 0)
            {
              record->smppSme.mode = MODE_TRX;
            } else
            {
              smsc_log_warn(logger, "unknown mode value \"%s\"", value.c_str());
            }
          } else if (strcmp(name.c_str(), "providerId") == 0) {
            record->smppSme.providerId = strtoll(value.c_str(), (char**)0, 0);
          } else if (strcmp(name.c_str(), "accessMask") == 0) {
            if(sscanf(value.c_str(),"0x%x",&record->smppSme.accessMask)!=1)
            {
              record->smppSme.accessMask = atoi(value.c_str());
            }
          } else if(strcmp(name.c_str(),"flags")==0)
          {

            record->smppSme.flags=smsc::smeman::parseSmeFlags(value.c_str());
          }else
          {
            smsc_log_warn(logger, "unknown param name \"%s\"", name.c_str());
          }
        }
      }

      if (record->smppSme.systemType == 0)
        record->smppSme.systemType = cStringCopy("");
      if (record->smppSme.password == 0)
        record->smppSme.password = cStringCopy("");
      if (record->smppSme.addrRange == 0)
        record->smppSme.addrRange = cStringCopy("");
      if (record->smppSme.receiptSchemeName == 0)
        record->smppSme.receiptSchemeName = cStringCopy("default");
      if (record != 0)
      {
        records.push_back(record);
        smsc_log_debug(logger, "readed SME: \"%s\"", record->smeUid);
      }
    }
  } catch (...) {
    return fail;
  }


  for (SRVector::iterator i = records.begin(); i != records.end(); i++) {
    smsc_log_debug(logger, "resulted SME: \"%s\"", (*i)->smeUid);
  }

  return success;
}

SmeManConfig::status SmeManConfig::reload()
{
  clear();
  std::auto_ptr<char> cfn;
  cfn = configFileName;
  return load(cfn.get());
}

SmeManConfig::status SmeManConfig::store(const char * const filename)
{
  std::ofstream out(filename);
  return store(out);
}

SmeManConfig::status SmeManConfig::store(std::ostream &out)
{
  try {
    out << "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>" << std::endl;
    out << "<!DOCTYPE records SYSTEM \"SmeRecords.dtd\">" << std::endl;
    out << "<records>" << std::endl;
    for (SRVector::const_iterator i = records.begin(); i != records.end(); i++)
    {
      SmeRecord * rec = *i;
      if (rec->rectype == SMPP_SME)
      {
        out << "  <smerecord type=\"smpp\" uid=\"" << rec->smeUid << "\">" << std::endl;
        out << "    <param name=\"priority\"          value=\"" << (uint16_t)rec->priority                              << "\"/>" << std::endl;
        out << "    <param name=\"typeOfNumber\"      value=\"" << (uint32_t)rec->smppSme.typeOfNumber          << "\"/>" << std::endl;
        out << "    <param name=\"numberingPlan\"     value=\"" << (uint32_t)rec->smppSme.numberingPlan         << "\"/>" << std::endl;
        out << "    <param name=\"interfaceVersion\"  value=\"" << (uint32_t)rec->smppSme.interfaceVersion      << "\"/>" << std::endl;
        out << "    <param name=\"systemType\"        value=\"" <<           rec->smppSme.systemType            << "\"/>" << std::endl;
        out << "    <param name=\"password\"          value=\"" <<           rec->smppSme.password              << "\"/>" << std::endl;
        out << "    <param name=\"addrRange\"         value=\"" <<           rec->smppSme.addrRange             << "\"/>" << std::endl;
        out << "    <param name=\"smeN\"              value=\"" << (uint32_t)rec->smppSme.smeN                  << "\"/>" << std::endl;
        out << "    <param name=\"timeout\"           value=\"" << (uint32_t)rec->smppSme.timeout               << "\"/>" << std::endl;
        out << "    <param name=\"wantAlias\"         value=\"" <<          (rec->smppSme.wantAlias?"yes":"no") << "\"/>" << std::endl;
        out << "    <param name=\"receiptSchemeName\" value=\"" <<           rec->smppSme.receiptSchemeName     << "\"/>" << std::endl;
        out << "  </smerecord>" << std::endl;
      } else {
        out << "  <smerecord type=\"ss7\" uid=\"" << rec->smeUid << "\">" << std::endl;
        out << "    not yet implemented" << std::endl;
        out << "  </smerecord>" << std::endl;
        smsc_log_warn(logger, "Storing SS7 SME records not yet implemented. (UID = \"%s\")", rec->smeUid);
      }
    }
    out << "</records>" << std::endl;
  } catch (...) {
    smsc_log_error(logger, "Some errors on SME records storing.");
    return fail;
  }
  return success;
}

SmeManConfig::RecordIterator SmeManConfig::getRecordIterator() const
{
  return RecordIterator(records);
}

std::string SmeManConfig::getText()
{
  std::ostringstream out;
  if (store(out) == success)
    return out.str();
  else
    return std::string();
}

}
}
}
