#include "SmeManConfig.h"

#include <fstream>
#include <sstream>
#include <xercesc/dom/DOM.hpp>
#include "logger/Logger.h"
#include "util/xml/utilFunctions.h"

namespace smsc {
namespace util {
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
    DOMDocument *document = reader.read(filename);
    DOMElement *elem = document->getDocumentElement();
    DOMNodeList *list = elem->getElementsByTagName(XmlStr("smerecord"));
    for (unsigned i=0; i<list->getLength(); i++)
    {
      DOMNode *node = list->item(i);
      DOMNamedNodeMap *attrs = node->getAttributes();
      XmlStr type(attrs->getNamedItem(XmlStr("type"))->getNodeValue());
      SmeRecord * record = new SmeRecord();
      record->smeUid = XmlStr(attrs->getNamedItem(XmlStr("uid"))->getNodeValue()).c_release();
      if (strcmp(type, "smpp") == 0)
      {
        record->rectype = SMPP_SME;
        DOMNodeList *childs = node->getChildNodes();
        for (unsigned j=0; j<childs->getLength(); j++)
        {
          DOMNode *child = childs->item(j);
          if (child->getNodeType() == DOMNode::ELEMENT_NODE)
          {
            DOMNamedNodeMap *childAttrs = child->getAttributes();
            XmlStr name(childAttrs->getNamedItem(XmlStr("name"))->getNodeValue());
            XmlStr value(childAttrs->getNamedItem(XmlStr("value"))->getNodeValue());
            if (strcmp(name, "typeOfNumber") == 0) {
              record->recdata.smppSme.typeOfNumber = strtoll(value.c_str(), (char**)0, 0);
            } else if (strcmp(name.c_str(), "priority") == 0) {
              record->priority = strtoll(value.c_str(), (char**)0, 0);
            } else if (strcmp(name.c_str(), "numberingPlan") == 0) {
              record->recdata.smppSme.numberingPlan = strtoll(value.c_str(), (char**)0, 0);
            } else if (strcmp(name.c_str(), "interfaceVersion") == 0) {
              record->recdata.smppSme.interfaceVersion = strtoll(value.c_str(), (char**)0, 0);
            } else if (strcmp(name.c_str(), "systemType") == 0) {
              record->recdata.smppSme.systemType = value.c_release();
            } else if (strcmp(name.c_str(), "password") == 0) {
              record->recdata.smppSme.password = value.c_release();
            } else if (strcmp(name.c_str(), "addrRange") == 0) {
              record->recdata.smppSme.addrRange = value.c_release();
            } else if (strcmp(name.c_str(), "smeN") == 0) {
              record->recdata.smppSme.smeN = strtoll(value.c_str(), (char**)0, 0);
            } else if (strcmp(name.c_str(), "timeout") == 0) {
              record->recdata.smppSme.timeout = strtoll(value.c_str(), (char**)0, 0);
            } else if (strcmp(name.c_str(), "proclimit") == 0) {
              record->recdata.smppSme.proclimit = strtoll(value.c_str(), (char**)0, 0);
            } else if (strcmp(name.c_str(), "schedlimit") == 0) {
              record->recdata.smppSme.schedlimit = strtoll(value.c_str(), (char**)0, 0);
            } else if (strcmp(name.c_str(),"forceDC") == 0) {
              record->recdata.smppSme.forceDC=!strcmp(value.c_str(),"true");
            } else if (strcmp(name.c_str(),"wantAlias") == 0) {
              record->recdata.smppSme.wantAlias=!strcmp(value.c_str(),"yes");
            } else if (strcmp(name.c_str(), "receiptSchemeName") == 0) {
              record->recdata.smppSme.receiptSchemeName = value.c_release();
            } else if (strcmp(name.c_str(), "disabled") == 0) {
              record->recdata.smppSme.disabled = !strcmp(value.c_str(),"true");
            } else if (strcmp(name.c_str(), "mode") == 0) {
              if (strcmp(value.c_str(),"tx") == 0) {
                record->recdata.smppSme.mode = MODE_TX;
              } else if (strcmp(value.c_str(),"rx") == 0) {
                record->recdata.smppSme.mode = MODE_RX;
              } else if (strcmp(value.c_str(),"trx") == 0) {
                record->recdata.smppSme.mode = MODE_TRX;
              } else {
                smsc_log_warn(logger, "unknown mode value \"%s\"", value.c_str());
              }
            } else {
              smsc_log_warn(logger, "unknown param name \"%s\"", name.c_str());
            }
          }
        }

        if (record->recdata.smppSme.systemType == 0)
          record->recdata.smppSme.systemType = cStringCopy("");
        if (record->recdata.smppSme.password == 0)
          record->recdata.smppSme.password = cStringCopy("");
        if (record->recdata.smppSme.addrRange == 0)
          record->recdata.smppSme.addrRange = cStringCopy("");
        if (record->recdata.smppSme.receiptSchemeName == 0)
          record->recdata.smppSme.receiptSchemeName = cStringCopy("default");
      }
      else if (strcmp(type.c_str(), "ss7"))
      {
        record->rectype = SS7_SME;
        smsc_log_warn(logger, "record type SS7_SME is not yet implemented. (UID = \"%s\"", record->smeUid);
        /*DOM_NodeList childs = node.getChildNodes();
        for (int j=0; j<childs.getLength(); j++)
        {
          DOM_Node child = childs.item(i);
          if (child.getNodeType == DOM_Node::ELEMENT_NODE)
          {
            DOM_NamedNodeMap childAttrs = child.getAttributes();
            std::auto_ptr<char> name = childAttrs.getNamedItem("name").getNodeValue().transcode();
            std::auto_ptr<char> value = childAttrs.getNamedItem("value").getNodeValue().transcode();
            if (strcmp(name.get(), ""))
            {
            } else if
          }
        }*/
        delete record;
        record = 0;
        continue;
      }
      else
      {
        smsc_log_warn(logger, "Unknown record type \"%s\", record skipped", type.c_str());
        delete record;
        record = 0;
        continue;
      }
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
        out << "    <param name=\"typeOfNumber\"      value=\"" << (uint32_t)rec->recdata.smppSme.typeOfNumber          << "\"/>" << std::endl;
        out << "    <param name=\"numberingPlan\"     value=\"" << (uint32_t)rec->recdata.smppSme.numberingPlan         << "\"/>" << std::endl;
        out << "    <param name=\"interfaceVersion\"  value=\"" << (uint32_t)rec->recdata.smppSme.interfaceVersion      << "\"/>" << std::endl;
        out << "    <param name=\"systemType\"        value=\"" <<           rec->recdata.smppSme.systemType            << "\"/>" << std::endl;
        out << "    <param name=\"password\"          value=\"" <<           rec->recdata.smppSme.password              << "\"/>" << std::endl;
        out << "    <param name=\"addrRange\"         value=\"" <<           rec->recdata.smppSme.addrRange             << "\"/>" << std::endl;
        out << "    <param name=\"smeN\"              value=\"" << (uint32_t)rec->recdata.smppSme.smeN                  << "\"/>" << std::endl;
        out << "    <param name=\"timeout\"           value=\"" << (uint32_t)rec->recdata.smppSme.timeout               << "\"/>" << std::endl;
        out << "    <param name=\"wantAlias\"         value=\"" <<          (rec->recdata.smppSme.wantAlias?"yes":"no") << "\"/>" << std::endl;
        out << "    <param name=\"receiptSchemeName\" value=\"" <<           rec->recdata.smppSme.receiptSchemeName     << "\"/>" << std::endl;
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
}
