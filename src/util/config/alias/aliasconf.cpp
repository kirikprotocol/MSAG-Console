/*
  $Id$
*/

#include "aliasconf.h"

#include <fstream>
#include <xercesc/dom/DOM_NamedNodeMap.hpp>
#include <util/Logger.h>
#include <util/cstrings.h>
#include <util/debug.h>
#include <memory>
#include "util/debug.h"
#include "sms/sms.h"

namespace smsc {
namespace util {
namespace config {
namespace alias{

using namespace std;
using namespace smsc::sms;

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
  : logger(smsc::util::Logger::getCategory("smsc.util.config.alias.AliasConfig"))
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
    DOM_Document document = reader.read(filename);
    DOM_Element elem = document.getDocumentElement();
    DOM_NodeList list = elem.getElementsByTagName("record");
    for (unsigned i=0; i<list.getLength(); i++)
    {
      DOM_Node node = list.item(i);
      DOM_NamedNodeMap attrs = node.getAttributes();
      auto_ptr<AliasRecord> record(new AliasRecord());
      record->aliasValue = 0;
      record->addrValue = 0;
      //record->addr = attrs.getNamedItem("addr").getNodeValue().transcode();
      {
        char* dta = attrs.getNamedItem("addr").getNodeValue().transcode();
        record->addrValue = new char[21]; memset(record->addrValue,0,21);
        Address tmpAddr;
        try{
          tmpAddr=Address(dta);
        }catch(...)
        {
          delete dta;
          logger.warn("incorrect format of 'addr = \"%20s\"'",  dta);
          continue;
        }
        record->addrTni=tmpAddr.type;
        record->addrNpi=tmpAddr.plan;
        memcpy(record->addrValue,tmpAddr.value,tmpAddr.length);
        delete dta;
        //continue;
      }
      //record->alias = attrs.getNameItem("alias").getNodeValue().transcode();
      {
        DOM_Node hide_attr_node = attrs.getNamedItem("hide");
        record->hide=!hide_attr_node.isNull() && !strcmp(hide_attr_node.getNodeValue().transcode(),"true");
        __trace2__("record->hide: %s",record->hide?"true":"false");
        char* dta = attrs.getNamedItem("alias").getNodeValue().transcode();
        record->aliasValue = new char[21]; memset(record->aliasValue,0,21);
        Address tmpAddr;
        try{
          tmpAddr=Address(dta);
        }catch(...)
        {
          delete dta;
          logger.warn("incorrect format of 'addr = \"%20s\"'",  dta);
          continue;
        }
        record->aliasTni=tmpAddr.type;
        record->aliasNpi=tmpAddr.plan;
        memcpy(record->aliasValue,tmpAddr.value,tmpAddr.length);
        delete dta;
        //continue;
      }
      DOM_NodeList childs = node.getChildNodes();
      //logger.info("record added");
      __trace2__("npi:%d,tni:%d,value:%20s",
                 record->aliasNpi,record->aliasTni,record->aliasValue);
      records.push_back(record.release());
    }
  } catch (DOMTreeReader::ParseException &e) {
    logger.warn("DomException:%s",e.what());
    return fail;
  }
  return success;
}

AliasConfig::status AliasConfig::reload()
{
  __trace2__("smsc::util::config::alias::AliasConfig - Reload config");
  clear();
  std::auto_ptr<char> tmp = config_filename;
  status result = load(tmp.get());
  __trace2__("smsc::util::config::alias::AliasConfig - loaded %u records", records.size());
  return result;
}


AliasConfig::status AliasConfig::store(const char * const filename) const
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
    logger.error("Some errors on Aliases records storing.");
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
