#include "SmeManConfig.h"

#include <fstream>
#include <xercesc/dom/DOM_NamedNodeMap.hpp>
#include <util/Logger.h>

namespace smsc {
namespace util {
namespace config {
namespace smeman {

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
  : logger(smsc::util::Logger::getCategory("smsc.util.config.smeman.SmeManConfig"))
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
    DOM_Document document = reader.read(filename);
    DOM_Element elem = document.getDocumentElement();
    DOM_NodeList list = elem.getElementsByTagName("smerecord");
    for (unsigned i=0; i<list.getLength(); i++)
    {
      DOM_Node node = list.item(i);
      DOM_NamedNodeMap attrs = node.getAttributes();
      std::auto_ptr<char> type(attrs.getNamedItem("type").getNodeValue().transcode());
      SmeRecord * record = new SmeRecord();
      record->smeUid = attrs.getNamedItem("uid").getNodeValue().transcode();
      if (strcmp(type.get(), "smpp") == 0)
      {
        record->rectype = SMPP_SME;
        DOM_NodeList childs = node.getChildNodes();
        for (unsigned j=0; j<childs.getLength(); j++)
        {
          DOM_Node child = childs.item(j);
          if (child.getNodeType() == DOM_Node::ELEMENT_NODE)
          {
            DOM_NamedNodeMap childAttrs = child.getAttributes();
            std::auto_ptr<char> name(childAttrs.getNamedItem("name").getNodeValue().transcode());
            std::auto_ptr<char> value(childAttrs.getNamedItem("value").getNodeValue().transcode());
            if (strcmp(name.get(), "typeOfNumber") == 0)
            {
              record->recdata.smppSme.typeOfNumber = strtoll(value.get(), (char**)0, 0);
            } else if (strcmp(name.get(), "numberingPlan") == 0)
            {
              record->recdata.smppSme.numberingPlan = strtoll(value.get(), (char**)0, 0);
            } else if (strcmp(name.get(), "interfaceVersion") == 0)
            {
              record->recdata.smppSme.interfaceVersion = strtoll(value.get(), (char**)0, 0);
            } else if (strcmp(name.get(), "systemType") == 0)
            {
              record->recdata.smppSme.systemType = value.release();
            } else if (strcmp(name.get(), "password") == 0)
            {
              record->recdata.smppSme.password = value.release();
            } else if (strcmp(name.get(), "addrRange") == 0)
            {
              record->recdata.smppSme.addrRange = value.release();
            } else if (strcmp(name.get(), "smeN") == 0)
            {
              record->recdata.smppSme.smeN = strtoll(value.get(), (char**)0, 0);
            } else if (strcmp(name.get(), "timeout") == 0)
            {
              record->recdata.smppSme.timeout = strtoll(value.get(), (char**)0, 0);
            } else if (strcmp(name.get(),"wantAlias") == 0)
            {
              record->recdata.smppSme.wantAlias=!strcmp(value.get(),"yes");
            } else {
              logger.warn("unknown param name \"%s\"", name.get());
            }
          }
        }
      }
      else if (strcmp(type.get(), "ss7"))
      {
        record->rectype = SS7_SME;
        logger.warn("record type SS7_SME is not yet implemented. (UID = \"%s\"", record->smeUid);
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
        logger.warn("Unknown record type \"%s\", record skipped", type.get());
        delete record;
        record = 0;
        continue;
      }
      if (record != 0)
      {
        records.push_back(record);
      }
    }
  } catch (...) {
    return fail;
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

SmeManConfig::status SmeManConfig::store(const char * const filename) const
{
  std::ofstream out(filename);
  return store(out);
}

SmeManConfig::status SmeManConfig::store(std::ostream &out) const
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
        out << "    <param name=\"typeOfNumber\"     value=\"" << (uint32_t)rec->recdata.smppSme.typeOfNumber     << "\"/>" << std::endl;
        out << "    <param name=\"numberingPlan\"    value=\"" << (uint32_t)rec->recdata.smppSme.numberingPlan    << "\"/>" << std::endl;
        out << "    <param name=\"interfaceVersion\" value=\"" << (uint32_t)rec->recdata.smppSme.interfaceVersion << "\"/>" << std::endl;
        out << "    <param name=\"systemType\"       value=\"" << rec->recdata.smppSme.systemType       << "\"/>" << std::endl;
        out << "    <param name=\"password\"         value=\"" << rec->recdata.smppSme.password         << "\"/>" << std::endl;
        out << "    <param name=\"addrRange\"        value=\"" << rec->recdata.smppSme.addrRange        << "\"/>" << std::endl;
        out << "    <param name=\"smeN\"             value=\"" << (uint32_t)rec->recdata.smppSme.smeN             << "\"/>" << std::endl;
        out << "    <param name=\"timeout\"          value=\"" << (uint32_t)rec->recdata.smppSme.timeout          << "\"/>" << std::endl;
        out << "    <param name=\"wantAlias\"        value=\"" << (rec->recdata.smppSme.wantAlias?"yes":"no")     << "\"/>" << std::endl;
        out << "  </smerecord>" << std::endl;
      } else {
        out << "  <smerecord type=\"ss7\" uid=\"" << rec->smeUid << "\">" << std::endl;
        out << "    not yet implemented" << std::endl;
        out << "  </smerecord>" << std::endl;
        logger.warn("Storing SS7 SME records not yet implemented. (UID = \"%s\")", rec->smeUid);
      }
    }
    out << "</records>" << std::endl;
  } catch (...) {
    logger.error("Some errors on SME records storing.");
    return fail;
  }
  return success;
}

SmeManConfig::RecordIterator SmeManConfig::getRecordIterator() const
{
  return RecordIterator(records);
}

std::string SmeManConfig::getText() const
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
