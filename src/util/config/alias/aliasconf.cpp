/*
  $Id$
*/

#include "aliasconf.h"

#include <fstream>
#include <xercesc/dom/DOM.hpp>
#include <memory>
#include "logger/Logger.h"
#include "util/cstrings.h"
#include "util/debug.h"
#include "util/xml/utilFunctions.h"
#include "sms/sms.h"

namespace smsc {
namespace util {
namespace config {
namespace alias{

using namespace std;
using namespace xercesc;
using namespace smsc::sms;
using namespace smsc::util::xml;

AliasConfig::RecordIterator::RecordIterator(SRVector const &records_vector)
{
  iter = records_vector.begin();
  end = records_vector.end();
}

bool AliasConfig::RecordIterator::hasRecord()
{
  return iter != end;
}

AliasConfig::status AliasConfig::RecordIterator::fetchNext(AliasRecord *&record)
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

AliasConfig::AliasConfig()
  : logger(smsc::logger::Logger::getInstance("smsc.util.config.alias.AliasConfig"))
{
}

void AliasConfig::clear()
{
  {
    for ( SRVector::iterator it = records.begin();
          it != records.end();
          ++it )
    {
      if ( *it ) delete *it;
    }
  }
  records.clear();
}

AliasConfig::status AliasConfig::putRecord(AliasRecord *record)
{
  records.push_back(record);
  return success;
}

AliasConfig::status AliasConfig::load(const char * const filename)
{
  config_filename.reset(cStringCopy(filename));
  try
  {
    DOMDocument *document = reader.read(filename);
    DOMElement *elem = document->getDocumentElement();
    DOMNodeList *list = elem->getElementsByTagName(XmlStr("record"));
    for (unsigned i=0; i<list->getLength(); i++)
    {
      DOMNode *node = list->item(i);
      DOMNamedNodeMap *attrs = node->getAttributes();
      auto_ptr<AliasRecord> record(new AliasRecord());
      record->aliasValue = 0;
      record->addrValue = 0;
      //record->addr = attrs.getNamedItem("addr").getNodeValue().transcode();
      {
        XmlStr dta(attrs->getNamedItem(XmlStr("addr"))->getNodeValue());
        try
        {
          smsc::sms::Address a(dta);
          record->addrValue = new char[smsc::sms::MAX_ADDRESS_VALUE_LENGTH+1];
          a.getValue(record->addrValue);
          record->addrNpi = a.getNumberingPlan();
          record->addrTni = a.getTypeOfNumber();
        }
        catch (runtime_error &e)
        {
          smsc_log_error(logger, "exception on reading address \"%s\", nested: %s", dta.c_str(), e.what());
          continue;
        }
        //continue;
      }
      //record->alias = attrs.getNameItem("alias").getNodeValue().transcode();
      {
        XmlStr dta(attrs->getNamedItem(XmlStr("alias"))->getNodeValue());
        try
        {
          smsc::sms::Address a(dta);
          record->aliasValue = new char[smsc::sms::MAX_ADDRESS_VALUE_LENGTH+1];
          a.getValue(record->aliasValue);
          record->aliasNpi = a.getNumberingPlan();
          record->aliasTni = a.getTypeOfNumber();
          DOMNode *hide_attr_node = attrs->getNamedItem(XmlStr("hide"));
          if (hide_attr_node)
            record->hide = !strcmp(XmlStr(hide_attr_node->getNodeValue()),"true");
        }
        catch (runtime_error &e)
        {
          smsc_log_error(logger, "exception on reading alias \"%s\", nested: %s", dta.c_str(), e.what());
          continue;
        }
        //continue;
      }
      DOMNodeList *childs = node->getChildNodes();
      records.push_back(record.release());
    }
  } catch (ParseException &e) {
    smsc_log_warn(logger, "DomException:%s",e.what());
    return fail;
  }
  return success;
}

AliasConfig::status AliasConfig::reload()
{
  __trace__("smsc::util::config::alias::AliasConfig - Reload config");
  clear();
  std::auto_ptr<char> tmp = config_filename;
  status result = load(tmp.get());
  __trace2__("smsc::util::config::alias::AliasConfig - loaded %u records", records.size());
  return result;
}


AliasConfig::status AliasConfig::store(const char * const filename)
{
  try {
    std::ofstream out(filename);
    out << "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>" << std::endl;
    out << "<!DOCTYPE aliases SYSTEM \"AliasRecords.dtd\">" << std::endl;
    out << "<aliases>" << std::endl;
    for (SRVector::const_iterator i = records.begin(); i != records.end(); i++)
    {
      out << "  <record ";
      out << "addr=\"";
      if ( (*i)->addrNpi != 1 || ((*i)->addrTni !=1 && (*i)->addrTni !=2))
      {
          out << "." << (*i)->addrTni <<
          "." << (*i)->addrNpi << ".";
      }
      out << (*i)->addrValue << "\" ";
      out << "alias=\"";
      if ( (*i)->aliasNpi != 1 || ((*i)->aliasTni !=1 && (*i)->aliasTni !=2))
      {
          out << "." << (*i)->aliasTni <<
          "." << (*i)->aliasNpi << ".";
      }
      out << (*i)->aliasValue << "\" />";
    }
    out << "</aliases>" << std::endl;
  } catch (...) {
    smsc_log_error(logger, "Some errors on Aliases records storing.");
    return fail;
  }
  return success;
}

AliasConfig::RecordIterator AliasConfig::getRecordIterator() const
{
  return RecordIterator(records);
}

}
}
}
}
